#ifndef _LNS_H
#define _LNS_H

#include <gecode/kernel.hh>

namespace Gecode {

  /** Meta-engine performing Large Neighborhood Search (LNS) based on a given engine. */
  template<template<class> class E, class T>
  class LNS : public EngineBase {
  public:
    
    /** Initialize the engine with a space
        @param s initial solution
     */
    LNS(T* s, const Search::Options& o);
    
    ~LNS(void);
    
    /** Find next solution
        @return the next solution or NULL if there is no solution or the engine has been stopped
     */
    T* next(void);
    
    Search::Statistics stats;
    Search::Statistics statistics(void) const;
    
    /** Checks whether the engine has been stopped */
    bool stopped(void) const;
  
  protected:
  
    /** Engine to use for the exploration of the neighborhood */
    E<T>* engine;
    
    /** Engine used to find the initial solution */
    E<T>* start_engine;
  };

  /** Perform LNS starting with a given solution */
  template<template<class> class E, class T>
  T* lns(T* s, const Search::Options& o);

}

#include <gecode/search/support.hh>
#include <gecode/driver.hh>

namespace Gecode {
  
  /** Option modeling the way a relaxed solution is constrained before the LNS step 
   */
  enum LNSConstrainType
  {
    /** The solution is not constrained */
    LNS_CT_NONE,
    
    /** The solution is loosely constrained, i.e., spaces of equal cost are considered solutions */
    LNS_CT_LOOSE,
    
    /** The solution is strictly constrained, i.e., only spaces of improving cost are considered solutions */
    LNS_CT_STRICT,
    
    /** The solution is constrained with a delta, i.e., spaces with a cost that is delta units greater than the current one are considered solutions (worsening solutions allowed) */
    LNS_CT_SA
  };
  
  /** A class representing instance (and search) options specific to LNS */
  class LNSInstanceOptions : public InstanceOptions {
  public:
    
    /** Constructor, posts options on the command line, provides accessors */
    LNSInstanceOptions(const char* s)
    : InstanceOptions(s),
    _time_per_variable("-lns_time_per_variable", "LNS: the time to grant for neighborhood exploration to each relaxed variable (in milliseconds)", 10.0),
    _constrain_type("-lns_constraint_type", "LNS: the type of constrain function to be applied to search (default: none, other values: strict, sa, gd)", LNS_CT_NONE),
    _max_iterations_per_intensity("-lns_max_iterations_per_intensity", "LNS: max iterations before increasing relaxation intensity", 1),
    _min_intensity("-lns_min_intensity", "LNS: the minimum relaxation intensity", 1),
    _max_intensity("-lns_max_intensity", "LNS: the maximum relxation intensity", 0),
    _sa_start_temperature("-lns_sa_start_temperature", "LNS(SA): start temperature", 1.0),
    _sa_cooling_rate("-lns_sa_cooling_rate", "LNS(SA): cooling rate", 0.99),
    _sa_neighbors_accepted("-lns_sa_neighbors_accepted", "LNS(SA): neighbors accepted per temperature", 100)    
    {
      _constrain_type.add(LNS_CT_NONE, "none");
      _constrain_type.add(LNS_CT_LOOSE, "loose");
      _constrain_type.add(LNS_CT_STRICT, "strict");
      _constrain_type.add(LNS_CT_SA, "sa");
      
      add(_time_per_variable);
      add(_constrain_type);
      add(_max_iterations_per_intensity);
      add(_min_intensity);
      add(_max_intensity);
      add(_sa_start_temperature);
      add(_sa_cooling_rate);
      add(_sa_neighbors_accepted);
    }
    
    double timePerVariable(void) const { return _time_per_variable.value(); }
    void timePerVariable(double v) { _time_per_variable.value(v); }
    
    LNSConstrainType constrainType(void) const { return static_cast<LNSConstrainType>(_constrain_type.value()); }
    void constrainType(LNSConstrainType v) { _constrain_type.value(v); }
    
    unsigned int maxIterationsPerIntensity(void) const { return _max_iterations_per_intensity.value(); }
    void maxIterationsPerIntensity(unsigned int v) { _max_iterations_per_intensity.value(v); }
    
    unsigned int minIntensity(void) const { return _min_intensity.value(); }
    void minIntensity(unsigned int v) { _min_intensity.value(v); }
    
    unsigned int maxIntensity(void) const { return _max_intensity.value(); }
    void maxIntensity(unsigned int v) { _max_intensity.value(v); }
    
    double SAstartTemperature(void) const { return _sa_start_temperature.value(); }
    void SAstartTemperature(double v) { _sa_start_temperature.value(v); }
    
    double SAcoolingRate(void) const { return _sa_cooling_rate.value(); }
    void SAcoolingRate(double v) { _sa_cooling_rate.value(v); }
    
