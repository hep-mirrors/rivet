// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/RivetYODA.hh"
#include "Rivet/Tools/ParticleIdUtils.hh"
#include "Rivet/Projections/Beam.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "fastjet/JadePlugin.hh"

namespace Rivet {


  /// @brief OPAL photon production
  /// @author Peter Richardson
  class OPAL_1993_S2692198 : public Analysis {
  public:

    /// Constructor
    OPAL_1993_S2692198() : Analysis("OPAL_1993_S2692198")
    {

    }


    /// @name Analysis methods
    //@{

    void analyze(const Event& e) {
      // First, veto on leptonic events by requiring at least 4 charged FS particles
//       const ChargedFinalState& cfs = applyProjection<ChargedFinalState>(e, "CFS");
//       const size_t numParticles = cfs.particles().size();

//       if (numParticles < 4) {
//         MSG_DEBUG("Failed ncharged cut");
//         vetoEvent;
//       }
//       MSG_DEBUG("Passed ncharged cut");

      // Get event weight for histo filling
      const double weight = e.weight();

      // extract the photons
      ParticleVector photons;
      ParticleVector nonPhotons;
      FourMomentum ptotal;
      const FinalState& fs = applyProjection<FinalState>(e, "FS");
      foreach (const Particle& p, fs.particles()) {
        ptotal+= p.momentum();
        if(p.pdgId()==PHOTON) {
          photons.push_back(p);
        }
        else {
          nonPhotons.push_back(p);
        }
      }
      // no photon return but still count for normalisation
      if(photons.empty()) return;
      // definition of the Durham algorithm
      fastjet::JetDefinition durham_def(fastjet::ee_kt_algorithm,fastjet::E_scheme,
                                        fastjet::Best);
      // definition of the JADE algorithm
      fastjet::JadePlugin jade;
      fastjet::JetDefinition jade_def = fastjet::JetDefinition(&jade);
      // now for the weird jet algorithm
      double evis = ptotal.mass();
      vector<fastjet::PseudoJet> input_particles;
      // pseudo jets from the non photons
      foreach (const Particle& p,  nonPhotons) {
        input_particles.push_back(fastjet::PseudoJet(p.momentum().px(),
                                                     p.momentum().py(),
                                                     p.momentum().pz(),
                                                     p.momentum().E()));
      }
      // pseudo jets from all bar the first photon
      for(unsigned int ix=1;ix<photons.size();++ix) {
        input_particles.push_back(fastjet::PseudoJet(photons[ix].momentum().px(),
                                                     photons[ix].momentum().py(),
                                                     photons[ix].momentum().pz(),
                                                     photons[ix].momentum().E()));
      }
      // now loop over the photons
      for(unsigned int ix=0;ix<photons.size();++ix) {
        FourMomentum pgamma = photons[ix].momentum();
        // run the jet clustering DURHAM
        fastjet::ClusterSequence clust_seq(input_particles, durham_def);
        // cluster the jets
        for (size_t j = 0; j < _nPhotonDurham->numBins(); ++j) {
          bool accept(true);
          double ycut = _nPhotonDurham->bin(j).midpoint();
          double dcut = sqr(evis)*ycut;
          vector<fastjet::PseudoJet> exclusive_jets =
            sorted_by_E(clust_seq.exclusive_jets(dcut));
          for(unsigned int iy=0;iy<exclusive_jets.size();++iy) {
            FourMomentum pjet(momentum(exclusive_jets[iy]));
            double cost = pjet.vector3().unit().dot(pgamma.vector3().unit());
            double ygamma = 2.*min(sqr(pjet.E()/evis),
                                   sqr(pgamma.E()/evis))*(1.-cost);
            if(ygamma<ycut) {
              accept = false;
              break;
            }
          }
          if(!accept) continue;
          _nPhotonDurham->fill(ycut, weight*_nPhotonDurham->bin(j).width());
          int njet = min(4,int(exclusive_jets.size())) - 1;
          if(j<_nPhotonJetDurham[njet]->numBins()) {
            _nPhotonJetDurham[njet]->fill(_nPhotonJetDurham[njet]->bin(j).midpoint(), weight*_nPhotonJetDurham[njet]->bin(j).width());
          }
        }
        // run the jet clustering JADE
        fastjet::ClusterSequence clust_seq2(input_particles, jade_def);
        for (size_t j = 0; j < _nPhotonJade->numBins(); ++j) {
          bool accept(true);
          double ycut = _nPhotonJade->bin(j).midpoint();
          double dcut = sqr(evis)*ycut;
          vector<fastjet::PseudoJet> exclusive_jets =
            sorted_by_E(clust_seq2.exclusive_jets(dcut));
          for(unsigned int iy=0;iy<exclusive_jets.size();++iy) {
            FourMomentum pjet(momentum(exclusive_jets[iy]));
            double cost = pjet.vector3().unit().dot(pgamma.vector3().unit());
            double ygamma = 2.*pjet.E()*pgamma.E()/sqr(evis)*(1.-cost);
            if(ygamma<ycut) {
              accept = false;
              break;
            }
          }
          if(!accept) continue;
          _nPhotonJade->fill(ycut, weight*_nPhotonJade->bin(j).width());
          int njet = min(4,int(exclusive_jets.size())) - 1;
          if(j<_nPhotonJetJade[njet]->numBins()) {
            _nPhotonJetJade[njet]->fill(_nPhotonJetJade[njet]->bin(j).midpoint(), weight*_nPhotonJetJade[njet]->bin(j).width());
          }
        }
        // add this photon back in for the next interation of the loop
        if(ix+1!=photons.size())
          input_particles[nonPhotons.size()+ix] =
            fastjet::PseudoJet(photons[ix].momentum().px(),photons[ix].momentum().py(),
                               photons[ix].momentum().pz(),photons[ix].momentum().E());
      }
    }


    void init() {
      // Projections
      addProjection(FinalState(), "FS");
//       addProjection(ChargedFinalState(), "CFS");

      // Book data sets
      _nPhotonJade       = bookHisto1D(1, 1, 1);
      _nPhotonDurham     = bookHisto1D(2, 1, 1);
      for(unsigned int ix=0;ix<4;++ix) {
        _nPhotonJetJade  [ix] = bookHisto1D(3 , 1, 1+ix);
        _nPhotonJetDurham[ix] = bookHisto1D(4 , 1, 1+ix);
      }
    }


    /// Finalize
    void finalize() {
      const double fact = 1000./sumOfWeights();
      scale(_nPhotonJade  ,fact);
      scale(_nPhotonDurham,fact);
      for(unsigned int ix=0;ix<4;++ix) {
        scale(_nPhotonJetJade  [ix],fact);
        scale(_nPhotonJetDurham[ix],fact);
      }
    }

    //@}

  private:

    Histo1DPtr _nPhotonJade;
    Histo1DPtr _nPhotonDurham;
    Histo1DPtr _nPhotonJetJade  [4];
    Histo1DPtr _nPhotonJetDurham[4];

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(OPAL_1993_S2692198);

}
