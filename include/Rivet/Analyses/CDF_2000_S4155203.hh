// -*- C++ -*-
#ifndef RIVET_CDF_2000_S4155203_HH
#define RIVET_CDF_2000_S4155203_HH

#include "Rivet/Rivet.hh"
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Projections/ChargedLeptons.hh"

namespace Rivet {


  /// @brief CDF Run I Z pT in Drell-Yan events
  /// @author Hendrik Hoeth
  class CDF_2000_S4155203 : public Analysis {
  public:

    /// @name Constructors etc.
    //@{

    /// Constructor: cuts on final state are \f$ -1 < \eta < 1 \f$ 
    /// and \f$ p_T > 0.5 \f$ GeV.
    CDF_2000_S4155203()
    { 
      setBeams(PROTON, ANTIPROTON);
      const ChargedFinalState clfs(-4.2, 4.2, 15*GeV);
      addProjection(ChargedLeptons(clfs), "CL");
    }


    /// Factory method
    static Analysis* create() {
      return new CDF_2000_S4155203();
    }
    //@}


  public:

    string getSpiresId() const {
      return "4155203";
    }
    string getDescription() const {
      return "CDF Run 1 Z pT measurement in Z->e+e- events";
    }
    string getExpt() const {
      return "CDF";
    }
    string getYear() const {
      return "2000";
    }



  public:

    /// @name Analysis methods
    //@{
    void init();
    void analyze(const Event& event);
    void finalize();
    //@}

  private:

    AIDA::IHistogram1D *_hist_zpt;

  private:

    /// Hide the assignment operator.
    CDF_2000_S4155203& operator=(const CDF_2000_S4155203&);

  };

}

#endif
