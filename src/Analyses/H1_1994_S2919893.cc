// -*- C++ -*-
#include "Rivet/Rivet.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/Math/Constants.hh"
#include "Rivet/Analyses/H1_1994_S2919893.hh"
#include "Rivet/Tools/ParticleIDMethods.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/DISKinematics.hh"

namespace Rivet {


  // Constructor
  H1_1994_S2919893::H1_1994_S2919893() {
    setBeams(ELECTRON, PROTON);
    addProjection(DISLepton(), "Lepton");
    addProjection(DISKinematics(), "Kinematics");
    addProjection(FinalState(), "FS");
  }


  void H1_1994_S2919893::analyze(const Event& event) {
        
    const FinalState& fs = applyProjection<FinalState>(event, "FS");
    const DISKinematics& dk = applyProjection<DISKinematics>(event, "Kinematics");
    const DISLepton& dl = applyProjection<DISLepton>(event,"Lepton");
    
    // Get the DIS kinematics
    double x  = dk.x();
    double w2 = dk.W2();
    double w = sqrt(w2);

    // Momentum of the scattered lepton
    FourMomentum leptonMom = dl.out().momentum();
    double ptel = pT(leptonMom);
    double enel = leptonMom.E();
    double thel = leptonMom.angle(dk.beamHadron().momentum())/degree;
    
    // Extract the particles other than the lepton
    ParticleVector particles;
    particles.reserve(fs.particles().size());
    const GenParticle& dislepGP = dl.out().genParticle();
    foreach (const Particle& p, fs.particles()) {
      const GenParticle& loopGP = p.genParticle(); 
      if (&loopGP == &dislepGP) continue;
      particles.push_back(p);
    }
    
    // Cut on the forward energy
    double efwd = 0.0;
    foreach (const Particle& p, particles) {
      double th = p.momentum().angle(dk.beamHadron().momentum())/degree;
      if (th > 4.4 && th < 15.) {
        efwd += p.momentum().E();
      }
    }

    // Apply the cuts
    // Lepton energy and angle, w2 and forward energy
    getLog()<<Log::DEBUG<<"enel/GeV = "<<enel/GeV<<", thel = "<<thel<<", w2 = "<<w2<<", efwd/GeV = "<<efwd/GeV<<std::endl;
    bool cut = enel/GeV > 14. && thel > 157. && thel < 172.5 && w2 >= 3000. && efwd/GeV > 0.5;
    if (!cut) vetoEvent(event);

    // Weight of the event
    const double weight = event.weight();
    // weights for x<1e-3 and x>1e-3
    if (x < 1e-3) {
      _wEnergy.first  += weight;
    } else {
      _wEnergy.second += weight;
    }

    // Boost to hadronic CM
    const LorentzTransform hcmboost = dk.boostHCM();
    // Loop over the particles
    long ncharged(0);
    for (size_t ip1 = 0; ip1 < particles.size(); ++ip1) {
      const Particle& p = particles[ip1];

      double th = p.momentum().angle(dk.beamHadron().momentum()) / degree;
      // Boost momentum to lab
      const FourMomentum hcmMom = hcmboost.transform(p.momentum());
      // Angular cut
      if (th <= 4.4) continue;

      // Energy flow histogram
      double et = fabs(Et(hcmMom));
      double eta = -hcmMom.pseudorapidity(); 
      if (x < 1e-3) {
        _histEnergyFlowLowX ->fill(eta, et*weight);
      } else {
        _histEnergyFlowHighX->fill(eta, et*weight);
      }
      if (PID::threeCharge(p.pdgId()) != 0) {
        /// @todo Use units in w comparisons... what are the units?
        if (w > 50. && w <= 200.) {
          double xf= -2 * hcmMom.z() / w;
          double pt2 = pT2(hcmMom);
          if (w > 50. && w <= 100.) {
            _histSpectraW77 ->fill(xf, weight); 
          } else if (w > 100. && w <= 150.) {
            _histSpectraW122->fill(xf, weight);
          } else if (w > 150. && w <= 200.) {
            _histSpectraW169->fill(xf, weight);
          }
          _histSpectraW117->fill(xf, weight);
          /// @todo Is this profile meant to be filled with 2 weight factors?
          _histPT2->fill(xf, pt2*weight/GeV2, weight);
          ++ncharged;
        }
      }
    

      // Energy-energy correlation
      if (th <= 8.) continue;
      double phi1 = p.momentum().azimuthalAngle(ZERO_2PI);
      double eta1 = p.momentum().pseudorapidity();
      double et1 = fabs(Et(p.momentum()));
      for (size_t ip2 = ip1+1; ip2 < particles.size(); ++ip2) {
        const Particle& p2 = particles[ip2];
        
        //double th2 = beamAngle(p2.momentum(), order);
        double th2 = p2.momentum().angle(dk.beamHadron().momentum()) / degree;
        if (th2 <= 8.) continue;
        double phi2 = p2.momentum().azimuthalAngle(ZERO_2PI);

        /// @todo Use angle function
        double deltaphi = phi1 - phi2;
        if (fabs(deltaphi) > PI) 
          deltaphi = fabs(fabs(deltaphi) - TWOPI);
        double eta2 = p2.momentum().pseudorapidity();
        double omega = sqrt(sqr(eta1-eta2) + sqr(deltaphi));
        double et2 = fabs(Et(p2.momentum()));
        double wt = et1*et2 / sqr(ptel) * weight;
        if(x < 1e-3) {
          _histEECLowX ->fill(omega, wt);
        } else {
          _histEECHighX->fill(omega,wt);
        }
      }
    }

    // Factors for normalization
    if (w > 50. && w <= 200.) {
      if (w <= 100.) {
        _w77.first  += ncharged*weight;
        _w77.second += weight;
      } else if (w <= 150.) {
        _w122.first  += ncharged*weight;
        _w122.second += weight;
      } else {
        _w169.first  += ncharged*weight;
        _w169.second += weight;
      }
      _w117.first  += ncharged*weight;
      _w117.second += weight;
    }
  }


