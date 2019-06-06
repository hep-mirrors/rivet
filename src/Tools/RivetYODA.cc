#include "Rivet/Config/RivetCommon.hh"
#include "Rivet/Tools/RivetYODA.hh"
#include "Rivet/Tools/RivetPaths.hh"
#include "YODA/ReaderYODA.h"
#include "YODA/ReaderAIDA.h"

// use execinfo for backtrace if available
#include "DummyConfig.hh"
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include <regex>
#include <sstream>

using namespace std;

namespace Rivet {


template <class T>
Wrapper<T>::~Wrapper() {}


template <class T>
Wrapper<T>::Wrapper(const vector<string>& weightNames, const T & p)
{
  for (const string& weightname : weightNames) {
    _persistent.push_back(make_shared<T>(p));
    _final.push_back(make_shared<T>(p));

    auto obj = _persistent.back();
    auto final = _final.back();
    if (weightname != "") {
      obj->setPath("/RAW" + obj->path() + "[" + weightname + "]");
      final->setPath(final->path() + "[" + weightname + "]");
    }
  }
}


template <class T>
typename T::Ptr Wrapper<T>::active() const {
    if ( !_active ) {
#ifdef HAVE_BACKTRACE
      void * buffer[4];
      backtrace(buffer, 4);
      backtrace_symbols_fd(buffer, 4 , 1);
#endif
      assert(false && "No active pointer set. Was this object booked in init()?");
    }
    return _active;
}


template <class T>
void Wrapper<T>::newSubEvent() {
  typename TupleWrapper<T>::Ptr tmp
      = make_shared<TupleWrapper<T>>(_persistent[0]->clone());
  tmp->reset();
  _evgroup.push_back( tmp );
  _active = _evgroup.back();
  assert(_active);
}



  string getDatafilePath(const string& papername) {
    /// Try to find YODA otherwise fall back to try AIDA
    const string path1 = findAnalysisRefFile(papername + ".yoda");
    if (!path1.empty()) return path1;
    const string path2 = findAnalysisRefFile(papername + ".aida");
    if (!path2.empty()) return path2;
    throw Rivet::Error("Couldn't find ref data file '" + papername + ".yoda" +
                       " in data path, '" + getRivetDataPath() + "', or '.'");
  }


  map<string, YODA::AnalysisObjectPtr> getRefData(const string& papername) {
    const string datafile = getDatafilePath(papername);

    // Make an appropriate data file reader and read the data objects
    /// @todo Remove AIDA support some day...
    YODA::Reader& reader = (datafile.find(".yoda") != string::npos) ?   \
      YODA::ReaderYODA::create() : YODA::ReaderAIDA::create();
    vector<YODA::AnalysisObject *> aovec;
    reader.read(datafile, aovec);

    // Return value, to be populated
    map<string, YODA::AnalysisObjectPtr> rtn;
    for ( YODA::AnalysisObject* ao : aovec ) {
      YODA::AnalysisObjectPtr refdata(ao);
      if (!refdata) continue;
      const string plotpath = refdata->path();
      // Split path at "/" and only return the last field, i.e. the histogram ID
      const size_t slashpos = plotpath.rfind("/");
      const string plotname = (slashpos+1 < plotpath.size()) ? plotpath.substr(slashpos+1) : "";
      rtn[plotname] = refdata;
    }
    return rtn;
  }


}


namespace {

  using Rivet::Fill;
  using Rivet::Fills;
  using Rivet::TupleWrapper;

    template <class T>
    double get_window_size(const typename T::Ptr & histo,
                           typename T::BinType x) {
        // the bin index we fall in
        const auto binidx = histo->binIndexAt(x);
        // gaps, overflow, underflow don't contribute
        if ( binidx == -1 )
            return 0;


        const auto & b = histo->bin(binidx);

        // if we don't have a valid neighbouring bin,
        // we use infinite width
        typename T::Bin b1(-1.0/0.0, 1.0/0.0);

        // points in the top half compare to the upper neighbour
        if ( x > b.xMid() ) {
            size_t nextidx = binidx + 1;
            if ( nextidx < histo->bins().size() )
                b1 = histo->bin(nextidx);
        }
        else { // compare to the lower neighbour
            int nextidx = binidx - 1;
            if ( nextidx >= 0 )
                b1 = histo->bin(nextidx);
        }
        // the factor 2 is arbitrary, could poss. be smaller
        return min( b.width(), b1.width() ) / 2.0;
    }

