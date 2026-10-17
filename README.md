# Bcp 1.4

[![A COIN-OR Project](https://coin-or.github.io/coin-or-badge.png)](https://www.coin-or.org)

Projects such as this one are maintained by a small group of volunteers under
the auspices of the non-profit [COIN-OR Foundation](https://www.coin-or.org)
and we need your help! Please consider [sponsoring our
activities](https://github.com/sponsors/coin-or) or [volunteering](mailto:volunteer@coin-or.org) to help!

[![Latest Release](https://img.shields.io/github/v/release/coin-or/Bcp?sort=semver)](https://github.com/coin-or/Bcp/releases)

_This file is auto-generated from [config.yml](.coin-or/config.yml) using the 
[generate_readme](.coin-or/generate_readme) script.
To make changes, please edit [config.yml](.coin-or/config.yml) or the generation scripts
[here](.coin-or/generate_readme) and [here](https://github.com/coin-or/coinbrew/blob/master/scripts/generate_readme)._

BCP is a parallel framework for implementing branch, cut, and price
algorithms for solving mixed integer programs (MIPs). BCP provides the user
with an object-oriented framework that can be used to develop an efficient
problem class specific MIP solver without all the implementational effort
involved with implementing a branch and bound framework from scratch.

Because BCP is an open-source framework, users have the flexibility to
customize any aspect of their BCP algorithm. BCP is appropriate for
researchers who would like to experiment with different MIP formulations, new
cut and/or variable generation techniques, branching strategies, etc., as well
as power users who would like to solve intractable problems in a parallel
environment. BCP processes the nodes of the branch-and-bound tree in parallel
by employing a master/worker model; it can be built with MPI for parallel runs
or run serially.

Bcp is written in C++ and is released as open source under the [Eclipse Public License 2.0](http://www.opensource.org/licenses/EPL-2.0).

It is distributed under the auspices of the [COIN-OR Foundation](https://www.coin-or.org)

The Bcp development site is https://github.com/coin-or/Bcp.

## CURRENT BUILD STATUS

[![Windows Builds](https://github.com/coin-or/Bcp/actions/workflows/windows-ci.yml/badge.svg?branch=stable/1.4)](https://github.com/coin-or/Bcp/actions/workflows/windows-ci.yml?query=branch%3Astable/1.4)

[![Linux and MacOS Builds](https://github.com/coin-or/Bcp/actions/workflows/linux-ci.yml/badge.svg?branch=stable/1.4)](https://github.com/coin-or/Bcp/actions/workflows/linux-ci.yml?query=branch%3Astable/1.4)

## DOWNLOAD

### Docker image

There is a Docker image that provides Bcp, as well as other projects
in the [COIN-OR Optimization
Suite](https://github.com/coin-or/COIN-OR-OptimizationSuite) [here](https://hub.docker.com/repository/docker/coinor/coin-or-optimization-suite)

### Binaries

For newer releases, binaries will be made available as assets attached to
releases in Github
[here](https://github.com/coin-or/Bcp/releases). Older binaries
are archived as part of Bcp
[here](https://www.coin-or.org/download/binary/Bcp).

Due to license incompatibilities, pre-compiled binaries may lack some
functionality. If binaries are not available for your platform for the latest
version and you would like to request them to be built and posted, feel free
to let us know in the discussion formum.

### Source

Source code can be obtained either by

 * Downloading a snapshot of the source code for the latest release version of Bcp from the
 [releases](https://github.com/coin-or/Bcp/releases) page.
 * Cloning this repository from [Github](https://github.com/coin-or/Bcp) or 
 * Using the [coinbrew](https://github.com/coin-or/coinbrew) script to get the project and all dependencies (recommended, see below).   

Below is a quick start guide for building on common platforms. More detailed
build instructions are
[here](https://coin-or.github.io/user_introduction.html).

### Dependencies

Bcp has a number of dependencies, which are detailed in
[config.yml](.coin-or/config.yml). Dependencies on other COIN-OR projects are
automatically downloaded when obtaining the source with `coinbrew`. For some
of the remaining third-party dependencies, automatic download scripts and
build wrappers are provided (and will also be automatically run for required
and recommended dependencies), while other libraries that are aeasy to obtain
must be installed using an appropriate package manager (or may come with your
OS by default). 

## BUILDING from source

The quick start assumes you are in a bash shell. 

### Using `coinbrew`

To download and build Bcp from source, execute the 
following on the command line. 
```
wget https://raw.githubusercontent.com/coin-or/coinbrew/master/coinbrew
chmod u+x coinbrew
./coinbrew fetch Bcp@1.4
./coinbrew build Bcp
```
For more detailed instructions on coinbrew, see https://coin-or.github.io/coinbrew.
The `coinbrew` script will fetch the additional projects specified in the Dependencies section of [config.yml](.coin-or/config.yml).

### Without `coinbrew` (Expert users)

 * Download the source code, e.g., by cloning the git repo https://github.com/coin-or/Bcp
 * Download and install the source code for the dependencies listed in [config.yml](.coin-or/config.yml)
 * Build the code as follows (make sure to set PKG_CONFIG_PTH to install directory for dependencies).

```
./configure -C
make
make test
make install
```

### Examples

The `Bcp/examples` directory has several applications of BCP:
 * `BAC`: a simple branch-and-cut application (`make test` builds and runs
   it as the unit test),
 * `MaxCut`: a branch-and-cut algorithm for the max-cut problem,
 * `MCF-1`, `MCF-2`, `MCF-3`: column generation for multicommodity flow
   problems.

After building and installing BCP, they can be built by running `make` in
the corresponding subdirectory of the build directory, e.g.,
```
cd build/Bcp/1.4/examples/BAC
make
./bac bb.par
```
The `INSTALL` and `README` files in `Bcp/examples/BAC` give more information
on customizing BCP.

## Doxygen Documentation

If you have `Doxygen` available, you can build a HTML documentation by typing

`make doxydoc` 

in the build directory. If Bcp was built via `coinbrew`, then the build
directory will be `./build/Bcp/1.4` by default. The doxygen documentation main file
is found at `<build-dir>/doxydoc/html/index.html`.

If you don't have `doxygen` installed locally, you can use also find the
documentation [here](http://coin-or.github.io/Bcp/Doxygen).


## Project Links

 * [Code of Conduct](https://www.coin-or.org/code-of-conduct/)
 * [COIN-OR Web Site](http://www.coin-or.org/)
 * [COIN-OR general discussion forum](https://github.com/orgs/coin-or/discussions)
 * [Bcp Discussion forum](https://github.com/coin-or/Bcp/discussions)
 * [Report a bug](https://github.com/coin-or/Bcp/issues/new)
 * [Doxygen generated documentation](http://coin-or.github.io/Bcp/Doxygen)

