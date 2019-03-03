
# Branch-Cut-Price Framework

## Getting Started

The [quick download and installation guide](https://projects.coin-or.org/BuildTools/wiki/downloadUnix) works in most cases if you don't have particular requirements.

For more detailed instructions, go to the [CoinHelp page](https://projects.coin-or.org/BuildTools/wiki/).


## The examples

If you have installed Bcp according to the instructions in the [quick download and installation guide](https://projects.coin-or.org/BuildTools/wiki/downloadUnix) you have built the code in the directory `build`.

There are two example codes installed in `build/Bcp/examples`: `MCF` and `BAC`. `MCF` is a multicommodity flow example
using column generation (the code is in development and might not be working yet). `BAC` is an example of a simple branch-and-cut application.

The INSTALL and README files in `build/Bcp/examples/BAC`
give pointers for installing and running the example, as well as
additional information for customizing the installation of BCP.



## BCP FAQs


### What is Branch-Cut-Price (BCP)?
BCP is a parallel framework for implementing branch, cut, and price algorithms for solving mixed integer programs (MIPs). BCP provides the user with an object-oriented framework that can be used to develop an efficient problem class specific MIP solver without all the implementational effort. involved with implementing a branch and bound framework from scratch.


### Who should use BCP?
Because BCP is an open-source framework, users have the flexibility to customize any aspect of their BCP algorithm. BCP is appropriate for researchers who would like to experiment with different MIP formulations, new cut and/or variable generation techniques, branching strategies, etc., as well as power users who would like to solve intractable problems in a parallel environment.


### How do I get it and how do I install it?
You should read the InstallationAndGettingStarted document.


### How is BCP parallelized?
BCP processes the Branch-and-Bound search tree nodes in parallel by employing a master/slave model. BCP is designed for a distributed network via a message passing protocol (currently PVM, or a serial version).


### Can BCP be used in a serial environment?
Yes. BCP contains a message passing module that mimics a parallel environment on a single processor.


### What kind of LP solvers can be used with BCP?
BCP uses the [OSI](https://projects.coin-or.org/Bcp), which enables the use of any LP solver that OSI is interfaced with.


### What are the main features of BCP?
Generalized branching objects, strong branching, reduced cost fixing, use of cut and variable pools, handling locally valid cuts, etc.


### Can one use BCP as a general IP solver?
Not at the moment. When BCP is interfaced with CGL and CGL is well populated, BCP could be used as a general solver.


### For what type of problems has BCP been used?
Multiple knapsack with color constraints([mkc](http://www.research.ibm.com/pdos/doc/papers/tr21138.ps)), max-cut, and some proprietary projects.




### What platforms does BCP run on?
BCP should run on any POSIX compatible unix (TM) platform.  BCP has been tested on:
 * AIX V4.3 or later using g++ or Visual Age C++ (xlC) V5.1 or greater.
 * Linux (both 32 and 64 bit) using g++
 * Solaris (SunOS 5.6 and 5.8) using g++ or CC (SUNPRO_CC V6.2 or greater)
 * Windows using g++ under Cygwin or Msys environment
 * Mac OSX (a.k.a. Darwin) using g++
 * FreeBSD using g++

For all platforms above: when g++ is used at least V2.95.3 is needed, but nothing older than V3.3 is recommended (V3.0 is particularly bad as there are a number of bugs in that compiler).

At this time BCP does not compile on Windows with MS VC++ Version 6, because this compiler does not handle templated functions as BCP requires. Contributions to compile BCP on more platforms, especially on Windows  perhaps with another compiler, are greatly appreciated.


### Is there any alternative to BCP?
[SYMPHONY](https://projects.coin-or.org/SYMPHONY) is a very similar open-source, parallel branch, cut, and price framework implemented in C. SYMPHONY compiles under MSVC++ and can also be compiled for shared memory architectures with an OpenMP compliant compiler. It has a more simplified interface and may be a better option for some users. 



## Additional information

Embedded documentation (Doxygen): [BCP class descriptions](http://www.coin-or.org/Doxygen/Bcp/index.html)

Manuals:
  * [BCP manual (PostScript)](http://www.coin-or.org/Presentations/bcp-man.ps) (out of date---the most recent version is in CVS)
  * [BCP manual (PDF)](http://www.coin-or.org/Presentations/bcp-man.pdf) (out of date---the most recent version is in CVS)

Presentations:
  * Tutorial on BCP by Ted Ralphs can be accessed from the [page on presentations from the 2004 CORS/INFORMS Joint Meeting in Banff](http://www.coin-or.org/Presentations/CORSINFORMSWorkshop04/index.html)
  * [BCP Tutorial](http://www.coin-or.org/Presentations/INFORMSWorkshop03.pdf) given by Ted Ralphs at INFORMS 2003.
  * [A Library Hierarchy for Implementing Scalable Parallel Search Algorithms](http://www.coin-or.org/Presentations/INFORMS02-ALPS.pdf): A presentation on the new parallel search framework currently under
development at INFORMS 2002 by Ted Ralphs, Laszlo Ladanyi, and Matthew Saltzman.

Publications:
  * Francois Margot, [BAC: A BCP based Branch-and-Cut Example](Bcp/examples/BAC/bac-1.3.8.pdf).
    This paper is an introduction to the Branch-and-Cut-and-Price (BCP) software from the user perspective.
    It focuses on a simple example illustrating the basic operations used in a Branch-and-Cut:cuts and heuristic solutions generation, and customized branching.
  * Laszlo Ladanyi, Jon Lee, Robin Lougee-Heimer, "Implementation of an exact algorithm for a cutting-stock problem using components of COIN-OR", _submitted for publication_, February 2003.
  * Ted K. Ralphs, Laszlo Lad&aacute;nyi, and Matthew J. Saltzman, [A Library Hierarchy for Implementing Scalable Parallel Search Algorithms](http://www.lehigh.edu/~tkr2/research/papers/JSC02.pdf), in review.
  * Ted K. Ralphs, Laszlo Lad&aacute;nyi, and Matthew J. Saltzman, [Parallel Branch, Cut, and Price for Large-scale Discrete Optimization](http://www.lehigh.edu/~tkr2/research/papers/PBCP.pdf), to appear in _Mathematical Programming_.
  * Laszlo Lad&aacute;nyi, Ted K. Ralphs, and Matthew J. Saltzman, Implementing Scalable Parallel Search Algorithms for Data-intensive Applications, _The Proceedings of the International Conference on Computational Science_ (2002), p. 592.

Examples:
  * [COIN-OR Tutorials](http://coral.ie.lehigh.edu/~coin/) page at the Lehigh Industrial and Systems Engineering Department.
  * [BAC: A BCP based Branch-and-Cut Example](Bcp/examples/BAC/bac-1.3.8.pdf) by Francois Margot.


## Project Links

 *  Help with COIN-OR: [CoinHelp page](https://projects.coin-or.org/BuildTools/wiki/).
 * [BCP html documentation](http://www.coin-or.org/Doxygen/Bcp/index.html)
 * `BCP` mailing lists:
   * For general questions: [BCP mailing list](http://list.coin-or.org/mailman/listinfo/coin-bcpdiscuss).
   * For tickets: [BCP-tickets mailing list](http://list.coin-or.org/mailman/listinfo/bcp-tickets).
 * Reporting bugs:
   * For download and installation: Go to the [WikiStart BuildTools Trac page] and post a new ticket. Follow these [instructions](http://www.coin-or.org/faqs.html#q10).
   * For a `BCP` Bug: Post a new ticket. Follow these [instructions](http://www.coin-or.org/faqs.html#q10).
 * [COIN-OR home page](http://www.coin-or.org/)
