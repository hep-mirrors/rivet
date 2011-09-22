// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/ZFinder.hh"
#include "Rivet/Particle.fhh"

namespace Rivet {


  /// @brief ATLAS Z pT in Drell-Yan events at 7 TeV
  /// @author Elena Yatsenko, Judith Katzy
  class ATLAS_2011_S9131140 : public Analysis {
  public:

    /// Constructor
    ATLAS_2011_S9131140()
      : Analysis("ATLAS_2011_S9131140")
    {
      _sumw_el_bare = 0;
      _sumw_el_dressed = 0;
      _sumw_mu_bare = 0;
      _sumw_mu_dressed = 0;
    }


    /// @name Analysis methods
    //@{

    void init() {

      // Set up projections
      ZFinder zfinder_dressed_el(-2.4, 2.4, 20, ELECTRON, 66.0*GeV, 116.0*GeV, 0.1, true, false);
      addProjection(zfinder_dressed_el, "ZFinder_dressed_el");
      ZFinder zfinder_bare_el(-2.4, 2.4, 20, ELECTRON, 66.0*GeV, 116.0*GeV, 0.1, true, false);
      addProjection(zfinder_bare_el, "ZFinder_bare_el");
      ZFinder zfinder_dressed_mu(-2.4, 2.4, 20, MUON, 66.0*GeV, 116.0*GeV, 0.1, true, false);
      addProjection(zfinder_dressed_mu, "ZFinder_dressed_mu");
      ZFinder zfinder_bare_mu(-2.4, 2.4, 20, MUON, 66.0*GeV, 116.0*GeV, 0.0, true, false);
      addProjection(zfinder_bare_mu, "ZFinder_bare_mu");

      // Book histograms
      _hist_zpt_el_dressed     = bookHistogram1D(1, 1, 2);  // electron "dressed"
      _hist_zpt_el_bare        = bookHistogram1D(1, 1, 3);  // electron "bare"
      _hist_zpt_mu_dressed     = bookHistogram1D(2, 1, 2);  // muon "dressed"
      _hist_zpt_mu_bare        = bookHistogram1D(2, 1, 3);  // muon "bare"
    }


    /// Do the analysis
    void analyze(const Event& evt) {
      const double weight = evt.weight();

      const ZFinder& zfinder_dressed_el = applyProjection<ZFinder>(evt, "ZFinder_dressed_el");
      if (!zfinder_dressed_el.bosons().empty()) {
        _sumw_el_dressed += weight;
        const FourMomentum pZ = zfinder_dressed_el.bosons()[0].momentum();
        _hist_zpt_el_dressed->fill(pZ.pT()/GeV, weight);
      }

      const ZFinder& zfinder_bare_el = applyProjection<ZFinder>(evt, "ZFinder_bare_el");
      if (!zfinder_bare_el.bosons().empty()) {
        _sumw_el_bare += weight;
	    const FourMomentum pZ = zfinder_bare_el.bosons()[0].momentum();
        _hist_zpt_el_bare->fill(pZ.pT()/GeV, weight);
      }

      const ZFinder& zfinder_dressed_mu = applyProjection<ZFinder>(evt, "ZFinder_dressed_mu");
      if (!zfinder_dressed_mu.bosons().empty()) {
        _sumw_mu_dressed += weight;
        const FourMomentum pZ = zfinder_dressed_mu.bosons()[0].momentum();
        _hist_zpt_mu_dressed->fill(pZ.pT()/GeV, weight);
      }

      const ZFinder& zfinder_bare_mu = applyProjection<ZFinder>(evt, "ZFinder_bare_mu");
      if (zfinder_bare_mu.bosons().empty()) {
        _sumw_mu_bare += weight;
        const FourMomentum pZ = zfinder_bare_mu.bosons()[0].momentum();
        _hist_zpt_mu_bare->fill(pZ.pT()/GeV, weight);
      }

    }


    void finalize() {
      scale(_hist_zpt_el_dressed, 1/_sumw_el_dressed);
      scale(_hist_zpt_el_bare, 1/_sumw_el_bare);
      scale(_hist_zpt_mu_dressed, 1/_sumw_mu_dressed);
      scale(_hist_zpt_mu_bare, 1/_sumw_mu_bare);
    }

    //@}


    private:

	double _sumw_el_bare, _sumw_el_dressed;
	double _sumw_mu_bare, _sumw_mu_dressed;

	AIDA::IHistogram1D* _hist_zpt_el_dressed;
	AIDA::IHistogram1D* _hist_zpt_el_bare;
	AIDA::IHistogram1D* _hist_zpt_mu_dressed;
	AIDA::IHistogram1D* _hist_zpt_mu_bare;
  };


  // Hook for the plugin system
  DECLARE_RIVET_PLUGIN(ATLAS_2011_S9131140);

}