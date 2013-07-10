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

#include "meta_lns.h"
#include "hybrid_model.h"
#include <list>


namespace Gecode { namespace Search { namespace Meta {
  
  /// FIXME: to be removed
  LNSInstanceOptions* LNS::lns_options;
  
  Space*
  LNS::next(void) {
    while (true) {
      // We have to distinguish at least these two cases:
      // 1. we landed here for the first time (or because of a restart)
      // 2. we just discovered a new best solution in the previous next() call
      // the two cases will be distinguished by the invariant that current is NULL
      // only in case 1 and different from NULL in case 2
      if (current == NULL)
      { // we landed in this function for the first time or after a restart
        intensity = lns_options->minIntensity();
        temperature = lns_options->SAstartTemperature();
        idle_iterations = 0;
        neighbors_accepted = 0;
        current = root->clone(shared);
        LNSModel* _current = dynamic_cast<LNSModel*>(current);
        _current->initial_solution_branching(restart);
        // The initial solution is searched with a copy of the engine that has the same
        // stop object as the overall LNS
        se->reset(current);
        Space* n = se->next();
        delete current;
        if (n != NULL) {
          if (best == NULL) // it's the very first time the function is called
          {
            best = n->clone(shared);
            current = n->clone(shared);
            return n;
          }
          LNSModel* _n = dynamic_cast<LNSModel*>(n);
          if (_n->strictlyImproving(best))
          {
            delete best;
            best = n->clone(shared);
            current = n->clone(shared);
            return n;
          }
          else
            current = n;
        }
        else // no initial solution has been found
          return NULL;
      }
      else
      { // we landed in this function after a previous call to next or we are currently looping
        if (idle_iterations > lns_options->maxIterationsPerIntensity())
        {
          if (intensity < lns_options->maxIntensity())
            intensity++;
          idle_iterations = 0;
          std::cerr << "Current intensity " << intensity << std::endl;
        }
        if (neighbors_accepted > lns_options->SAneighborsAccepted())
        {
          temperature *= lns_options->SAcoolingRate();
          neighbors_accepted = 0;
        }
        Space* neighbor = root->clone(shared);
        LNSModel* _current = dynamic_cast<LNSModel*>(current);
        unsigned int relaxed_variables = _current->relax(neighbor, intensity);
        LNSModel* _neighbor = dynamic_cast<LNSModel*>(neighbor);
        _neighbor->neighborhood_branching();
        e_stop->limit(relaxed_variables * lns_options->timePerVariable());
        e_stop->reset();
        switch (lns_options->constrainType()) {
          case LNS_CT_LOOSE:
            _neighbor->constrain(current, false, 0.0);
            break;
          case LNS_CT_STRICT:
            _neighbor->constrain(current, true, 0.0);
            break;
          case LNS_CT_SA:
          {
            double p = r(RAND_MAX) / (double)RAND_MAX; // p should be a uniformly random number in (0, 1]
            double delta = -temperature * std::log(p);
            _neighbor->constrain(current, false, delta);
          }
            break;
          case LNS_CT_NONE:
          default:
            break;
        }
        e->reset(neighbor);
        Space* n = NULL;
        std::list<Space*> prev_solutions;
        do
          prev_solutions.push_back(e->next());
        while (prev_solutions.back() != NULL);
        if (prev_solutions.size() > 1)
          prev_solutions.pop_back(); // remove the last NULL solution
        n = prev_solutions.back();
        prev_solutions.pop_back();
        for (std::list<Space*>::iterator it = prev_solutions.begin(); it != prev_solutions.end(); it++)
          delete *it;
        if (n != NULL)
        {
          neighbors_accepted++;
          LNSModel* _n = dynamic_cast<LNSModel*>(n);
          if (_n->strictlyImproving(best))
          {
            delete best;
            best = n->clone(shared);
            delete current;
            current = n->clone(shared);
            idle_iterations = 0;
            intensity = lns_options->minIntensity();
            return n;
          }
          else if (_n->improving(current) || lns_options->constrainType() == LNS_CT_SA)
          {
            delete current;
            current = n;
          }
        }
        if (m_stop != NULL && m_stop->stop(statistics(), opt)) // the overall search has to be stopped
        {
          // eventually ask to restart
          delete current;
          current = NULL;
          restart++;
          return NULL;
        }
      }
      idle_iterations++;
    }
    GECODE_NEVER;
    return NULL;
  }
  
  Search::Statistics
  LNS::statistics(void) const {
    return stats + e->statistics();
  }
  
  bool
  LNS::stopped(void) const {
    /*
     * What might happen during parallel search is that the
     * engine has been stopped but the meta engine has not, so
     * the meta engine does not perform a restart. However the
     * invocation of next will do so and no restart will be
     * missed.
     */
    return e->stopped();
  }
  
  void
  LNS::reset(Space* s) {
    current = s->clone(shared);
    idle_iterations = 0;
    intensity = lns_options->minIntensity();
    neighbors_accepted = 0;
    temperature = lns_options->SAstartTemperature();
  }
  
  LNS::~LNS(void) {
    // Deleting e also deletes stop
    delete e;
    delete root;
  }
  
}}}

// STATISTICS: search-other
