// -*- C++ -*-
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/JetShape.hh"
#include "Rivet/Cmp.hh"


namespace Rivet {

  int JetShape::compare(const Projection& p) const {
    const JetShape& other = dynamic_cast<const JetShape&>(p);
    if (_jetaxes.size() != other._jetaxes.size()) return 1;
    /// @todo Add comparison of vecs
    // for (size_t i = 0; i < _jetaxes.size(); ++i) {
    //   if (_jetaxes[i] != other._jetaxes[i]) return 1;
    // }
    return 0;
  }


  void JetShape::project(const Event& e) {
    Log& log = getLog();
    
    // Project into final state
    //const FinalState& fs = e.applyProjection(*_fsproj);
    // Project into vetoed final state if not done yet (supposed to be done by jet algorithm)
    const VetoedFinalState& vfs = e.applyProjection(_vfsproj);
    
    // Clear for each event and resize with zero vectors
    /// @todo Eliminate pointers to simplify this sort of thing:
    for (size_t i = 0; i < _diffjetshapes.size(); ++i) {
      _diffjetshapes[i].clear();
    }
    _diffjetshapes.clear();

    for (size_t i=0; i<_jetaxes.size(); ++i) {
      vector<double> diffjetshape(_nbins, 0.);
      _diffjetshapes.push_back(diffjetshape);
      //AIDA::IHistogram1D elem("diff", "diff", _nbin, _rmin, _rmax);
      //_diffjetshapes.push_back(elem);
    }
    //_diffjetshapes.resize(_jetaxes.size());


    /// @todo Simplify
    for (size_t i = 0; i < _intjetshapes.size(); ++i) {
      _intjetshapes[i].clear();
    }
    _intjetshapes.clear();

    for (size_t i = 0; i < _jetaxes.size(); ++i) {
      vector<double> intjetshape(_nbins, 0.);
      _intjetshapes.push_back(intjetshape);
      //AIDA::IHistogram1D ele("int", "int", _nbin, _rmin, _rmax);
      //_intjetshapes.push_back(ele);
    }
    //_intjetshapes.resize(_jetaxes.size());
    
    _PsiSlot.clear();
    _PsiSlot.resize(_jetaxes.size(), 0.);
    //_PsiSlot.reset();
    
    
    // Determine jet shapes
    double y1, y2, eta1, eta2, phi1, phi2, drad;
    /// @todo Fix!
    double dradmin=TWOPI; //dummy assignment, to avoid compile warning
    int dradminind=0; //dummy asignment, to avoid compile warning
    if (_jetaxes.size()>0) {
      for (ParticleVector::const_iterator p = vfs.particles().begin(); p != vfs.particles().end(); ++p) {
        
        for (size_t j = 0; j < _jetaxes.size(); j++) {
          y1 = _jetaxes[j].rapidity();
          y2 = p->getMomentum().rapidity();
          eta1 = _jetaxes[j].vector3().pseudorapidity();
          eta2 = p->getMomentum().vector3().pseudorapidity();
          phi1 = _jetaxes[j].vector3().azimuthalAngle();
          phi2 = p->getMomentum().vector3().azimuthalAngle();
          
          if (_distscheme==SNOWMASS) {
            drad = delta_rad(eta1, phi1, eta2, phi2);
          } else { //_distscheme = ENERGY
            drad = delta_rad(y1, phi1, y2, phi2);
          }
          
          if (j == 0 || drad < dradmin) {
            dradminind = j;
            dradmin = drad;
          }
        }
        
        for (size_t i = 0; i < _nbins; ++i) {
          if (dradmin<_rmin+(i+1)*_interval) {
            //cout << "dradmin=" << dradmin << " < _rmin+(i+1)*_interval=" << _rmin+(i+1)*_interval << endl;
            _intjetshapes[dradminind][i] += p->getMomentum().vector3().polarRadius();
            //_intjetshapes[dradminind].fill(_rmin+(i+0.5)*_interval, p->getMomentum().perp());
            if (dradmin>_rmin+i*_interval) {
              _diffjetshapes[dradminind][i] += p->getMomentum().vector3().polarRadius()/_interval;
              //_diffjetshapes[dradminind].fill(_rmin+(i+0.5)*_interval, p->getMomentum().perp()/_interval);
            }
          }
        }
        
        if (dradmin < _r1minPsi/_rmax) {
          _PsiSlot[dradminind] += p->getMomentum().vector3().polarRadius();
          //_PsiSlot.fill(dradminind+0.5, p->getMomentum().perp()); //x=i(jet)
        }
        
      }
      
      
      // Normalize to total pT
      for (size_t j = 0; j < _jetaxes.size(); j++) {
        //if (_intjetshapes[j].binHeight(_nbins-1) > 0.) {
        if (_intjetshapes[j][_nbins-1] > 0.) {
          //_PsiShape[j] = 1.-_PsiShape[j]/_intjetshapes[j][_nbins-1];
          _PsiSlot[j] /= _intjetshapes[j][_nbins-1];
          ////_PsiSlot.scale(1./_intjetshapes[j].binHeight(_nbins-1));
          //for (unsigned int j=0; j<_jetaxes.size(); j++) {
          for (size_t i = 0; i < _nbins; ++i) {
            _diffjetshapes[j][i] /= _intjetshapes[j][_nbins-1];
            //_diffjetshapes[j].scale(1./_intjetshapes[j].binHeight(_nbins-1));
            _intjetshapes[j][i] /= _intjetshapes[j][_nbins-1];
            //_intjetshapes[j].scale(1./_intjetshapes[j].binHeight(_nbins-1));
          }
          //}
        }
      }
      
    }
    log << Log::DEBUG << "Done" << endl;
  }
  
  
}
