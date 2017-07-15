# hcc-phsa

This is a fork of HCC that works on top of the HSA standard. The goal of
the hcc-phsa fork is to make it a robust compiler that provides the various
higher-level heterogeneous programming model options provided by HCC for
HSA-supported devices. It is tested against the Portable HSA (phsa) stack
which is an open source implementation of HSA utilizing the gcc's BRIG
frontend for HSAIL finalization.

The primary programming model of interest is currently C++17 Parallel STL which
allows heterogeneous computation offloading using the C++ standard.

hcc-phsa was forked from the hcc at a point of development before upstream HCC
removed the HSAIL backend support. See the original HCC's 
[README.TXT](https://raw.githubusercontent.com/parmance/hcc/phsa/README.TXT)
for more info. The HCC
project's upstream repository is [here](https://github.com/RadeonOpenCompute/hcc).

The hcc-phsa work is being done by [Parmance's](http://parmance.com) developers
for [General Processor Technologies](http://www.generalprocessortech.com/).

## Using HCC with PHSA

First, build and install [phsa](https://github.com/HSAFoundation/phsa) as
instructed. However, instead of using the master branch of the phsa-runtime,
use the 'hcc' branch found [here](https://github.com/parmance/phsa-runtime/tree/hcc).
It has some minor modifications needed to make hcc run.

Then install prerequisities:

On Ubuntu 16.04:

```
sudo apt-get install git cmake make g++  g++-multilib gcc-multilib \
python findutils libelf1 libpci3 file debianutils libunwind-dev \
pkg-config
```

HCC can be fetched and built as follows:

```
git clone https://github.com/parmance/hcc -b phsa
cd hcc
mkdir build && cd build
cmake -DHSA_ROOT=../../phsa-runtime/ -DHSA_LIBRARY_DIR=$PWD/../../phsa-runtime/build/src/ ..
make world all
cd ../..
```

Note: You need to replace the above *../../phsa-runtime/* and *../../phsa-runtime/build/src/*
paths to match the location where you installed and built the 'phsa' branch of
the phsa-runtime.

Before using HCC and PHSA, you need set the environment variables as
instructed by the PHSA installation instructions. In addition, the HCC libs
should be added to the LD_LIBRARY_PATH unless you installed them to a
standard location.

Now you can smoke test your HCC build by executing the following commands
in the directory where your 'hcc' repository is located:

```
hcc/build/compiler/bin/clang++ `hcc/build/build/Release/bin/hcc-config \
--cxxflags --ldflags` hcc/tests/Unit/ParallelSTL/reduce_carray.cpp \
-o reduce_carray -I hcc/include/ -L hcc/build/build/Release/lib/
HCC_RUNTIME=HSA ./reduce_carray
echo $?
```

This builds and runs the Parallel STL array reduction unit test of HCC
using HSA. The echo command should print out 0 if the execution was a success.

## Tips

**If you haven't installed gccbrig, you will encounter a problem while running
tests that -lhsail-rt cannot be found.** This is because the test script
wipes LDFLAGS, thus it is not possible to pass them to phsa-runtime for it
to find the runtime lib from a non-standard (build) location. The solution
is to either install gccbrig to a standard location, or modify *compiler/utils/lit/lit/TestingConfig.py*. If you add line

```
'LDFLAGS' : os.environ.get('LDFLAGS',''),
```

inside the environment dict initialization code, it should fix the problem.

## Status

For the Parallel STL tests, the status when executing on top of phsa on an
x86_64 CPU Agent is the following (2017-07-05 status):

```
Expected Passes    : 117
Expected Failures  : 6
Unexpected Failures: 6
```

You can run the Parallel STL tests separately with the following command:

```
cd build
HCC_RUNTIME=HSA python compiler/bin/llvm-lit -j 2 --path compiler/bin -sv tests/Unit/ --filter=ParallelSTL
```

The whole 'make check' test suite results (ran with 'HCC_RUNTIME=HSA make check')
are as follows:

```
Expected Passes    : 585
Expected Failures  : 22
Unsupported Tests  : 10
Unexpected Failures: 6
```
