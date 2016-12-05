#ifndef _LNS_SPACE_H
#define _LNS_SPACE_H

#include <gecode/kernel.hh>
#include <gecode/driver.hh>

using namespace Gecode;

class LNSAbstractSpace
{
public:

  /** Post a random branching, e.g. good for finding a random initial solution in LNS */
  virtual void initial_solution_branching(unsigned long int restart) = 0;

  /** Post a branching for LNS iteration step, the idea is that it should likely find a good solution  */
  virtual void neighborhood_branching() = 0;

  /** Method to generate a relaxed solution (i.e., a neighbor) from the current one (this) */
  virtual unsigned int relax(Space* neighbor, unsigned int free) = 0;

  /* Returns whether the current space is improving w.r.t. s */
  virtual bool improving(const Space& s, bool strict = true) = 0;

  /* Constrain current solution cost to improve over the one passed as parameter plus/minus a delta */
  virtual void constrain(const Space& s, bool strict, double delta) = 0;
};

template <class ScriptType>
class LNSScript : public LNSAbstractSpace, public ScriptType
{
public:
    virtual bool improving(const Space& s, bool strict = true)
  {
    const LNSScript& _s = dynamic_cast<const LNSScript&>(s);
    if (strict)
      return this->cost().val() < _s.cost().val();
    else
      return this->cost().val() <= _s.cost().val();
  }

  virtual void constrain(const Space& s, bool strict, double delta)
  {
    const LNSScript& _s = dynamic_cast<const LNSScript&>(s);
    if (strict)
      rel(*this, this->cost() < _s.cost().val() + delta);
    else
      rel(*this, this->cost() <= _s.cost().val() + delta);
  }

protected:
  LNSScript() : ScriptType(nullptr) {}
  template<class O>
  LNSScript(const O& opt) : ScriptType(opt) {}
  LNSScript(bool share, LNSScript& s) : ScriptType(share,s) {}
};

#endif
