#include <gecode/search.hh>
#include "meta_lns.h"

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