  void H1_1994_S2919893::init() {
    _w77  = make_pair(0.,0.);
    _w122 = make_pair(0.,0.);
    _w169 = make_pair(0.,0.);
    _w117 = make_pair(0.,0.);
    _wEnergy = make_pair(0.,0.);
    _histEnergyFlowLowX =  bookHistogram1D(1, 1, 1, "Transverse energy flow as a function of rapidity, $x < 10^{-3}$");
    _histEnergyFlowHighX = bookHistogram1D(1, 1, 2, "Transverse energy flow as a function of rapidity $x > 10^{-3}$");
    _histEECLowX         = bookHistogram1D(2, 1, 1, "EEC for $x < 10^{-3}$");
    _histEECHighX        = bookHistogram1D(2, 1, 2, "EEC for $x > 10^{-3}$");
    _histSpectraW77      = bookHistogram1D(3, 1, 1, "Charged particle spectra for  $50 < W < 100$");
    _histSpectraW122     = bookHistogram1D(3, 1, 2, "Charged particle spectra for $100 < W < 150$");
    _histSpectraW169     = bookHistogram1D(3, 1, 3, "Charged particle spectra for $150 < W < 200$");
    _histSpectraW117     = bookHistogram1D(3, 1, 4, "Charged particle spectra for all $W$");
    _histPT2             = bookProfile1D  (4, 1, 1, "Average $p_\\perp^2$ as function of $x_F$");
  }


  // Finalize
  void H1_1994_S2919893::finalize() { 
    // Normalize inclusive single particle distributions to the average number 
    // of charged particles per event.
    double avgNumParts = _w77.first/_w77.second;
    normalize(_histSpectraW77, avgNumParts);

    avgNumParts = _w122.first/_w122.second;
    normalize(_histSpectraW122, avgNumParts);

    avgNumParts = _w169.first/_w169.second;
    normalize(_histSpectraW169, avgNumParts);

    avgNumParts = _w117.first/_w117.second;
    normalize(_histSpectraW117, avgNumParts);

    scale(_histEnergyFlowLowX , 1./_wEnergy.first );
    scale(_histEnergyFlowHighX, 1./_wEnergy.second);

    scale(_histEECLowX , 1./_wEnergy.first );
    scale(_histEECHighX, 1./_wEnergy.second); 
  }


}
