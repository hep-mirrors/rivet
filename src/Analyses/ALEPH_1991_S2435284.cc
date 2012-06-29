// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Projections/Multiplicity.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/RivetYODA.hh"

namespace Rivet {


  /// @brief ALEPH LEP1 charged multiplicity in hadronic Z decay
  /// @author Andy Buckley
  class ALEPH_1991_S2435284 : public Analysis {
  public:

    /// Constructor.
    ALEPH_1991_S2435284()
      : Analysis("ALEPH_1991_S2435284")
    {
    }


    /// @name Analysis methods
    //@{

    /// Book projections and histogram
    void init() {
      const ChargedFinalState cfs;
      addProjection(cfs, "FS");
      addProjection(Multiplicity(cfs), "Mult");

      _histChTot = bookHisto1D(1, 1, 1);
    }


    /// Do the analysis
    void analyze(const Event& event) {
      const Multiplicity& m = applyProjection<Multiplicity>(event, "Mult");
      MSG_DEBUG("Total charged multiplicity = " << m.totalMultiplicity());
      _histChTot->fill(m.totalMultiplicity(), event.weight());
    }


    /// Normalize the histogram
    void finalize() {
      scale(_histChTot, 2.0/sumOfWeights()); // same as in ALEPH 1996
    }

    //@}


  private:

    /// @name Histograms
    //@{
    Histo1DPtr _histChTot;
    //@}

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(ALEPH_1991_S2435284);

}
