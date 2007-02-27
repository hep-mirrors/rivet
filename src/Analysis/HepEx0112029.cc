// -*- C++ -*-

#include "Rivet/Tools/Logging.hh"
#include "Rivet/Analysis/HepEx0112029.hh"
using namespace Rivet;

#include "AIDA/IHistogram1D.h"
using namespace AIDA;

#include "HepPDT/ParticleID.hh"
using namespace HepMC;


/////////////////////////////////////////////////


// Constructore
HepEx0112029::HepEx0112029() :
  fs(),
  ktjets(fs) 
{ }




HepEx0112029::~HepEx0112029() {}


// Book histograms
void HepEx0112029::init() {
  histJetEt1_         = bookHistogram1D("JetET1", "Jet transverse energy", 11, 14., 75.);
}


// Do the analysis
void HepEx0112029::analyze(const Event & event) {
  Logger& log = getLogger(ANALYSIS_HEPEX0112029);
  log << LogPriority::DEBUG << "Starting analyzing" << endlog;

  // Analyse and print some info
  const KtJets& jets = event.applyProjection(ktjets);

  int nj = jets.getNJets();

  log << LogPriority::INFO << "Jet multiplicity            = " << nj << endlog;

  std::vector<KtJet::KtLorentzVector> jetList = jets.getJetsEt();

  // Fill histograms
  for (std::vector<KtJet::KtLorentzVector>::iterator j = jetList.begin(); j != jetList.end(); j++) {

    histJetEt1_->fill(j->perp(), 1.0);

  }
  
  // Finished...
  log << LogPriority::DEBUG << "Finished analyzing" << endlog;
}


// Finalize
void HepEx0112029::finalize() { }


// Provide info object
RivetInfo HepEx0112029::getInfo() const {
  return Analysis::getInfo() + ktjets.getInfo();
}
