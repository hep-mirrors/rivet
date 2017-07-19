// -*- C++ -*-
#ifndef RIVET_DressedLeptons_HH
#define RIVET_DressedLeptons_HH

#include "Rivet/Projection.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/IdentifiedFinalState.hh"
#include "Rivet/Config/RivetCommon.hh"

namespace Rivet {


  /// A charged lepton meta-particle created by clustering photons close to the bare lepton
  class DressedLepton : public Particle {
  public:

    DressedLepton(const Particle& lepton, const Particles& photons=Particles(), bool momsum=true)
      : Particle(lepton.pid(), lepton.momentum())
    {
      setConstituents({{lepton}}); //< bare lepton is first constituent
      addConstituents(photons, momsum);
    }

    /// Add a photon to the dressed lepton
    /// @todo Deprecate and override add/setConstituents instead?
    void addPhoton(const Particle& p, bool momsum=true) {
      if (p.pid() != PID::PHOTON) throw Error("Clustering a non-photon on to a DressedLepton");
      addConstituent(p, momsum);
    }

    /// Retrieve the bare lepton
    const Particle& bareLepton() const {
      const Particle& l = constituents().front();
      if (!l.isChargedLepton()) throw Error("First constituent of a DressedLepton is not a bare lepton: oops");
      return l;
    }
    /// Retrieve the bare lepton (alias)
    /// @deprecated Prefer the more physicsy bareLepton()
    const Particle& constituentLepton() const { return bareLepton(); }

    /// Retrieve the clustered photons
    const Particles photons() const { return slice(constituents(), 1); }
    /// Retrieve the clustered photons (alias)
    /// @deprecated Prefer the shorter photons()
    const Particles constituentPhotons() const { return photons(); }

  };


  /// @brief Cluster photons from a given FS to all charged particles (typically leptons)
  ///
  /// This stores the original (bare) charged particles and photons as particles()
  /// while the newly created clustered lepton objects are accessible as
  /// dressedLeptons(). The clustering is done by a delta(R) cone around each bare
  /// lepton, with double counting being avoided by only adding a photon to the _closest_
  /// bare lepton if it happens to be within the capture radius of more than one.
  class DressedLeptons : public FinalState {
  public:

    /// @brief Constructor with a single input FinalState (used for both photons and bare leptons)
    ///
    /// Provide a single final state projection used to select the photons and bare
    /// leptons, a photon-clustering delta(R) cone size around each bare lepton, and an optional
    /// cut on the _dressed_ leptons (i.e. the momenta after clustering).
    /// The final argument controls whether non-prompt photons are to be included.
    /// Set the clustering radius to 0 or negative to disable clustering.
    DressedLeptons(const FinalState& barefs,
                   double dRmax, const Cut& cut=Cuts::open(),
                   bool useDecayPhotons=false);

    /// @brief Constructor with distinct photon and lepton FinalStates
    ///
    /// Provide final state projections used to select the photons and bare
    /// leptons (wish we had put the first two args the other way around...),
    /// a clustering delta(R) cone size around each bare lepton, and an optional
    /// cut on the _dressed_ leptons (i.e. the momenta after clustering.)
    /// The final argument controls whether non-prompt photons are to be included.
    /// Set the clustering radius to 0 or negative to disable clustering.
    DressedLeptons(const FinalState& photons, const FinalState& bareleptons,
                   double dRmax, const Cut& cut=Cuts::open(),
                   bool useDecayPhotons=false);


    /// Clone this projection
    DEFAULT_RIVET_PROJ_CLONE(DressedLeptons);

    /// @brief Retrieve the dressed leptons
    /// @note Like particles() but with helper functions
    vector<DressedLepton> dressedLeptons() const {
      vector<DressedLepton> rtn;
      for (const Particle& p : particles())
        rtn += static_cast<const DressedLepton>(p);
      return rtn;
    }


  protected:

    /// Apply the projection on the supplied event.
    void project(const Event& e);

    /// Compare projections.
    int compare(const Projection& p) const;


  private:

    /// Maximum cone radius to find photons in
    double _dRmax;

    /// Whether to include photons from hadron (particularly pi0) and hadronic tau decays
    bool _fromDecay;

  };



}


#endif
