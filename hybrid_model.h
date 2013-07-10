#ifndef _HYBRID_MODEL_H
#define _HYBRID_MODEL_H

#include <gecode/kernel.hh>
#include <gecode/driver.hh>

using namespace Gecode;

/** An interface defining all the necessary methods that a LNS-ready model must implement */
class LNSModel
{
public:
    
  /** Post the full branching instructions for classical CP tree search (i.e., complete the model with branching) */
  virtual void tree_search_branching() = 0;
  
  /** Post a random branching, e.g. good for finding a random initial solution in LNS 
      @param restart indicates the number of this restart, can be useful to customize branching
   */
  virtual void initial_solution_branching(unsigned long int restart) = 0;
  
  /** Post a branching for LNS iteration step, the idea is that it should likely find a good solution  */
  virtual void neighborhood_branching() = 0;
  
  /** Method to generate a relaxed solution (i.e., tentative) from the current one (this) 
      @param tentative (typically) unassigned solution that must be assigned with values from the current one
      @param free indicative intensity of relaxation, i.e., amount of solution to leave free, e.g., variables
      @return the real number of freed variables
   */
  virtual unsigned int relax(Space* tentative, unsigned int free) = 0;
  
  /** Returns the maximum number of relaxable variables */
  virtual unsigned int relaxable_vars() = 0;
  
  /** Checks whether the current space is improving w.r.t. s
      @param s space to compare with
      @return true if the current space is improving w.r.t. s 
   */
  virtual bool improving(Space* s) = 0;
  
  /** Checks whether the current space is strictly improving w.r.t. s
   @param s space to compare with
   @return true if the current space is strictly improving w.r.t. s
   */  virtual bool strictlyImproving(Space* s) = 0;
  
  /* Constrain current solution cost to improve over the one passed as parameter plus/minus a delta */
  virtual void constrain(Space* s, bool strict, double delta) = 0;
};

/* A template LNS-ready model which implements some of the function, i.e., improving, strictlyImproving and constrain, using information from the original space class. */
template <class BaseModel, class MySpace>
class HybridModel : public MySpace, public BaseModel, public LNSModel
{
public:
  
  HybridModel(const InstanceOptions& opt) : BaseModel(this, opt)
  {}

  HybridModel(bool share, HybridModel<BaseModel, MySpace>& s) : MySpace(share, s), BaseModel(share, s, this)
  {}

  virtual IntVar cost() const
  { return BaseModel::cost(); }
  
  virtual bool improving(Space* s)
  {
    // FIXME: maybe we can consider using compare(), which however should be defined in Script based classes

    MinimizeSpace* minimize = dynamic_cast<MinimizeSpace*>(s);
    if (minimize != NULL) // this was a minimize space
      return this->cost().val() <= minimize->cost().val();
    else
    {
      MaximizeSpace* maximize = dynamic_cast<MaximizeSpace*>(s);
      if (maximize != NULL) // this was a maximize space
        return this->cost().val() >= maximize->cost().val();
    }
    return false; // TODO: consider whether to throw an exception
  }
  
  virtual bool strictlyImproving(Space* s)
  {
    MinimizeSpace* minimize = dynamic_cast<MinimizeSpace*>(s);
    if (minimize != NULL) // this was a minimize space
      return this->cost().val() < minimize->cost().val();
    else
    {
      MaximizeSpace* maximize = dynamic_cast<MaximizeSpace*>(s);
      if (maximize != NULL) // this was a maximize space
        return this->cost().val() > maximize->cost().val();
    }
    return false; // TODO: consider whether to throw an exception
  }
  
  virtual void constrain(Space* s, bool strict, double delta)
  {
    MinimizeSpace* minimize = dynamic_cast<MinimizeSpace*>(s);
    if (minimize != NULL) // this was a minimize space
    {
      if (strict)
        rel(*this, this->cost() < minimize->cost().val() + delta);
      else
        rel(*this, this->cost() <= minimize->cost().val() + delta);
    }
    else
    {
      MaximizeSpace* maximize = dynamic_cast<MaximizeSpace*>(s);
      if (maximize != NULL) // this was a maximize space
      {
        if (strict)
          rel(*this, this->cost() > maximize->cost().val() - delta);
        else
          rel(*this, this->cost() >= maximize->cost().val() - delta);
      }
    }
    // TODO: consider whether to throw an exception
  }

  virtual void print(std::ostream& os = std::cout) const
  { BaseModel::print(os); }
  
};

#endif

