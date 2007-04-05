// -*- C++ -*-
#ifndef RIVET_FinalState_H
#define RIVET_FinalState_H

#include "Rivet/Projections/Projection.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Event.hh"


namespace Rivet {

  /// Project out all final-state particles in an event.
  class FinalState: public Projection {
    
  public:
    
    /** @name Standard constructors and destructors. */
    //@{
    /**
     * The default constructor. May specify the minimum and maximum
     * pseudorapidity.
     */
    inline FinalState(double mineta = -MaxRapidity,
                      double maxeta = MaxRapidity,
                      double minpt = 0.0)
      : etamin(mineta), etamax(maxeta), ptmin(minpt) 
    { }
    
  public:
    /// Return the name of the projection
    inline string name() const {
      return "FinalState";
    }
    
  protected:
    
    /// Apply the projection to the event.
    void project(const Event & e);
    
    /// Compare projections.
    int compare(const Projection & p) const;
    
  public:
    
    /// Access the projected final-state particles.
    inline const ParticleVector & particles() const { return theParticles; }
    
  private:
    
    /// The minimum allowed pseudo-rapidity.
    double etamin;
    
    /// The maximum allowed pseudo-rapidity.
    double etamax;
    
    /// The minimum allowed transverse momentum.
    double ptmin;
    
    /// The final-state particles.
    ParticleVector theParticles;
    
  private:
    
    /// Hide the assignment operator.
    FinalState & operator=(const FinalState &);
    
  };
  
}


#endif /* RIVET_FinalState_H */
