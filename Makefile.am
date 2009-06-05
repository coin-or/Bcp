# Copyright (C) 2006 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Common Public License.

## $Id$

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = doxydoc/doxygen.conf

########################################################################
#                          Subdirectories                              #
########################################################################

# subdirs is set by configure as the list of all subdirectories to recurse
# into
SUBDIRS = $(subdirs)

########################################################################
#                           Extra Targets                              #
########################################################################

test: all
	cd Bcp; $(MAKE) test

unitTest: test

tests:
	for dir in $(subdirs); do \
	  if test -r $$dir/test/Makefile; then \
	    (cd $$dir; $(MAKE) test) \
	  fi; \
	done

unitTests: tests

doxydoc:
	cd $(srcdir); doxygen doxydoc/doxygen.conf

.PHONY: test unitTest tests unitTests doxydoc
.PHONY: doxydoc

########################################################################
#                         Maintainer Stuff                             #
########################################################################

# Files that are generated and should be cleaned with make distclean
DISTCLEANFILES = \
	Applications/Mkc/mkc@EXEEXT@ \
	Applications/Mkc/mkc@EXEEXT@ \
	Applications/Mkc/MKC_lp__branch.@OBJEXT@ \
	Applications/Mkc/MKC_lp.@OBJEXT@ \
	Applications/Mkc/MKC_lp__param.@OBJEXT@ \
	Applications/Mkc/MKC_tm_param.@OBJEXT@ \
	Applications/Mkc/MKC_tm.@OBJEXT@ \
	Applications/Mkc/MKC_vargen.@OBJEXT@ \
	Applications/Mkc/MKC_solution.@OBJEXT@ \
	Applications/Mkc/MKC_init.@OBJEXT@ \
	Applications/Mkc/MKC_optim.@OBJEXT@ \
	Applications/Mkc/MKC_knapsack.@OBJEXT@ \
	\
	Applications/Csp/csp@EXEEXT@ \
	Applications/Csp/CSP_colgen.@OBJEXT@ \
	Applications/Csp/CSP_lp_param.@OBJEXT@ \
	Applications/Csp/CSP_lp.@OBJEXT@ \
	Applications/Csp/CSP_lp_branch.@OBJEXT@ \
	Applications/Csp/CSP_tm_param.@OBJEXT@ \
	Applications/Csp/CSP_tm.@OBJEXT@ \
	Applications/Csp/CSP_CSPROBLEM.@OBJEXT@ \
	Applications/Csp/CSP_init.@OBJEXT@ \
	Applications/Csp/CSP_userexits.@OBJEXT@ \
	\
	Applications/MaxCut/maxcut@EXEEXT@ \
	Applications/MaxCut/MC_cutgen.@OBJEXT@ \
	Applications/MaxCut/MC_ising_cycles.@OBJEXT@ \
	Applications/MaxCut/MC_shortest_path_cycle.@OBJEXT@ \
	Applications/MaxCut/MC_lp_branch.@OBJEXT@ \
	Applications/MaxCut/MC_lp_param.@OBJEXT@ \
	Applications/MaxCut/MC_lp.@OBJEXT@ \
	Applications/MaxCut/MC_tm.@OBJEXT@ \
	Applications/MaxCut/MC_tm_param.@OBJEXT@ \
	Applications/MaxCut/MC_kruskal.@OBJEXT@ \
	Applications/MaxCut/MC.@OBJEXT@ \
	Applications/MaxCut/MC_mst_heur.@OBJEXT@ \
	Applications/MaxCut/MC_INST.@OBJEXT@ \
	Applications/MaxCut/MC_cut.@OBJEXT@ \
	Applications/MaxCut/MC_init.@OBJEXT@ \
	Applications/MaxCut/MC_solution.@OBJEXT@

include BuildTools/Makemain.inc