    template <class T>
     typename T::BinType
     fillT2binT(typename T::FillType a) {
       return a;
    }

    template <>
    YODA::Profile1D::BinType
    fillT2binT<YODA::Profile1D>(YODA::Profile1D::FillType a) {
      return get<0>(a);
    }

    template <>
    YODA::Profile2D::BinType
    fillT2binT<YODA::Profile2D>(YODA::Profile2D::FillType a) {
      return YODA::Profile2D::BinType{ get<0>(a), get<1>(a) };
    }



  template <class T>
  void commit(vector<typename T::Ptr> & persistent,
              const vector< vector<Fill<T>> > & tuple,
              const vector<valarray<double>> & weights ) {

    // TODO check if all the xs are in the same bin anyway!
    // Then no windowing needed

    assert(persistent.size() == weights[0].size());

    for ( const auto & x : tuple ) {
      double maxwindow = 0.0;
      for ( const auto & xi : x ) {
        // TODO check for NOFILL here
        // persistent[0] has the same binning as all the other persistent objects
        double window = get_window_size<T>(persistent[0], fillT2binT<T>(xi.first));
        if ( window > maxwindow )
          maxwindow = window;
      }
      const double wsize = maxwindow;
      // all windows have same size

      set<double> edgeset;
      // bin edges need to be in here!
      for ( const auto & xi : x ) {
        edgeset.insert(fillT2binT<T>(xi.first) - wsize);
        edgeset.insert(fillT2binT<T>(xi.first) + wsize);
      }

          vector< std::tuple<double,valarray<double>,double> > hfill;
          double sumf = 0.0;
          auto edgit = edgeset.begin();
          double ehi = *edgit;
          while ( ++edgit != edgeset.end() ) {
            double elo = ehi;
            ehi = *edgit;
            valarray<double> sumw(0.0, persistent.size()); // need m copies of this
            bool gap = true; // Check for gaps between the sub-windows.
            for ( size_t i = 0; i < x.size(); ++i  ) {
              // check equals comparisons here!
              if ( fillT2binT<T>(x[i].first) + wsize >= ehi
                   &&
                   fillT2binT<T>(x[i].first) - wsize <= elo ) {
                sumw += x[i].second * weights[i];
                gap = false;
              }
            }
            if ( gap ) continue;
            hfill.push_back( make_tuple( (ehi + elo)/2.0, sumw, (ehi - elo) ) );
            sumf += ehi - elo;
          }

          for ( auto f : hfill )
            for ( size_t m = 0; m < persistent.size(); ++m )
              persistent[m]->fill( get<0>(f), get<1>(f)[m], get<2>(f)/sumf );
            // Note the scaling to one single fill

    }

  }


  template<>
  void commit<YODA::Histo2D>(vector<YODA::Histo2D::Ptr> & persistent,
              const vector< vector<Fill<YODA::Histo2D>> > & tuple,
              const vector<valarray<double>> & weights)
  {}

  template<>
  void commit<YODA::Profile2D>(vector<YODA::Profile2D::Ptr> & persistent,
              const vector< vector<Fill<YODA::Profile2D>> > & tuple,
              const vector<valarray<double>> & weights)
  {}

    template <class T>
    double distance(T a, T b) {
      return abs(a - b);
    }

