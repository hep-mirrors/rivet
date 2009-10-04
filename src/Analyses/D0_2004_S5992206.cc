// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/PVertex.hh"
#include "Rivet/Projections/TotalVisibleMomentum.hh"

namespace Rivet {


  /* @brief D0 Run II jet analysis
   * @author Lars Sonnenschein
   * 
   * Measurement of angular correlations in di-jet events.
   * 
   * 
   * @par Run conditions
   * 
   * @arg \f$ \sqrt{s} = \f$ 1960 GeV
   * @arg Run with generic QCD events.
   * @arg Several \f$ p_\perp^\text{min} \f$ cutoffs are probably required to fill the histograms:
   *   @arg \f$ p_\perp^\text{min} = \f$ 50, 75, 100, 150 GeV for the four pT ranges respecively
   * 
   */ 
  class D0_2004_S5992206 : public Analysis {

  public:

    /// @name Constructors etc.
    //@{

    /// Constructor.
    D0_2004_S5992206() : Analysis("D0_2004_S5992206")
    {
      setBeams(PROTON, ANTIPROTON);
    }

    //@}


    /// @name Analysis methods
    //@{

    void init() {
      // Final state for jets, mET etc.
      const FinalState fs(-3.0, 3.0);
      addProjection(fs, "FS");
      addProjection(FastJets(fs, FastJets::D0ILCONE, 0.7, 6*GeV), "Jets");
      addProjection(TotalVisibleMomentum(fs), "CalMET");
      addProjection(PVertex(), "PV");
      
      // Veto neutrinos, and muons with pT above 1.0 GeV
      VetoedFinalState vfs(fs);
      vfs.vetoNeutrinos();
      vfs.addVetoDetail(MUON, 1.0, MAXDOUBLE);
      addProjection(vfs, "VFS");

      // Book histograms
      _histJetAzimuth_pTmax75_100  = bookHistogram1D(1, 2, 1);
      _histJetAzimuth_pTmax100_130 = bookHistogram1D(2, 2, 1);
      _histJetAzimuth_pTmax130_180 = bookHistogram1D(3, 2, 1);
      _histJetAzimuth_pTmax180_    = bookHistogram1D(4, 2, 1);
    }


    /// Do the analysis
    void analyze(const Event & event) {

      // Analyse and print some info
      const JetAlg& jetpro = applyProjection<JetAlg>(event, "Jets");
      getLog() << Log::DEBUG << "Jet multiplicity before any pT cut = " << jetpro.size() << endl;
      
      // Find vertex and check  that its z-component is < 50 cm from the nominal IP
      const PVertex& pv = applyProjection<PVertex>(event, "PV");
      if (fabs(pv.position().z())/cm > 50.0) vetoEvent;
      
      const Jets jets  = jetpro.jetsByPt(40.0*GeV);
      if (jets.size() >= 2) {
        getLog() << Log::DEBUG << "Jet multiplicity after pT > 40 GeV cut = " << jets.size() << endl; 
      } else {
        vetoEvent;
      }
      const double rap1 = jets[0].momentum().rapidity();
      const double rap2 = jets[1].momentum().rapidity();
      if (fabs(rap1) > 0.5 || fabs(rap2) > 0.5) {
        vetoEvent;
      }
      getLog() << Log::DEBUG << "Jet eta and pT requirements fulfilled" << endl;
      const double pT1 = jets[0].momentum().pT();
      
      const TotalVisibleMomentum& caloMissEt = applyProjection<TotalVisibleMomentum>(event, "CalMET");
      getLog() << Log::DEBUG << "Missing Et = " << caloMissEt.momentum().pT()/GeV << endl;
      if (caloMissEt.momentum().pT() > 0.7*pT1) {
        vetoEvent;
      }
      
      if (pT1/GeV >= 75.0) {
        const double weight = event.weight();
        const double dphi = deltaPhi(jets[0].momentum().phi(), jets[1].momentum().phi());
        if (inRange(pT1/GeV, 75.0, 100.0)) {
          _histJetAzimuth_pTmax75_100->fill(dphi, weight);
        } else if (inRange(pT1/GeV, 100.0, 130.0)) {
          _histJetAzimuth_pTmax100_130->fill(dphi, weight);
        } else if (inRange(pT1/GeV, 130.0, 180.0)) {
          _histJetAzimuth_pTmax130_180->fill(dphi, weight);
        } else if (pT1/GeV > 180.0) {
          _histJetAzimuth_pTmax180_->fill(dphi, weight);
        }
      }
      
    }
    
    
    // Finalize
    void finalize() { 
      // Normalize histograms to unit area
      normalize(_histJetAzimuth_pTmax75_100);
      normalize(_histJetAzimuth_pTmax100_130);
      normalize(_histJetAzimuth_pTmax130_180);
      normalize(_histJetAzimuth_pTmax180_);
    }
    
    //@}


  private:

    /// @name Histograms
    //@{
    AIDA::IHistogram1D* _histJetAzimuth_pTmax75_100;
    AIDA::IHistogram1D* _histJetAzimuth_pTmax100_130;
    AIDA::IHistogram1D* _histJetAzimuth_pTmax130_180;
    AIDA::IHistogram1D* _histJetAzimuth_pTmax180_;
    //@}

  };

    
    
  // This global object acts as a hook for the plugin system
  AnalysisBuilder<D0_2004_S5992206> plugin_D0_2004_S5992206;

}
