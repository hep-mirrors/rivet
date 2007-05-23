// -*- C++ -*-

#include "Rivet/Rivet.hh"
#include "Rivet/RivetAIDA.hh"
#include "Rivet/AnalysisHandler.hh"
#include "Rivet/Analysis/Analysis.hh"
#include "Rivet/Analysis/TestAnalysis.hh"
#include "Rivet/Analysis/ExampleTree.hh"
#include "Rivet/Analysis/PL273B181.hh"
#include "Rivet/Analysis/HepEx9506012.hh"
#include "Rivet/Analysis/HepEx0112029.hh"
#include "Rivet/Analysis/PRD65092002.hh"
#include "Rivet/Analysis/HepEx0409040.hh"
#include "Rivet/Tools/Logging.hh"

using namespace AIDA;


//////////////////////////////////////////////////////////////


namespace Rivet {

  /// If you write a new analysis, add the ID for it here.
  Analysis& Analysis::getAnalysis(const AnalysisName atype) {
    switch (atype) {
    case ANALYSIS_TEST:
      return *(new TestAnalysis());
    case ANALYSIS_EXAMPLETREE:
      return *(new ExampleTree());
    case ANALYSIS_HEPEX9506012:
      return *(new HepEx9506012());
    case ANALYSIS_HEPEX0112029:
      return *(new HepEx0112029());
    case ANALYSIS_HEPEX0409040:
      return *(new HepEx0409040());  
    case ANALYSIS_PL273B181:
      return *(new PL273B181());
    case ANALYSIS_PRD65092002:
      return *(new PRD65092002());
    }
    throw runtime_error("Tried to get an analysis not known in the Rivet::AnalysisName enum.");
  }


  //////////////////////////////////////////////////////////////


  IAnalysisFactory& Analysis::analysisFactory() {
    return getHandler().analysisFactory();
  }


  ITree& Analysis::tree() {
    return getHandler().tree();
  }


  IHistogramFactory& Analysis::histogramFactory() {
    return getHandler().histogramFactory();
  }


  Log& Analysis::getLog() {
    string logname = "Rivet.Analysis." + getName();
    return Log::getLog(logname);
  }


  IHistogram1D* Analysis::bookHistogram1D(const unsigned int datasetId, const unsigned int xAxisId, 
                                          const unsigned int yAxisId, const string& title) {
    stringstream axisCode;
    axisCode << "ds" << datasetId << "-x" << xAxisId << "-y" << yAxisId;
    const map<string, BinEdges> data = getBinEdges(getName());
    makeHistoDir();
    const string path = getHistoDir() + "/" + axisCode.str();
    return histogramFactory().createHistogram1D(path, title, data.find(axisCode.str())->second);
  }


  IHistogram1D* Analysis::bookHistogram1D(const string& name, const string& title, 
                                          const int nbins, const double lower, const double upper) {
    makeHistoDir();
    const string path = getHistoDir() + "/" + name;
    return histogramFactory().createHistogram1D(path, title, nbins, lower, upper);
  }


  IHistogram1D* Analysis::bookHistogram1D(const string& name, const string& title, 
                                          const vector<double>& binedges) {
    makeHistoDir();
    const string path = getHistoDir() + "/" + name;
    return histogramFactory().createHistogram1D(path, title, binedges);
  }


  void Analysis::makeHistoDir() {
    if (!_madeHistoDir) {
      if (! getName().empty()) {
        tree().mkdir(getHistoDir());
      }
      _madeHistoDir = true;
    }
  }


  const Cuts Analysis::getCuts() const {
    Cuts totalCuts = _cuts;
    for (set<Projection*>::const_iterator p = _projections.begin(); p != _projections.end(); ++p) {
      totalCuts.addCuts((*p)->getCuts());
    }
    return totalCuts;
  }
  

  const bool Analysis::checkConsistency() const {
    // Check consistency of analysis beams with allowed beams of each contained projection.
    for (set<Projection*>::const_iterator p = _projections.begin(); p != _projections.end(); ++p) {
      if (! compatible(getBeams(), (*p)->getBeamPairs()) ) {
        throw runtime_error("Analysis " + getName() + " beams are inconsistent with " 
                            + "allowed beams for projection " + (*p)->getName());
      }
    }
    // Check the consistency of the accumulated cuts (throws if wrong).
    getCuts().checkConsistency();
    return true;
  }


  set<Projection*> Analysis::getProjections() const {
    set<Projection*> totalProjections = _projections;
    for (set<Projection*>::const_iterator p = _projections.begin(); p != _projections.end(); ++p) {
      totalProjections.insert((*p)->getProjections().begin(), (*p)->getProjections().end());
    }
    return totalProjections;
  }


}
