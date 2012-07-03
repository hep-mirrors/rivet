// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/RivetYODA.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/FinalState.hh"

namespace Rivet {


  /// @brief Jet rates in \f$ e^+ e^- \f$ at OPAL and JADE
  /// @author Frank Siegert
  class JADE_OPAL_2000_S4300807 : public Analysis {
  public:

    /// @name Constructors etc.
    //@{

    /// Constructor
    JADE_OPAL_2000_S4300807() : Analysis("JADE_OPAL_2000_S4300807") {
    }

    //@}


    /// @name Analysis methods
    //@{

    void init() {
      // Projections
      const FinalState fs;
      addProjection(fs, "FS");
      FastJets JadeJets = FastJets(fs, FastJets::JADE, 0.7);
      FastJets DurhamJets = FastJets(fs, FastJets::DURHAM, 0.7);
      JadeJets.useInvisibles(true);
      DurhamJets.useInvisibles(true);
      addProjection(JadeJets, "JadeJets");
      addProjection(DurhamJets, "DurhamJets");

      // Histos
      int offset = 0;
      switch (int(sqrtS()/GeV + 0.5)) {
      case 35: offset = 7; break;
      case 44: offset = 8; break;
      case 91: offset = 9; break;
      case 133: offset = 10; break;
      case 161: offset = 11; break;
      case 172: offset = 12; break;
      case 183: offset = 13; break;
      case 189: offset = 14; break;
      default: break;
      }
      for (size_t i = 0; i < 5; ++i) {
        _h_R_Jade[i] = bookHisto1D(offset, 1, i+1);
        _h_R_Durham[i] = bookHisto1D(offset+9, 1, i+1);
        if (i < 4) _h_y_Durham[i] = bookHisto1D(offset+17, 1, i+1);
      }
    }



    void analyze(const Event& e) {
      const double weight = e.weight();
      MSG_DEBUG("Num particles = " << applyProjection<FinalState>(e, "FS").particles().size());

      const FastJets& jadejet = applyProjection<FastJets>(e, "JadeJets");
      if (jadejet.clusterSeq()) {
        /// @todo Put this in an index loop?
        double y_23 = jadejet.clusterSeq()->exclusive_ymerge_max(2);
        double y_34 = jadejet.clusterSeq()->exclusive_ymerge_max(3);
        double y_45 = jadejet.clusterSeq()->exclusive_ymerge_max(4);
        double y_56 = jadejet.clusterSeq()->exclusive_ymerge_max(5);

        for (size_t i = 0; i < _h_R_Jade[0]->numBins(); ++i) {
          double ycut = _h_R_Jade[0]->bin(i).midpoint();
          double width = _h_R_Jade[0]->bin(i).width();
          if (y_23 < ycut) {
            _h_R_Jade[0]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Jade[1]->numBins(); ++i) {
          double ycut = _h_R_Jade[1]->bin(i).midpoint();
          double width = _h_R_Jade[1]->bin(i).width();
          if (y_34 < ycut && y_23 > ycut) {
            _h_R_Jade[1]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Jade[2]->numBins(); ++i) {
          double ycut = _h_R_Jade[2]->bin(i).midpoint();
          double width = _h_R_Jade[2]->bin(i).width();
          if (y_45 < ycut && y_34 > ycut) {
            _h_R_Jade[2]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Jade[3]->numBins(); ++i) {
          double ycut = _h_R_Jade[3]->bin(i).midpoint();
          double width = _h_R_Jade[3]->bin(i).width();
          if (y_56 < ycut && y_45 > ycut) {
            _h_R_Jade[3]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Jade[4]->numBins(); ++i) {
          double ycut = _h_R_Jade[4]->bin(i).midpoint();
          double width = _h_R_Jade[4]->bin(i).width();
          if (y_56 > ycut) {
            _h_R_Jade[4]->fill(ycut, weight*width);
          }
        }
      }

      const FastJets& durjet = applyProjection<FastJets>(e, "DurhamJets");
      if (durjet.clusterSeq()) {
        /// @todo Put this in an index loop?
        double y_23 = durjet.clusterSeq()->exclusive_ymerge_max(2);
        double y_34 = durjet.clusterSeq()->exclusive_ymerge_max(3);
        double y_45 = durjet.clusterSeq()->exclusive_ymerge_max(4);
        double y_56 = durjet.clusterSeq()->exclusive_ymerge_max(5);

        _h_y_Durham[0]->fill(y_23, weight);
        _h_y_Durham[1]->fill(y_34, weight);
        _h_y_Durham[2]->fill(y_45, weight);
        _h_y_Durham[3]->fill(y_56, weight);

        for (size_t i = 0; i < _h_R_Durham[0]->numBins(); ++i) {
          double ycut = _h_R_Durham[0]->bin(i).midpoint();
          double width = _h_R_Durham[0]->bin(i).width();
          if (y_23 < ycut) {
            _h_R_Durham[0]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Durham[1]->numBins(); ++i) {
          double ycut = _h_R_Durham[1]->bin(i).midpoint();
          double width = _h_R_Durham[1]->bin(i).width();
          if (y_34 < ycut && y_23 > ycut) {
            _h_R_Durham[1]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Durham[2]->numBins(); ++i) {
          double ycut = _h_R_Durham[2]->bin(i).midpoint();
          double width = _h_R_Durham[2]->bin(i).width();
          if (y_45 < ycut && y_34 > ycut) {
            _h_R_Durham[2]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Durham[3]->numBins(); ++i) {
          double ycut = _h_R_Durham[3]->bin(i).midpoint();
          double width = _h_R_Durham[3]->bin(i).width();
          if (y_56 < ycut && y_45 > ycut) {
            _h_R_Durham[3]->fill(ycut, weight*width);
          }
        }
        for (size_t i = 0; i < _h_R_Durham[4]->numBins(); ++i) {
          double ycut = _h_R_Durham[4]->bin(i).midpoint();
          double width = _h_R_Durham[4]->bin(i).width();
          if (y_56 > ycut) {
            _h_R_Durham[4]->fill(ycut, weight*width);
          }
        }
      }
    }



    /// Finalize
    void finalize() {
      for (size_t n = 0; n < 4; ++n) {
        scale(_h_y_Durham[n], 1.0/sumOfWeights());
      }

      for (size_t n = 0; n < 5; ++n) {
        // Scale integrated jet rates to 100%
        scale(_h_R_Jade[n],   100./sumOfWeights());
        scale(_h_R_Durham[n], 100./sumOfWeights());
      }
    }

    //@}


  private:

    /// @name Histograms
    //@{
    Histo1DPtr _h_R_Jade[5];
    Histo1DPtr _h_R_Durham[5];
    Histo1DPtr _h_y_Durham[4];
    //@}

  };



  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(JADE_OPAL_2000_S4300807);

}
