// -*- C++ -*-
#include "Rivet/Rivet.hh"
#include "Rivet/Tools/Logging.hh"
#include "Rivet/ProjectionHandler.hh"
#include "Rivet/Cmp.hh"
#include <algorithm>

namespace Rivet {


  // Initialize instance pointer to null.
  ProjectionHandler* ProjectionHandler::_instance = 0;


  //int Log_TRACE = Log::INFO;


  ProjectionHandler* ProjectionHandler::create() {
    if (!_instance) {
      _instance = new ProjectionHandler();
      Log::getLog("Rivet.ProjectionHandler")
        << Log::TRACE << "Created new ProjectionHandler at " << _instance << endl;
    }
    return _instance;
  }


  void ProjectionHandler::destroy() {
    delete _instance;
  }


  // Get a logger.
  Log& ProjectionHandler::getLog() const {
    return Log::getLog("Rivet.ProjectionHandler");
  }



  void ProjectionHandler::clear() {
    _projs.clear();
    _namedprojs.clear();
  }



  // Delete contained pointers.
  ProjectionHandler::~ProjectionHandler() {
    clear();
  }



  // Take a Projection, compare it to the others on record, and return (by
  // reference) an equivalent Projection which is guaranteed to be the
  // (persistent) version that will be applied to an event.
  const Projection& ProjectionHandler::registerProjection(const ProjectionApplier& parent,
                                                          const Projection& proj,
                                                          const string& name)
  {
    getLog() << Log::TRACE << "Trying to register"
             << " projection " << &proj  << " (" << proj.name() << ")"
             << " for parent " << &parent << " (" << parent.name() << ")"
             << " with name '" << name << "'" << endl;

    // Check for duplicate use of "name" on "parent"
    const bool dupOk = _checkDuplicate(parent, proj, name);
    if (!dupOk) {
      cerr << "Duplicate name '" << name << "' in parent '" << parent.name() << "'." << endl;
      exit(1);
    }

    // Choose which version of the projection to register with this parent and name
    const Projection* p = _getEquiv(proj);
    if (p == 0) { // a null pointer is a non-match
      // If there is no equivalent projection, clone proj and use the clone for registering
      p = _clone(proj);
    }

    // Do the registering
    p = _register(parent, *p, name);

    // Return registered proj
    return *p;
  }




  // Attach and retrieve a projection as a pointer.
  const Projection* ProjectionHandler::registerProjection(const ProjectionApplier& parent,
                                                          const Projection* proj,
                                                          const string& name) {
    if (proj == 0) return 0;
    const Projection& p = registerProjection(parent, *proj, name);
    return &p;
  }



  // Clone neatly
  const Projection* ProjectionHandler::_clone(const Projection& proj)
  {
    // Clone a new copy of the passed projection on the heap
    getLog() << Log::TRACE << "Cloning projection " << proj.name() << " from " << &proj << endl;
    const Projection* newproj = proj.clone();
    getLog() << Log::TRACE << "Cloned projection " << proj.name() << " at " << newproj << endl;
    getLog() << Log::TRACE << "Clone types " << typeid(proj).name() << " -> " << typeid(newproj).name() << endl;

    // Copy all the child ProjHandles when cloning, since otherwise links to "stack parents"
    // will be generated by their children, without any connection to the cloned parent
    if (&proj != newproj) {
      NamedProjsMap::const_iterator nps = _namedprojs.find(&proj);
      if (nps != _namedprojs.end()) {
        getLog() << Log::TRACE << "Cloning registered projections list: "
                 << &proj << " -> " << newproj << endl;
        _namedprojs[newproj] = nps->second;
      }
    }

    return newproj;
  }



  // Take a Projection, compare it to the others on record, and
  // return (by reference) an equivalent Projection which is guaranteed to be
  // the version that will be applied to an event.
  const Projection* ProjectionHandler::_register(const ProjectionApplier& parent,
                                                 const Projection& proj,
                                                 const string& name)
  {
    ProjHandle ph(&proj);
    getLog() << Log::TRACE << "Registering new projection at " << ph << endl;

    // Add the passed Projection to _projs
    _projs.insert(ph);

    // Add the ProjApplier* => name location to the associative container
    _namedprojs[&parent][name] = ph;

    return ph.get();
  }




