# Copyright (C) 2006 International Business Machines.
# All Rights Reserved.
# This file is distributed under the Common Public License.

## $Id$

# Author:  Laszlo Ladanyi             IBM    2006-06-17

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_PREREQ(2.59)

AC_INIT([Bcp],[1.2.3],[https://projects.coin-or.org/Bcp/newticket])

AC_COPYRIGHT([
Copyright 2006 International Business Machines and others.
All Rights Reserved.
This file is part of the open source package Coin which is distributed
under the Common Public License.])

AC_REVISION(0.9)

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(configure.ac)

# Where should everything be installed by default?  Here, we want it
# to be installed directly in 'bin', 'lib', 'include' subdirectories
# of the directory where configure is run.  The default would be
# /usr/local.
AC_PREFIX_DEFAULT([`pwd`])

#############################################################################
#              Create the libtool script for the subdirectories             #
#############################################################################

AC_COIN_CREATE_LIBTOOL

#############################################################################
#                  Check which subprojects are there                        #
#############################################################################

AC_COIN_MAIN_SUBDIRS(CoinUtils)
AC_COIN_MAIN_SUBDIRS(Vol)
AC_COIN_MAIN_SUBDIRS(Clp)
AC_COIN_MAIN_SUBDIRS(Osi)
AC_COIN_MAIN_SUBDIRS(Cgl)
AC_COIN_MAIN_SUBDIRS(Bcp)
AC_COIN_MAIN_SUBDIRS(Applications/MaxCut)
AC_COIN_MAIN_SUBDIRS(Applications/Mkc)
AC_COIN_MAIN_SUBDIRS(Applications/Csp)

#############################################################################
#                             Other dependencies                            #
#############################################################################

AC_COIN_HAS_USER_LIBRARY([Mpi], [MPI], [mpi.h], )
# ToDo: Automatically choose MPI compiler

##############################################################################
# Copy stuff needed for the applications to build                            #
##############################################################################

# In case this is a VPATH configuration we need to make sure that the
# input files for the examples are available in the VPATH directory.

AC_COIN_VPATH_LINK(Applications/MaxCut/junger1.par)
AC_COIN_VPATH_LINK(Applications/MaxCut/junger1)

AC_COIN_VPATH_LINK(Applications/Mkc/mkc.par)
AC_COIN_VPATH_LINK(Applications/Mkc/mkc.mps.gz)

AC_COIN_VPATH_LINK(Applications/Csp/CSP.par)
AC_COIN_VPATH_LINK(Applications/Csp/Data/cspdata3.dat)
AC_COIN_VPATH_LINK(Applications/Csp/Data/cspdataJL.dat)
AC_COIN_VPATH_LINK(Applications/Csp/Data/fiber29_9080.txt)
AC_COIN_VPATH_LINK(Applications/Csp/Data/fiber29_9080_merged.txt)

##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile
		 Applications/Csp/Makefile
		 Applications/MaxCut/Makefile
		 Applications/Mkc/Makefile])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE
