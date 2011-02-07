// -*- C++ -*-
// CDF Z pT analysis

#include "Rivet/Analysis.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/ZFinder.hh"

namespace Rivet {


  /// @brief CDF Run I Z \f$ p_\perp \f$ in Drell-Yan events
  /// @author Hendrik Hoeth
  class CDF_2000_S4155203 : public Analysis {
  public:

    /// Constructor
    CDF_2000_S4155203()
      : Analysis("CDF_2000_S4155203")
    {
      setBeams(PROTON, ANTIPROTON);
      setNeedsCrossSection(true);
    }


    /// @name Analysis methods
    //@{

    void init() {
      // Set up projections
      ZFinder zfinder(FinalState(), ELECTRON, 66.0*GeV, 116.0*GeV, 0.0, 0.0);
      addProjection(zfinder, "ZFinder");

      // Book histogram
      _hist_zpt = bookHistogram1D(1, 1, 1);
    }


    /// Do the analysis
    void analyze(const Event& e) {
      const ZFinder& zfinder = applyProjection<ZFinder>(e, "ZFinder");
      if (zfinder.particles().size() != 1) {
        getLog() << Log::DEBUG << "Num e+ e- pairs found = " << zfinder.particles().size() << endl;
        vetoEvent;
      }

      FourMomentum pZ = zfinder.particles()[0].momentum();
      if (pZ.mass2() < 0) {
        getLog() << Log::DEBUG << "Negative Z mass**2 = " << pZ.mass2()/GeV2 << "!" << endl;
        vetoEvent;
      }

      getLog() << Log::DEBUG << "Dilepton mass = " << pZ.mass()/GeV << " GeV"  << endl;
      getLog() << Log::DEBUG << "Dilepton pT   = " << pZ.pT()/GeV << " GeV" << endl;
      _hist_zpt->fill(pZ.pT()/GeV, e.weight());
    }


    void finalize() {
      scale(_hist_zpt, crossSection()/picobarn/sumOfWeights());
    }

    //@}


  private:

    AIDA::IHistogram1D *_hist_zpt;

  };



  // This global object acts as a hook for the plugin system
  AnalysisBuilder<CDF_2000_S4155203> plugin_CDF_2000_S4155203;

}
