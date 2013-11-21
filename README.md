# LNS meta-engine for GECODE (v4)

This repository contains a LNS meta-engine for GECODE (v4).

## How to build it

The project uses CMake to generate build files. In order to build it, run

    mkdir build; cd build
    cmake ..
    make

## Remarks

In order to test it, a patch (`hybrid_gecode.patch`) must be applied to the `gecode/search.hh` include file in order to enable *friendship* of the `BaseEngine` class with `LNS`.

Currently there are a few workarounds (in order of relevance):

* the post of branchers has to be decoupled from model posting (see the methods to be implemented in `lns_space.h`) [medium]
* a few additional methods have to be added to enable LNS [medium]
* the `script::run` method does not currently handle meta-engines (such as LNS) as actual engines, therefore the partial template instantiation should take place explicitly (i.e., in `LNSTSP`) [easy]