    unsigned int SAneighborsAccepted(void) const { return _sa_neighbors_accepted.value(); }
    void SAneighborsAccepted(unsigned int v) { _sa_neighbors_accepted.value(v); }
        
  protected:
    
    LNSInstanceOptions(const LNSInstanceOptions& opt)
    : InstanceOptions(opt.instance()), _time_per_variable(opt._time_per_variable), _constrain_type(opt._constrain_type), _max_iterations_per_intensity(opt._max_iterations_per_intensity),
_min_intensity(opt._min_intensity), _max_intensity(opt._max_intensity),
    _sa_start_temperature(opt._sa_start_temperature), _sa_cooling_rate(opt._sa_cooling_rate), _sa_neighbors_accepted(opt._sa_neighbors_accepted)
    {}
    
    Driver::DoubleOption _time_per_variable;
    Driver::StringOption _constrain_type;
    Driver::UnsignedIntOption _max_iterations_per_intensity;
    Driver::UnsignedIntOption _min_intensity;
    Driver::UnsignedIntOption _max_intensity;
    
    // FIXME: add SA parameters
    Driver::DoubleOption _sa_start_temperature;
    Driver::DoubleOption _sa_cooling_rate;
    Driver::UnsignedIntOption _sa_neighbors_accepted;
  };
}

namespace Gecode { namespace Search {
  
  /** This class implements a combined stop criterion for LNS-based meta-engines the
      underlying engine is handled through a TimeStop, while the overall stop object
      is by the user, e.g., through the script controlling the meta-engine. 
   */
  class LNSMetaStop : public Stop {
  protected:
    
    /** "Real" stop object, as provided by the user */
    Stop* lns_stop;
    
    /** Internal stop object, i.e., for use in the LNS step */
    TimeStop* e_stop;
    
  public:
    
    LNSMetaStop(Stop* lns_stop0, TimeStop* e_stop0) : lns_stop(lns_stop0), e_stop(e_stop0)
    { }
   
    /** Verify the stopping condition, i.e., wether any of the two stop object is active */
    virtual bool stop(const Statistics& s, const Options& o) {
      return (e_stop != NULL && e_stop->stop(s,o)) || (lns_stop != NULL && lns_stop->stop(s,o));
    }
  };
  
  /** FIXME: waiting for a more integrated (and not intrusive) solution, this class is abused
      for passing specific parameters to the LNS engine */
  class LNSParameters : public LNSInstanceOptions {
  public:
    LNSParameters(const LNSInstanceOptions& opt0) : LNSInstanceOptions(opt0) {}
  };
}}

#include "meta_lns.h"

namespace Gecode {

  namespace Search {
    
    /** Instantiates a LNS engine given the neede parameters */
    GECODE_SEARCH_EXPORT Engine* lns(
      Space* s, // initial space
      size_t sz,
      TimeStop* e_stop,
      Engine* se, // start engine
      Engine* e,  // engine for LNS step
      Search::Statistics& st,
      const Options& o
    );
  }
  
  template<template<class> class E, class T>
  forceinline
  LNS<E,T>::LNS(T* s, const Search::Options& m_opt) {    
    Search::Options e_opt;
    e_opt.clone = true;
    e_opt.threads = m_opt.threads;
    e_opt.c_d = m_opt.c_d;
    e_opt.a_d = m_opt.a_d;
    Search::TimeStop* ts = new Search::TimeStop(0);
    Search::LNSMetaStop* ms = new Search::LNSMetaStop(m_opt.stop, ts);
    e_opt.stop = ms;
    Space* root;
    if (m_opt.clone) {
      if (s->status(stats) == SS_FAILED) {
        stats.fail++;
        root = new Search::FailedSpace();
      } else {
        root = s->clone();
      }
    } else {
      root = s;
    }
    engine = new E<T>(dynamic_cast<T*>(root),e_opt);
    Search::Engine* ee = engine->e; // FIXME: now this class has to be friend of BaseEngine to allow it
    engine->e = NULL;
    start_engine = new E<T>(dynamic_cast<T*>(root),m_opt);
    Search::Engine* se = start_engine->e;
    start_engine->e = NULL;
    e = Search::lns(root,sizeof(T),ts,se,ee,stats,m_opt);
  }

  template<template<class> class E, class T>
  forceinline T*
  LNS<E,T>::next(void) {
    return dynamic_cast<T*>(e->next());
  }

  template<template<class> class E, class T>
  forceinline Search::Statistics
  LNS<E,T>::statistics(void) const {
    return e->statistics();
  }

  template<template<class> class E, class T>
  forceinline bool
  LNS<E,T>::stopped(void) const {
    return e->stopped();
  }
  
  
  template<template<class> class E, class T>
  forceinline 
  LNS<E,T>::~LNS(void) {
    //delete engine;
  }

  template<template<class> class E, class T>
  forceinline T*
  lns(T* s, const Search::Options& o) {    
    LNS<E,T> l(s,o);
    return l.next();
  }
}

#endif