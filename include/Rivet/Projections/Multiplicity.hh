// -*- C++ -*-
#ifndef RIVET_Multiplicity_HH
#define RIVET_Multiplicity_HH
// Declaration of the Multiplicity class.

#include "Rivet/Projections/Projection.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Tools/Event/Particle.hh"
#include "Rivet/Tools/Event/Event.hh"


namespace Rivet {

  /**
   * Project out all final-state particles in an event.
   */
  class Multiplicity : public Projection {

  public:

    /// @name Standard constructors and destructors.  @{ / Default
    //constructor. Must specify a FinalState projection which is
    //assumed to live throughout the run.
    inline Multiplicity(FinalState & fsp);

    /// Copy constructor.
    inline Multiplicity(const Multiplicity &);

    /// Destructor.
    virtual ~Multiplicity();
    //@}

  protected:

    /// Perform the projection on the Event: only to be called by 
    /// Event::addProjection(Projection &).
    void project(const Event & e);

    /// This function defines a unique ordering between different 
    /// Projection objects of the same class. Should only be called from 
    /// operator<(const Projection &).
    int compare(const Projection & p) const;

  public:

    /// @name Access the projected multiplicities.
    //@ {
    /// Total multiplicity
    inline const unsigned int totalMultiplicity() const;

    /// Charged multiplicity
    inline const unsigned int totalChargedMultiplicity() const;

    /// Uncharged multiplicity
    inline const unsigned int totalUnchargedMultiplicity() const;

    /// Hadron multiplicity
    inline const unsigned int hadronMultiplicity() const;

    /// Hadronic charged multiplicity
    inline const unsigned int hadronChargedMultiplicity() const;

    /// Hadronic uncharged multiplicity
    inline const unsigned int hadronUnchargedMultiplicity() const;
    //@ }

  /**
   * Return the RivetInfo object of this Projection. Derived classes
   * should re-implement this function to return the combined
   * RivetInfo object of this and of any other Projection upon which
   * this depends.
   */
  virtual RivetInfo getInfo() const;

  private:

    /// Total multiplicities
    unsigned int totalMult_, totalChMult_, totalUnchMult_;

    /// Hadronic multiplicities
    unsigned int hadMult_, hadChMult_, hadUnchMult_;

    /// The FinalState projection used by this projection
    FinalState * fsproj;

  private:

    /**
     * The assignment operator is private and must never be called.
     * In fact, it shouldn't even be implemented.
     */
    Multiplicity & operator=(const Multiplicity &);

  };

}

#include "Rivet/Projections/Multiplicity.icc"

#endif /* RIVET_Multiplicity_HH */
