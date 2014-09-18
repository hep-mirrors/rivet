// -*- C++ -*-
#include "Rivet/Analyses/MC_ParticleAnalysis.hh"
#include "Rivet/Projections/IdentifiedFinalState.hh"

namespace Rivet {

  using namespace Cuts;


  /// @brief MC validation analysis for electrons
  class MC_ELECTRONS : public MC_ParticleAnalysis {
  public:

    MC_ELECTRONS()
      : MC_ParticleAnalysis("MC_ELECTRONS", 2, "electron")
    {    }


  public:

    void init() {
      IdentifiedFinalState electrons;
      electrons.acceptIdPair(PID::ELECTRON);
      addProjection(electrons, "Electrons");

      MC_ParticleAnalysis::init();
    }


    void analyze(const Event& event) {
      const Particles es = applyProjection<FinalState>(event, "Electrons").particlesByPt(pT>=0.5*GeV);
      MC_ParticleAnalysis::_analyze(event, es);
    }


    void finalize() {
      MC_ParticleAnalysis::finalize();
    }

  };


  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MC_ELECTRONS);

}
