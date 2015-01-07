/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Luca Di Gaspero <luca.digaspero@uniud.it>
 *     Tommaso Urli <tommaso.urli@uniud.it>
 *
 *  Copyright:
 *     Luca Di Gaspero, Tommaso Urli, 2013
 *
 *
 */



#ifndef __GECODE_SEARCH_META_LNS_HH__
#define __GECODE_SEARCH_META_LNS_HH__

#include <gecode/search.hh>

#include "gecode-lns/lns.hh"

namespace Gecode { namespace Search { namespace Meta {

  /// Engine for restart-based search
  class LNS : public Engine {
  private:
    /// The actual engine(s)
    Engine* se;
    Engine* e;
    /// The root space to create new partial solutions from scratch
    Space* root;
    /// The best solution that far
    Space* best;
    /// The current solution
    Space* current;
    /// The stop control object for the sub-engine
    TimeStop* e_stop;
    /// The stop control object for the overall LNS
    Stop* m_stop;
    /// The statistics
    Search::Statistics& stats;
    /// The options
    const Options& opt;
    /// The number of times stop has reached
    unsigned long int restart;
    /// The number of idle iterations performed (for detecting stagnation)
    unsigned long int idle_iterations;
    /// The current intensity for LNS
    unsigned int intensity;
    /// Whether the slave can be shared with the master
    bool shared;
    /// Random numbers generator
    Rnd r;
    /// Current temperature for SA
    double temperature;
    /// Neighbors accepted at current temperature
    unsigned long int neighbors_accepted;

    /// Empty no-goods (copied from RBS)
    GECODE_SEARCH_EXPORT
    static NoGoods eng;

  public:
    /// Constructor
    LNS(Space*, size_t, TimeStop* e_stop0,
        Engine* se0, Engine* e0, Search::Statistics& stats0, const Options& opt0);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Reset engine to restart at space \a s
    virtual void reset(Space* s);
    /// Destructor
    virtual ~LNS(void);
    /// FIXME: waiting for a definitive way to pass specific options to the (meta-)engines, currently they will be embedded in
    /// a static member of the LNS class
    static LNSBaseOptions* lns_options;
    /// Return no-goods
    virtual NoGoods& nogoods(void);
  };

  forceinline
  LNS::LNS(Space* s, size_t, TimeStop* e_stop0,
           Engine* se0, Engine* e0, Search::Statistics& stats0, const Options& opt0)
    : se(se0), e(e0), root(s), best(0), current(0), e_stop(e_stop0), m_stop(opt0.stop), stats(stats0), opt(opt0), restart(0), idle_iterations(0),
  shared(opt.threads == 1), temperature(1.0) {

    r.time();
  }

}}}

#endif

// STATISTICS: search-other
