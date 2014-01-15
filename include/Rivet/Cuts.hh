#ifndef RIVET_Cuts_HH
#define RIVET_Cuts_HH
#include <boost/smart_ptr.hpp>

namespace Rivet {

  /// @internal Forward declaration of helper class. Not for end users.
  class CuttableBase;

  /// @internal Base class for cut objects. 
  /// @note End users should always use the @ref Cut typedef instead.
  class CutBase;
  /// Main cut object
  typedef boost::shared_ptr<CutBase> Cut;

  class CutBase {
  public:
    /// Main work method. 
    /// @internal Forwards the received object to @ref accept_, 
    /// wrapped in the Cuttable converter
    template <typename ClassToCheck> bool accept(const ClassToCheck &) const;
    /// Comparison to another Cut
    virtual bool operator==(const Cut & c) const = 0;
    /// Default destructor
    virtual ~CutBase() {}
  protected:
    /// @internal Actual accept implementation, overloadable by various cut combiners
    virtual bool accept_(const CuttableBase &) const = 0;
  };

  /// Main cut object
  typedef boost::shared_ptr<CutBase> Cut;

  // compare two cuts for equality, forwards to the cut-specific implementation
  inline bool operator==(const Cut & a, const Cut & b) { return *a == b; }

  /// Namespace used for ambiguous identifiers.
  namespace Cuts {
    /// Available categories of cut objects
    enum Quantity { pT, mass, rap, eta, phi };
    /// Fully open cut singleton, accepts everything
    const Cut & open();
  }

  /// @name Cut constructors
  //@{
  Cut operator < (Cuts::Quantity, double);
  Cut operator > (Cuts::Quantity, double);
  Cut operator <= (Cuts::Quantity, double);
  Cut operator >= (Cuts::Quantity, double);

  /// @name Shortcuts for common cuts
  //@{
  Cut Range(Cuts::Quantity, double m, double n);
  inline Cut EtaIn(double m, double n) { return Range(Cuts::eta,m,n); }
  /// @todo Can do more here: AbsEtaLess, PtGtr
  //@}

  /// @internal Overload helpers for integer arguments
  //@{
  inline Cut operator <  (Cuts::Quantity qty, int i) { return qty <  double(i); }
  inline Cut operator >  (Cuts::Quantity qty, int i) { return qty >  double(i); }
  inline Cut operator <= (Cuts::Quantity qty, int i) { return qty <= double(i); }
  inline Cut operator >= (Cuts::Quantity qty, int i) { return qty >= double(i); }
  //@}

  //@}

  /// @name Cut combiners
  //@{
  /// Logical AND operation on two cuts
  Cut operator & (const Cut aptr, const Cut bptr);
  /// Logical OR operation on two cuts
  Cut operator | (const Cut aptr, const Cut bptr);
  /// Logical NOT operation on a cut
  Cut operator ~ (const Cut cptr);
  /// Logical XOR operation on two cuts
  Cut operator ^ (const Cut aptr, const Cut bptr);
  //@}
}

#endif
