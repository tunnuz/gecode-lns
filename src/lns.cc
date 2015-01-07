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

 #include <gecode/search.hh>
 #include "gecode-lns/meta_lns.hh"

 namespace Gecode { namespace Search {

   Engine*
   lns(Space* s, size_t sz, TimeStop* e_stop,
       Engine* se, Engine* e, Search::Statistics& st, const Options& o) {
 #ifdef GECODE_HAS_THREADS
     Options to = o.expand();
     return new Meta::LNS(s,sz,e_stop,se,e,st,to);
 #else
     return new Meta::LNS(s,sz,e_stop,se,e,st,o);
 #endif
   }

 }}

 // STATISTICS: search-other