  // Try to find a equivalent projection in the system
  const Projection* ProjectionHandler::_getEquiv(const Projection& proj) const
  {
    // Get class type using RTTI
    const std::type_info& newtype = typeid(proj);
    getLog() << Log::TRACE << "RTTI type of " << &proj << " is " << newtype.name() << endl;

    // Compare to ALL projections via _projs collection
    getLog() << Log::TRACE << "Comparing " << &proj
             << " with " << _projs.size()
             << " registered projection" << (_projs.size() == 1 ? "" : "s") <<  endl;
    foreach (const ProjHandle& ph, _projs) {
      // Make sure the concrete types match, using RTTI.
      const std::type_info& regtype = typeid(*ph);
      getLog() << Log::TRACE << "RTTI type comparison with " << ph << ": "
               << newtype.name() << " vs. " << regtype.name() << endl;
      if (newtype != regtype) continue;
      getLog() << Log::TRACE << "RTTI type matches with " << ph << endl;

      // Test for semantic match
      if (pcmp(*ph, proj) != EQUIVALENT) {
        getLog() << Log::TRACE << "Projections at "
                 << &proj << " and " << ph << " are not equivalent" << endl;
      } else {
        getLog() << Log::TRACE << "MATCH! Projections at "
                 << &proj << " and " << ph << " are equivalent" << endl;
        return ph.get();
      }
    }

    // If no match, just return a null pointer
    return 0;
  }



  string ProjectionHandler::_getStatus() const {
    ostringstream msg;
    msg << "Current projection hierarchy:" << endl;
    foreach (const NamedProjsMap::value_type& nps, _namedprojs) {
      //const string parentname = nps.first->name();
      msg << nps.first << endl; //"(" << parentname << ")" << endl;
      foreach (const NamedProjs::value_type& np, nps.second) {
        msg << "  " << np.second << " (" << np.second->name()
            << ", locally called '" << np.first << "')" << endl;
      }
      msg << endl;
    }
    return msg.str();
  }



  // Check that the same parent hasn't already used this name for something else
  bool ProjectionHandler::_checkDuplicate(const ProjectionApplier& parent,
                                          const Projection& proj,
                                          const string& name) const
  {
    NamedProjsMap::const_iterator ipnps = _namedprojs.find(&parent);
    if (ipnps != _namedprojs.end()) {
      const NamedProjs pnps = ipnps->second;
      const NamedProjs::const_iterator ipph = pnps.find(name);
      if (ipph != pnps.end()) {
        const ProjHandle pph = ipph->second;
        getLog() << Log::ERROR << "Projection clash! "
                 << parent.name() << " (" << &parent << ") "
                 << "is trying to overwrite its registered '" << name << "' "
                 << "projection (" << pph << "="
                 << pph->name() << ") with a non-equivalent projection "
                 << "(" << &proj << "=" << proj.name() << ")" << endl;
        getLog() << Log::ERROR << _getStatus();
        return false;
      }
    }
    return true;
  }




  void ProjectionHandler::removeProjectionApplier(ProjectionApplier& parent) {
    NamedProjsMap::iterator npi = _namedprojs.find(&parent);
    if (npi != _namedprojs.end()) _namedprojs.erase(npi);
    //
    const Projection* parentprojptr = dynamic_cast<Projection*>(&parent);
    if (parentprojptr) {
      ProjHandle parentph(parentprojptr);
      ProjHandles::iterator pi = find(_projs.begin(), _projs.end(), parentph);
      if (pi != _projs.end()) _projs.erase(pi);
    }
  }




  set<const Projection*> ProjectionHandler::getChildProjections(const ProjectionApplier& parent,
                                                                ProjDepth depth) const
  {
    set<const Projection*> toplevel;
    NamedProjs nps = _namedprojs.find(&parent)->second;
    foreach (NamedProjs::value_type& np, nps) {
      toplevel.insert(np.second.get());
    }
    if (depth == SHALLOW) {
      // Only return the projections directly contained within the top level
      return toplevel;
    } else {
      // Return recursively built projection list
      set<const Projection*> alllevels = toplevel;
      foreach (const Projection* p, toplevel) {
        set<const Projection*> allsublevels = getChildProjections(*p, DEEP);
        alllevels.insert(allsublevels.begin(), allsublevels.end());
      }
      return alllevels;
    }
  }




  const Projection& ProjectionHandler::getProjection(const ProjectionApplier& parent,
                                                     const string& name) const {
    //getLog() << Log::TRACE << "Searching for child projection '"
    //         << name << "' of " << &parent << endl;
    NamedProjsMap::const_iterator nps = _namedprojs.find(&parent);
    if (nps == _namedprojs.end()) {
      ostringstream msg;
      msg << "No projections registered for parent " << &parent;
      throw Error(msg.str());
    }
    NamedProjs::const_iterator np = nps->second.find(name);
    if (np == nps->second.end()) {
      ostringstream msg;
      msg << "No projection '" << name << "' found for parent " << &parent;
      throw Error(msg.str());
    }
    // If it's registered with the projection handler, we must be able to safely
    // dereference the Projection pointer to a reference...
    return *(np->second);
  }



}