    template <>
    double distance<tuple<double,double> >(tuple<double,double> a, tuple<double,double> b) {
      return Rivet::sqr(get<0>(a) - get<0>(b)) + Rivet::sqr(get<1>(a) - get<1>(b));
    }

  }


  namespace Rivet {

    bool copyao(YODA::AnalysisObjectPtr src, YODA::AnalysisObjectPtr dst) {
      for (const std::string& a : src->annotations())
        dst->setAnnotation(a, src->annotation(a));
      if ( aocopy<Counter>(src,dst) ) return true;
      if ( aocopy<Histo1D>(src,dst) ) return true;
      if ( aocopy<Histo2D>(src,dst) ) return true;
      if ( aocopy<Profile1D>(src,dst) ) return true;
      if ( aocopy<Profile2D>(src,dst) ) return true;
      if ( aocopy<Scatter1D>(src,dst) ) return true;
      if ( aocopy<Scatter2D>(src,dst) ) return true;
      if ( aocopy<Scatter3D>(src,dst) ) return true;
      return false;
    }

    bool addaos(YODA::AnalysisObjectPtr dst, YODA::AnalysisObjectPtr src, double scale) {
      if ( aoadd<Counter>(dst,src,scale) ) return true;
      if ( aoadd<Histo1D>(dst,src,scale) ) return true;
      if ( aoadd<Histo2D>(dst,src,scale) ) return true;
      if ( aoadd<Profile1D>(dst,src,scale) ) return true;
      if ( aoadd<Profile2D>(dst,src,scale) ) return true;
      return false;
    }

  }


namespace {

/// fills is a vector of sub-event with an ordered set of x-values of
/// the fills in each sub-event. NOFILL should be an "impossible"
/// value for this histogram. Returns a vector of sub-events with
/// an ordered vector of fills (including NOFILLs) for each sub-event.
template <class T>
vector< vector<Fill<T> > >
match_fills(const vector<typename TupleWrapper<T>::Ptr> & evgroup, const Fill<T> & NOFILL)
{
  vector< vector<Fill<T> > > matched;
  // First just copy subevents into vectors and find the longest vector.
  unsigned int maxfill = 0; // length of biggest vector
  int imax = 0; // index position of biggest vector
  for ( const auto & it : evgroup ) {
    const auto & subev = it->fills();
    if ( subev.size() > maxfill ) {
      maxfill = subev.size();
      imax = matched.size();
    }
    matched.push_back(vector<Fill<T> >(subev.begin(), subev.end()));
  }
  // Now, go through all subevents with missing fills.
  const vector<Fill<T>> & full = matched[imax]; // the longest one
  for ( auto & subev : matched ) {
    if ( subev.size() == maxfill ) continue;

    // Add NOFILLs to the end;
    while ( subev.size() < maxfill ) subev.push_back(NOFILL);

    // Iterate from the back and shift all fill values backwards by
    // swapping with NOFILLs so that they better match the full
    // subevent.
    for ( int i = maxfill - 1; i >= 0; --i ) {
      if ( subev[i] == NOFILL ) continue;
      size_t j = i;
      while ( j + 1 < maxfill && subev[j + 1] == NOFILL &&
              distance(fillT2binT<T>(subev[j].first),
                       fillT2binT<T>(full[j].first))
              >
              distance(fillT2binT<T>(subev[j].first),
                       fillT2binT<T>(full[j + 1].first)) )
      {
            swap(subev[j], subev[j + 1]);
            ++j;
      }
    }
  }
  // transpose
  vector<vector<Fill<T>>> result(maxfill,vector<Fill<T>>(matched.size()));
  for (size_t i = 0; i < matched.size(); ++i)
      for (size_t j = 0; j < maxfill; ++j)
          result.at(j).at(i) = matched.at(i).at(j);
  return result;
}

}




namespace Rivet {

  template <class T>
  void Wrapper<T>::pushToPersistent(const vector<valarray<double> >& weight) {
      assert( _evgroup.size() == weight.size() );

      // have we had subevents at all?
      const bool have_subevents = _evgroup.size() > 1;
      if ( ! have_subevents ) {



        // simple replay of all tuple entries
        // each recorded fill is inserted into all persistent weightname histos
        for ( size_t m = 0; m < _persistent.size(); ++m )
            for ( const auto & f : _evgroup[0]->fills() )
                _persistent[m]->fill( f.first, f.second * weight[0][m] );



      } else {



        // outer index is subevent, inner index is jets in the event
        vector<vector<Fill<T>>> linedUpXs
            = match_fills<T>(_evgroup, {typename T::FillType(), 0.0});
        commit<T>( _persistent, linedUpXs, weight );



      }
      _evgroup.clear();
      _active.reset();
  }

  template <class T>
  void Wrapper<T>::pushToFinal() {
    for ( size_t m = 0; m < _persistent.size(); ++m ) {
      copyao(_persistent.at(m), _final.at(m));
    }
  }



