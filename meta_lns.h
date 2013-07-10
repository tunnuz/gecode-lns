#ifndef _META_LNS_HH
#define _META_LNS_HH

#include <gecode/search.hh>
#include "lns.h"

namespace Gecode { namespace Search { namespace Meta {

  /** Engine for LNS-based search */
  class LNS : public Engine {
  private:

    /** The underlying engine for finding the initial solution */
    Engine* se;
    
    /** The underlying engine for the LNS step, e.g., BAB, DFS, etc. */
    Engine* e;
    
    /** The root space to create new partial solutions from scratch */
    Space* root;
    
    /** The best solution so far */
    Space* best;
    
    /** The current solution */
    Space* current;
    
    /** The stop object for the sub-engine */
    TimeStop* e_stop;
    
    /** The stop object for the overall LNS engine */
    Stop* m_stop;
    
    /** Statistics */
    Search::Statistics& stats;
    
    /** The search options */
    const Options& opt;
    
    /** The number of times the search has been restarted */
    unsigned long int restart;
    
    /** The number of non-improving iterations performed (for detecting stagnation) */
    unsigned long int idle_iterations;
    
    /** The current intensity for LNS relaxtion */
    unsigned int intensity;
    
    /** Whether the slave can be shared with the master */
    bool shared;
    
    /** PRNG */
    Rnd r;
    
    /** Current temperature for SA stopping criterion */
    double temperature;
    
    /** Neighbors accepted at current temperature (to trigger cooling step) */
    unsigned long int neighbors_accepted;
    
  public:
    
    /** Constructor */
    LNS(Space*, size_t, TimeStop* e_stop0, Engine* se0, Engine* e0, Search::Statistics& stats0, const Options& opt0);
    
    /** Return next solution (NULL, if none exists or search has been stopped) */
    virtual Space* next(void);
    
    /** Return statistics */
    virtual Search::Statistics statistics(void) const;
    
    /** Check whether engine has been stopped */
    virtual bool stopped(void) const;
    
    /** Reset engine to restart at space 
        @param s the space to restart with
     */
    virtual void reset(Space* s);
    
    virtual ~LNS(void);
    
    /** FIXME: waiting for a definitive way to pass specific options to the (meta-)engines, currently they will be embedded in a static member of the LNS class */
    static LNSInstanceOptions* lns_options;
  };

  forceinline
  LNS::LNS(Space* s, size_t, TimeStop* e_stop0, 
           Engine* se0, Engine* e0, Search::Statistics& stats0, const Options& opt0)
    : se(se0), e(e0), root(s), best(0), current(0), e_stop(e_stop0), m_stop(opt0.stop), stats(stats0), opt(opt0), restart(0), idle_iterations(0),
  shared(opt.threads == 1), temperature(1.0) {}

}}}

#endif