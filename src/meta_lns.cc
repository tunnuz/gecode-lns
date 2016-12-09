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

#include "gecode-lns/meta_lns.hh"
#include "gecode-lns/lns_space.hh"
#include <list>

using namespace std;

namespace Gecode { namespace Search { namespace Meta {

    /** Injected by main */
    LNSBaseOptions* LNS::lns_options;

    /** Nogoods are not really handled */
    NoGoods LNS::eng;
    NoGoods&
    LNS::nogoods(void) {
        return eng;
    }

    /** Search */
    Space* LNS::next(void) {

        while (true) {

            /** We have to distinguish at least these two cases:
             *
             *  1. we landed here for the first time (or because of a restart)
             *      --> current == NULL
             *  2. we just discovered a new best solution in the previous next() call
             *      --> current != NULL
             */

            // We landed in this function for the first time or after a restart
            if (current == NULL)
            {
                // Reset default search parameters (including Simulated Annealing ones)
                intensity = lns_options->minIntensity();
                temperature = lns_options->SAstartTemperature();
                idle_iterations = 0;
                neighbors_accepted = 0;
                current = root->clone(shared);
                LNSAbstractSpace* _current = dynamic_cast<LNSAbstractSpace*>(current);

                // In a restart, constraint cost if stated by the options
                if (best != NULL)
                {
                    switch (lns_options->constrainType()) {
                        case LNS_CT_LOOSE:
                            dynamic_cast<LNSAbstractSpace*>(current)->constrain(*best, false, 0.0);
                            break;
                        case LNS_CT_STRICT:
                            dynamic_cast<LNSAbstractSpace*>(current)->constrain(*best, true, 0.0);
                            break;
                        case LNS_CT_SA:
                        {
                            double p = (double) r(RAND_MAX) / (double)RAND_MAX; // p should be a uniformly random number in (0, 1]
                            double delta = -temperature * std::log(p);
                            dynamic_cast<LNSAbstractSpace*>(current)->constrain(*best, false, delta);
                        }
                            break;
                        case LNS_CT_NONE:
                        default:
                            break;
                    }
                }

                _current->initial_solution_branching(restart);

                // Look for (one) initial solution with same stopping condition as the overall LNS
                se->reset(current);
                Space* n = se->next();

                // If we find a starting solution
                if (n != NULL) {

                    // Best is this solution if it wasn't there
                    if (best == NULL)
                    {
                        best = n->clone(shared);
                        current = n->clone(shared);
                        return n;
                    }

                    // Best is this solution if it's better than previous
                    LNSAbstractSpace* _n = dynamic_cast<LNSAbstractSpace*>(n);
                    if (_n->improving(*best, true))
                    {
                        delete best;
                        best = n->clone(shared);
                        current = n->clone(shared);
                        return n;
                    }
                    else
                        current = n;
                }
                else
                    // Problem has no solution
                    return NULL;
            }

            // We landed in this function after a previous call to next or we are currently looping
            else
            {
                // If we have run out of iterations for this intensity
                if (idle_iterations > lns_options->maxIterationsPerIntensity())
                {
                    // If we still have intensity levels, increase intensity and reset idle iterations
                    if (intensity < lns_options->maxIntensity())
                        intensity++;
                    else {
			// just restart from minimum intensity (the whole restart with inferior cost is too hard on cp)
                        intensity = lns_options->minIntensity();
                        //restart++;
                        //current = NULL;
                        idle_iterations = 0;
                        continue;
                    }
                    idle_iterations = 0;
                }

                // Handle Simulated Annealing variables
                if (neighbors_accepted > lns_options->SAneighborsAccepted())
                {
                    temperature *= lns_options->SAcoolingRate();
                    neighbors_accepted = 0;
                }

                // Initialize empty neighbour
                Space* neighbor = root->clone(shared);
                LNSAbstractSpace* _current = dynamic_cast<LNSAbstractSpace*>(current);

                // Relax (fix) current solution into neighbour
                unsigned int relaxed_variables = _current->relax(neighbor, intensity);
                LNSAbstractSpace* _neighbor = dynamic_cast<LNSAbstractSpace*>(neighbor);

                // Use neighborhood branching
                _neighbor->neighborhood_branching();

                // Depending on the constrain type, limit the cost of the neighbour
                switch (lns_options->constrainType()) {
                    case LNS_CT_LOOSE:
                        _neighbor->constrain(*current, false, 0.0);
                        break;
                    case LNS_CT_STRICT:
                        _neighbor->constrain(*current, true, 0.0);
                        break;
                    case LNS_CT_SA:
                    {
                        double p = (double) r(RAND_MAX) / (double)RAND_MAX; // p should be a uniformly random number in (0, 1]
                        double delta = -temperature * std::log(p);
                        _neighbor->constrain(*current, false, delta);
                    }
                        break;
                    case LNS_CT_NONE:
                    default:
                        break;
                }

                // Check for space status before solving
                Space* n = NULL;
                SpaceStatus neighbor_status = neighbor->status(stats);
                if (neighbor_status == SS_SOLVED)
                    n = neighbor;
                else if (neighbor_status == SS_FAILED)
                {
                    delete neighbor;
                    n = NULL;
                }

                // If status is still unsolved, optimize
                else
                {
                    e->reset(neighbor);
                    TimeStop* t_stop = dynamic_cast<TimeStop*>(e_stop);

                    // Set time limit
                    if (lns_options->neighborTime() > 0) {
                        double m = lns_options->perVariable() ? 1 : 0;
                        dynamic_cast<TimeStop*>(e_stop)->limit(lns_options->neighborTime() * (relaxed_variables * m));
                        dynamic_cast<TimeStop*>(e_stop)->reset();
                    }
                    else {
                        // Run until a solutions has been found, but not past overall LNS stopping criterion
                        e_stop = m_stop;
                    }

                    // If we want to stop at first neighbour
                    if (lns_options->stopAtFirstNeighbor())
                    {
                        n = e->next();
                    }
                    else
                    {
                        // Find all solutions until time is up
                        std::list<Space*> prev_solutions;
                        do
                            prev_solutions.push_back(e->next());
                        while (prev_solutions.back() != NULL);

                        // Remove the last NULL solution if was generated by mistake
                        if (prev_solutions.size() > 1)
                            prev_solutions.pop_back();

                        // Remove intermediate solutions, n is last solution
                        n = prev_solutions.back();
                        prev_solutions.pop_back();
                        for (std::list<Space*>::iterator it = prev_solutions.begin(); it != prev_solutions.end(); it++)
                            delete *it;
                    }

                    // Restore e_stop if it has been changed
                    if (e_stop == m_stop)
                        e_stop = t_stop;

                }

                // If found a neighbour
                if (n != NULL)
                {
                    neighbors_accepted++;
                    LNSAbstractSpace* _n = dynamic_cast<LNSAbstractSpace*>(n);

                    // Improving move: replace current, reset search
                    if (_n->improving(*best, true))
                    {
                        delete best;
                        best = n->clone(shared);
                        delete current;
                        current = n->clone(shared);
                        idle_iterations = 0;
                        intensity = lns_options->minIntensity();
                        return n;
                    }

                    // Side move: replace current, but do not reset search
                    else if (lns_options->constrainType() == LNS_CT_SA || lns_options->constrainType() == LNS_CT_NONE || _n->improving(*current, lns_options->constrainType() == LNS_CT_STRICT))
                    {
                        delete current;
                        current = n->clone(shared);
                        delete n;
                        n = NULL;
                    }
                }

                // If the overall search has been stopped
                if (m_stop != NULL && m_stop->stop(statistics(), opt))
                {
                    // eventually ask to restart
                    if (n != NULL)
                        delete n;
                    if (neighbor != NULL)
                    {
                        //cerr << "Neighbor is not null, and it's ok to erase it." << endl;
                        //delete neighbor;
                    }
                    if (current != NULL)
                        delete current;
                    current = NULL;
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
        current = s;
        LNSAbstractSpace* _s = dynamic_cast<LNSAbstractSpace*>(s);
        if (best && _s->improving(*best, true))
        {
            delete best;
            best = s->clone(shared);
        }
        idle_iterations = 0;
        intensity = lns_options->minIntensity();
        neighbors_accepted = 0;
        temperature = lns_options->SAstartTemperature();
    }

    LNS::~LNS(void) {
        // Deleting e also deletes stop
        delete e;
    }

}}}

// STATISTICS: search-other