  template <>
  void Wrapper<YODA::Counter>::pushToPersistent(const vector<valarray<double> >& weight) {
    for ( size_t m = 0; m < _persistent.size(); ++m ) {
      for ( size_t n = 0; n < _evgroup.size(); ++n ) {
        for ( const auto & f : _evgroup[n]->fills() ) {
          _persistent[m]->fill( f.second * weight[n][m] );
        }
      }
    }

    _evgroup.clear();
    _active.reset();
  }

  template <>
  void Wrapper<YODA::Scatter1D>::pushToPersistent(const vector<valarray<double> >& weight) {

    _evgroup.clear();
    _active.reset();
  }

  template <>
  void Wrapper<YODA::Scatter2D>::pushToPersistent(const vector<valarray<double> >& weight) {

    _evgroup.clear();
    _active.reset();
  }

  template <>
  void Wrapper<YODA::Scatter3D>::pushToPersistent(const vector<valarray<double> >& weight) {

    _evgroup.clear();
    _active.reset();
  }

  // explicitly instantiate all wrappers

  template class Wrapper<YODA::Histo1D>;
  template class Wrapper<YODA::Histo2D>;
  template class Wrapper<YODA::Profile1D>;
  template class Wrapper<YODA::Profile2D>;
  template class Wrapper<YODA::Counter>;
  template class Wrapper<YODA::Scatter1D>;
  template class Wrapper<YODA::Scatter2D>;
  template class Wrapper<YODA::Scatter3D>;

  AOPath::AOPath(string fullpath) {
    // First checck if This is a global system path
    _path = fullpath;
    std::regex resys("^(/RAW)?/([^\\[/]+)(\\[(.+)\\])?$");
    smatch m;
    _valid = regex_search(fullpath, m, resys);
    if ( _valid ) {
      _raw = (m[1] == "/RAW");
      _name = m[2];
      _weight = m[4];
      return;
    }
    // If not, assume it is a normal analysis path.
    std::regex repath("^(/RAW)?(/REF)?/([^/:]+)(:[^/]+)?(/TMP)?/([^\\[]+)(\\[(.+)\\])?");
    _valid = regex_search(fullpath, m, repath);
    if ( !_valid ) return;     
    _raw = (m[1] == "/RAW");
    _ref = (m[2] == "/REF");
    _analysis = m[3];
    _optionstring = m[4];
    _tmp = (m[5] == "/TMP");
    _name = m[6];
    _weight = m[8];
    std::regex reopt(":([^=]+)=([^:]+)");
    string s = _optionstring;
    while ( regex_search(s, m, reopt) ) {
      _options[m[1]] = m[2];
      s = m.suffix();
    }
  }

  void AOPath::fixOptionString() {
    ostringstream oss;
    for ( auto optval : _options )
      oss << ":" << optval.first << "=" << optval.second;
    _optionstring = oss.str();
  }

  string AOPath::mkPath() const {
    ostringstream oss;
    if ( isRaw() ) oss << "/RAW";
    else if ( isRef() ) oss << "/REF";
    if ( _analysis != "" ) oss << "/" << analysis();
    for ( auto optval : _options )
      oss << ":" << optval.first << "=" << optval.second;
    if ( isTmp() ) oss << "/TMP";
    oss << "/" << name();
    if ( weight() != "" )
      oss << "[" << weight() << "]";
    return oss.str();
  }

  void AOPath::debug() const {
    cout << "Full path:  " << _path << endl;
    if ( !_valid ) {
      cout << "This is not a valid analysis object path" << endl << endl;
      return;
    }
    cout << "Check path: " << mkPath() << endl;
    cout << "Analysis:   " << _analysis << endl;
    cout << "Name:       " << _name << endl;
    cout << "Weight:     " << _weight << endl;
    cout << "Properties: ";
    if ( _raw ) cout << "raw ";
    if ( _tmp ) cout << "tmp ";
    if ( _ref ) cout << "ref ";
    cout << endl;
    cout << "Options:    ";
    for ( auto opt : _options )
      cout << opt.first << "->" << opt.second << " ";
    cout << endl << endl;
  }

}
