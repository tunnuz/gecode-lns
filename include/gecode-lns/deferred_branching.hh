#ifndef DEFERRED_BRANCHING
#define DEFERRED_BRANCHING

#include <gecode/search.hh>

using namespace Gecode;

/**
 A class template representing a Space whose branching is posted separately from the model. 
 The template class (T) should redefine the basic branching styles.
 @remarks the cost variable is "cost".
 */
class DeferredBranchingSpace
{
public:
  
  /** Post a branching good for performing a typical tree search, e.g., B&B, DFS, ... */
  virtual void tree_search_branching() {  }
  
};

/**
 A class that takes a DeferredBranchingSpace as a template parameter and just posts the branching straight away
 */
template <class T, class O>
class InstantBranchingSpace : public T
{
public:
  
  // Gecode default constructor
  InstantBranchingSpace<T, O>(const O& o) : T(o)
  {
    this->tree_search_branching();
  }

  /** Gecode copy constructor (branching already posted) */
  InstantBranchingSpace<T, O>(bool share, InstantBranchingSpace<T, O>& t) : T(share, t)
  { }
  
  /** Gecode copy method */
  virtual Space* copy(bool share)
  {
    return new InstantBranchingSpace<T, O>(share, *this);
  }
};

#endif
