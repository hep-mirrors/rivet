// -*- C++ -*-
#include "Rivet/Projections/JetAlg.hh"

namespace Rivet {


  JetAlg::JetAlg(const FinalState& fs, Muons usemuons, Invisibles useinvis)
    : _useMuons(usemuons), _useInvisibles(useinvis)
  {
    setName("JetAlg");
    declare(fs, "FS");
    VisibleFinalState vfs(fs);
    // MSG_DEBUG("Making visible final state from provided FS");
    declare(vfs, "VFS");
  }


}
