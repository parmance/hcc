# phsa-hcc

This is a fork of HCC that works on top of the HSA standard. It is tested
against the Portable HSA (phsa) stack, an open source implementation of HSA
utilizing the gcc's BRIG frontend for HSAIL finalization.

phsa-hcc was forked from the hcc at a point of development before upstream HCC
removed the HSAIL backend support.

The goal of the phsa-hcc fork is to make it a robust compiler that provides
the various higher-level heterogeneous programming model options provided
by HCC for HSA-supported devices.

The primary programming model of interest is the C++17 Parallel STL, which
allows heterogeneous computation offloading using the C++ standard.

See the original HCC's [README.TXT](https://raw.githubusercontent.com/parmance/hcc/phsa/README.TXT) for more info on HCC project.

## Using HCC with PHSA

First, build and install [phsa](https://github.com/HSAFoundation/phsa) as
instructed. However, instead of using the master branch of the phsa-runtime,
use the one 'hcc' branch found [here](https://github.com/parmance/phsa-runtime/tree/hcc). It has some minor modifications needed to make hcc run.

After the phsa stack is installed, building HCC is done with the following
commands:

```
git clone https://github.com/parmance/hcc -b phsa
cd hcc
mkdir build && cd build
cmake -DHSA_ROOT=../../phsa-runtime/ -DHSA_LIBRARY_DIR=$PWD/../../phsa-runtime/build/src/ ..
make world all
cd ../..
```

Note: You need to replace the above '../../phsa-runtime/' and '../../phsa-runtime/build/src/'
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
--cxxflags --ldflags` hcc/tests/Unit/ParallelSTL/reduce_carray.cpp -o reduce_carray -I hcc/include/ -L hcc/build/build/Release/lib/
HCC_RUNTIME=HSA ./reduce_carray
echo $?
```

This builds and runs the array reduction unit test of HCC using HSA.
The echo command should print out 0 if the execution was a success.

## Status

For the Parallel STL tests, the status when executing on top of phsa on an
x86_64 CPU Agent is the following (2017-07-05 status):

```
Expected Passes    : 117
Expected Failures  : 6
Unexpected Failures: 6
```

You can run the Parallel STL tests only with the following command:

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
