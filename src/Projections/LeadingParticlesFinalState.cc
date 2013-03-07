#include "Rivet/Projections/LeadingParticlesFinalState.hh"
#include "Rivet/Particle.hh"

namespace Rivet {


  int LeadingParticlesFinalState::compare(const Projection& p) const {
    // First compare the final states we are running on
    int fscmp = mkNamedPCmp(p, "FS");
    if (fscmp != EQUIVALENT) return fscmp;

    // Then compare the two as final states
    const LeadingParticlesFinalState& other = dynamic_cast<const LeadingParticlesFinalState&>(p);
    fscmp = FinalState::compare(other);
    if (fscmp != EQUIVALENT) return fscmp;

    int locmp = cmp(_leading_only, other._leading_only);
    if (locmp != EQUIVALENT) return locmp;

    // Finally compare the IDs
    if (_ids < other._ids) return ORDERED;
    else if (other._ids < _ids) return UNORDERED;
    return EQUIVALENT;
  }


  void LeadingParticlesFinalState::project(const Event & e) {
    _theParticles.clear();
    const FinalState& fs = applyProjection<FinalState>(e, "FS");

    // Temporary container
    map<long, ParticleVector::const_iterator> tmp;

    const ParticleVector& particles = fs.particles();
    MSG_DEBUG("Original final state particles size " << particles.size());
    ParticleVector::const_iterator ifs;
    for (ifs = particles.begin(); ifs != particles.end(); ++ifs) {
      if (inList(*ifs) && FinalState::accept(*ifs->genParticle())) {
        // Look for an existing particle in tmp container
        map < long, ParticleVector::const_iterator >::const_iterator itmp = tmp.find(ifs->pdgId());
        if (itmp != tmp.end()) {  // if a particle with this type has been already selected
          // If the new pT is higher than the previous one, then substitute...
          if (ifs->momentum().pT() > itmp->second->momentum().pT()) {
            tmp[ifs->pdgId()] = ifs;
          }
          // ...otherwise insert in the container
        } else {
          tmp[ifs->pdgId()] = ifs;
        }
      }
    }

    // Loop on the tmp container and fill _theParticles
    map<long, ParticleVector::const_iterator>::const_iterator i;
    for (i = tmp.begin(); i != tmp.end(); ++i) {
      MSG_DEBUG("LeadingParticlesFinalState is accepting particle ID " << i->second->pdgId()
               << " with momentum " << i->second->momentum());
      _theParticles.push_back(*(i->second));
    }

    if (_leading_only) {
      double ptmax=0.0;
      Particle pmax;

      foreach (const Particle& p, _theParticles) {
        if (p.momentum().pT() > ptmax) {
          ptmax = p.momentum().pT();
          pmax = p;
        }
      }

      _theParticles.clear();
      _theParticles.push_back(pmax);
    }
  }


  bool LeadingParticlesFinalState::inList(const Particle & particle) const {
    std::set < long >::const_iterator ilist = _ids.find(particle.pdgId());
    if (ilist != _ids.end()) return true;
    return false;
  }


}
