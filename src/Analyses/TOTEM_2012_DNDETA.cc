// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Projections/ChargedFinalState.hh"

namespace Rivet {


  class TOTEM_2012_DNDETA : public Analysis {
  public:

    TOTEM_2012_DNDETA()
      : Analysis("TOTEM_2012_DNDETA")
    {    }


  public:

    void init() {
      ChargedFinalState cfsm(-6.50, -5.35, 40.*MeV);
      ChargedFinalState cfsp( 5.35,  6.50, 40.*MeV);
      addProjection(cfsm, "CFSM");
      addProjection(cfsp, "CFSP");

      _h_eta = bookHistogram1D(1, 1, 1);
      _sumofweights = 0.;
    }


    void analyze(const Event& event) {
      const double weight = event.weight();

      const ChargedFinalState cfsm = applyProjection<ChargedFinalState>(event, "CFSM");
      const ChargedFinalState cfsp = applyProjection<ChargedFinalState>(event, "CFSP");

      if (cfsm.size() == 0 && cfsp.size() == 0) vetoEvent;

      _sumofweights += weight;

      foreach (const Particle& p, cfsm.particles() + cfsp.particles()) {
        _h_eta->fill(fabs(p.momentum().eta()), weight);
      }

    }


    void finalize() {
      scale(_h_eta, 1./(2*_sumofweights));
    }


  private:

    double _sumofweights;
    AIDA::IHistogram1D *_h_eta;


  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(TOTEM_2012_DNDETA);

}