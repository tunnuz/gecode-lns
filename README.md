# LNS meta-engine for GECODE (v4)

This repository contains a LNS meta-engine for GECODE (v4).

In order to test it, a patch (`gecode_lns.patch`) should be applied to the `gecode/search.hh` include file in order to enable *friendship* of the `BaseEngine` class with `LNS`.

Currently there are a few workarounds (in order of relevance):

* the parameters for LNS (i.e., `LNSOptions`) are passed to the LNS engine through a static reference to the options object (i.e., similarly to a global variable) [hard]
* the post of branchers has to be decoupled from model posting (see the methods to be implemented in `lns_space.h`) [medium]
* a few additional methods have to be added to enable LNS [medium]
* the `script::run` method does not currently handle meta-engines (such as LNS) as actual engines, therefore the partial template instantiation should take place explicitly (i.e., in `LNSTSP`) [easy]
