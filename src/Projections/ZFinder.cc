// -*- C++ -*-
#include "Rivet/Projections/ZFinder.hh"
#include "Rivet/Projections/InvMassFinalState.hh"
#include "Rivet/Projections/LeptonClusters.hh"
#include "Rivet/Projections/VetoedFinalState.hh"
#include "Rivet/Tools/ParticleIdUtils.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/Cmp.hh"

namespace Rivet {


  ZFinder::ZFinder(double etaMin, double etaMax,
                   double pTmin,
                   PdgId pid,
                   double minmass, double maxmass,
                   double dRmax, bool clusterPhotons, bool trackPhotons,
                   double masstarget) {
    vector<pair<double, double> > etaRanges;
    etaRanges += std::make_pair(etaMin, etaMax);
    _init(etaRanges, pTmin, pid, minmass, maxmass, dRmax, clusterPhotons, trackPhotons, masstarget);
  }


  ZFinder::ZFinder(const std::vector<std::pair<double, double> >& etaRanges,
                   double pTmin,
                   PdgId pid,
                   double minmass, const double maxmass,
                   double dRmax, bool clusterPhotons, bool trackPhotons,
                   double masstarget) {
    _init(etaRanges, pTmin, pid, minmass, maxmass, dRmax, clusterPhotons, trackPhotons, masstarget);
  }


  void ZFinder::_init(const std::vector<std::pair<double, double> >& etaRanges,
                      double pTmin,  PdgId pid,
                      double minmass, double maxmass,
                      double dRmax, bool clusterPhotons, bool trackPhotons,
                      double masstarget)
  {
    setName("ZFinder");

    _minmass = minmass;
    _maxmass = maxmass;
    _masstarget = masstarget;
    _pid = pid;
    _trackPhotons = trackPhotons;

    FinalState fs;
    IdentifiedFinalState bareleptons(fs);
    bareleptons.acceptIdPair(pid);
    LeptonClusters leptons(fs, bareleptons, dRmax,
                           clusterPhotons,
                           etaRanges, pTmin);
    addProjection(leptons, "LeptonClusters");

    VetoedFinalState remainingFS;
    remainingFS.addVetoOnThisFinalState(*this);
    addProjection(remainingFS, "RFS");
  }


  /////////////////////////////////////////////////////


  const FinalState& ZFinder::remainingFinalState() const
  {
    return getProjection<FinalState>("RFS");
  }


  int ZFinder::compare(const Projection& p) const {
    PCmp LCcmp = mkNamedPCmp(p, "LeptonClusters");
    if (LCcmp != EQUIVALENT) return LCcmp;

    const ZFinder& other = dynamic_cast<const ZFinder&>(p);
    return (cmp(_minmass, other._minmass) || cmp(_maxmass, other._maxmass) ||
            cmp(_pid, other._pid) || cmp(_trackPhotons, other._trackPhotons));
  }


  void ZFinder::project(const Event& e) {
    clear();

    const LeptonClusters& leptons = applyProjection<LeptonClusters>(e, "LeptonClusters");

    InvMassFinalState imfs(FinalState(), std::make_pair(_pid, -_pid), _minmass, _maxmass, _masstarget);
    ParticleVector tmp;
    tmp.insert(tmp.end(), leptons.clusteredLeptons().begin(), leptons.clusteredLeptons().end());
    imfs.calc(tmp);

    if (imfs.particlePairs().size() < 1) return;
    ParticlePair Zconstituents(imfs.particlePairs()[0]);
    Particle l1(Zconstituents.first), l2(Zconstituents.second);
    _constituents += l1, l2;
    FourMomentum pZ = l1.momentum() + l2.momentum();
    const int z3charge = PID::threeCharge(l1.pdgId()) + PID::threeCharge(l2.pdgId());
    assert(z3charge == 0);

    stringstream msg;
    msg << "Z reconstructed from: " << endl
        << "   " << l1.momentum() << " " << l1.pdgId() << endl
        << " + " << l2.momentum() << " " << l2.pdgId() << endl;

    _bosons.push_back(Particle(ZBOSON, pZ));
    MSG_DEBUG(name() << " found one Z");

    // Find the LeptonClusters which survived the IMFS cut such that we can
    // extract their original particles
    foreach (const Particle& p, _constituents) {
      foreach (const ClusteredLepton& l, leptons.clusteredLeptons()) {
        if (p.pdgId()==l.pdgId() && p.momentum()==l.momentum()) {
          _theParticles.push_back(l.constituentLepton());
          if (_trackPhotons) {
            _theParticles.insert(_theParticles.end(),
                                 l.constituentPhotons().begin(),
                                 l.constituentPhotons().end());
          }
        }
      }
    }
  }


}
