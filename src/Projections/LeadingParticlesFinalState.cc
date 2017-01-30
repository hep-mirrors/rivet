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
    const Particles& particles = fs.particles();
    MSG_DEBUG("Original final state particles size " << particles.size());

    map<PdgId, const Particle*> tmp;
    for (const Particle& p : particles) {
      const PdgId pid = p.pid();
      // If it's a PID we're looking for, and passes the cuts
      if (_ids.find(pid) != _ids.end() && FinalState::accept(p.genParticle())) {
        // Look for an existing particle in tmp container
        if (tmp.find(pid) != tmp.end()) { // if a particle with this type has been already selected
          const Particle& p2 = *tmp.find(pid)->second;
          // If the new pT is higher than the previous one, then substitute...
          if (p.pT() > p2.pT()) tmp[pid] = &p;
        } else {
          // ...otherwise insert in the container
          tmp[pid] = &p;
        }
      }
    }

    // Loop on the tmp container and fill _theParticles
    for (const pair<PdgId,const Particle*>& id_p : tmp) {
      MSG_DEBUG("Accepting particle ID " << id_p.first << " with momentum " << id_p.second->momentum());
      _theParticles.push_back(*id_p.second);
    }

    if (_leading_only && !_theParticles.empty()) {
      sortByPt(_theParticles);
      const Particle pmax = _theParticles.front();
      _theParticles.clear();
      _theParticles.push_back(pmax);
    }
  }


}
