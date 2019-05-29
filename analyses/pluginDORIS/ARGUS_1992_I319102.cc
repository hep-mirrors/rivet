// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"

namespace Rivet {


  /// @brief Add a short analysis description here
  class ARGUS_1992_I319102 : public Analysis {
  public:

    /// Constructor
    DEFAULT_RIVET_ANALYSIS_CTOR(ARGUS_1992_I319102);


    /// @name Analysis methods
    //@{

    /// Book histograms and initialise projections before the run
    void init() {

      // Initialise and register projections
      declare(FinalState(), "FS");

      // Book histograms
      if(fuzzyEquals(sqrtS()/GeV, 10.47 , 1E-3))
	_h_N = bookHisto1D(2, 1, 1);
      else if(fuzzyEquals(sqrtS()/GeV, 10.575, 1E-3))
	_h_N = bookHisto1D(3, 1, 1);
      // counters for R
      _c_hadrons = bookCounter("/TMP/sigma_hadrons");
      _c_muons   = bookCounter("/TMP/sigma_muons");
    }


    /// Perform the per-event analysis
    void analyze(const Event& event) {
      const FinalState& fs = apply<FinalState>(event, "FS");

      map<long,int> nCount;
      int ntotal(0);
      unsigned int nCharged(0);
      foreach (const Particle& p, fs.particles()) {
	nCount[p.pdgId()] += 1;
	++ntotal;
	if(PID::isCharged(p.pdgId())) ++nCharged;
      }
      // mu+mu- + photons
      if(nCount[-13]==1 and nCount[13]==1 &&
	 ntotal==2+nCount[22])
	_c_muons->fill(event.weight());
      // everything else
      else {
	_c_hadrons->fill(event.weight());
	if(_h_N)
	  _h_N->fill(nCharged,event.weight());
      }
    }


    /// Normalise histograms etc., after the run
    void finalize() {
      Scatter1D R = *_c_hadrons/ *_c_muons;
      double              rval = R.point(0).x();
      pair<double,double> rerr = R.point(0).xErrs();
      double fact = crossSection()/ sumOfWeights() /picobarn;
      double sig_h = _c_hadrons->val()*fact;
      double err_h = _c_hadrons->err()*fact;
      double sig_m = _c_muons  ->val()*fact;
      double err_m = _c_muons  ->err()*fact;
      Scatter2D temphisto(refData(1, 1, 1));
      Scatter2DPtr hadrons  = bookScatter2D("sigma_hadrons");
      Scatter2DPtr muons    = bookScatter2D("sigma_muons"  );
      Scatter2DPtr     mult = bookScatter2D(1, 1, 1);
      for (size_t b = 0; b < temphisto.numPoints(); b++) {
	const double x  = temphisto.point(b).x();
	pair<double,double> ex = temphisto.point(b).xErrs();
	pair<double,double> ex2 = ex;
	if(ex2.first ==0.) ex2. first=0.0001;
	if(ex2.second==0.) ex2.second=0.0001;
	if (inRange(sqrtS()/GeV, x-ex2.first, x+ex2.second)) {
	  mult   ->addPoint(x, rval, ex, rerr);
	  hadrons->addPoint(x, sig_h, ex, make_pair(err_h,err_h));
	  muons  ->addPoint(x, sig_m, ex, make_pair(err_m,err_m));
	}
	else {
	  mult   ->addPoint(x, 0., ex, make_pair(0.,.0));
	  hadrons->addPoint(x, 0., ex, make_pair(0.,.0));
	  muons  ->addPoint(x, 0., ex, make_pair(0.,.0));
	}
      }
      if(_h_N) {
	normalize(_h_N,200.);
      }
    }

    //@}


    /// @name Histograms
    //@{
    Histo1DPtr _h_N;
    CounterPtr _c_hadrons, _c_muons;
    //@}


  };


  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(ARGUS_1992_I319102);


}
