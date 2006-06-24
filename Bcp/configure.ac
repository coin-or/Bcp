# Copyright (C) 2006 International Business Machines.
# All Rights Reserved.
# This file is distributed under the Common Public License.

## $Id$

# Author:  Laszlo Ladanyi             IBM    2006-06-17

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_PREREQ(2.59)

AC_INIT([Bcp],[0.1],[https://projects.coin-or.org/Bcp/newticket])

AC_COPYRIGHT([
Copyright 2006 International Business Machines and others.
All Rights Reserved.
This file is part of the open source package Coin which is distributed
under the Common Public License.])

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(src/include/BCP_tm.hpp)

# Where should everything be installed by default?  Here, we want it
# to be installed directly in 'bin', 'lib', 'include' subdirectories
# of the directory where configure is run.  The default would be
# /usr/local.
AC_PREFIX_DEFAULT([`pwd`])

#############################################################################
#                         Standard build tool stuff                         #
#############################################################################

# Get the system type
AC_CANONICAL_BUILD

# If this project depends on external projects, the Externals file in
# the source root directory contains definition of where to find those
# externals.  The following macro ensures that those externals are
# retrieved by svn if they are not there yet.
AC_COIN_PROJECTDIR_INIT

# Check if user wants to produce debugging code
AC_COIN_DEBUG_COMPILE

# Get the name of the C++ compiler and appropriate compiler options
AC_COIN_PROG_CXX

# We filter out the -pedantic-errors flag for C and C++ compiler in
# case the user wants to use MPI, since otherwise the compilation
# fails
if test "${with_mpi_lib+set}" = set; then
  CFLAGS=`echo $CFLAGS | sed -e s/-pedantic-errors//g`
  CXXFLAGS=`echo $CXXFLAGS | sed -e s/-pedantic-errors//g`
fi

# Initialize automake and libtool
AC_COIN_INIT_AUTO_TOOLS

#############################################################################
#                              COIN components                              #
#############################################################################

AC_COIN_HAS_PROJECT(Bcp)
AC_COIN_HAS_PROJECT(CoinUtils)
AC_COIN_HAS_PROJECT(Vol)
AC_COIN_HAS_PROJECT(Clp)
AC_COIN_HAS_PROJECT(Cgl)
AC_COIN_HAS_PROJECT(Osi)

#############################################################################
#                             Other dependencies                            #
#############################################################################

AC_COIN_HAS_USER_LIBRARY([Mpi], [MPI], [mpi.h], [MPI_Irecv])
# ToDo: Automatically choose MPI compiler
# AC_COIN_HAS_USER_LIBRARY([Pvm], [PVM], [pvm3.h], [???])

AC_CHECK_HEADER(sys/resource.h)
AC_CHECK_HEADER(unistd.h)

##############################################################################
#                   VPATH links for example input files                      #
##############################################################################

# In this case, the source code is taken from the Bcp examples directory

AC_CONFIG_LINKS(
test/BAC/LP/BB_lp.cpp:examples/BAC/LP/BB_lp.cpp
test/BAC/TM/BB_tm.cpp:examples/BAC/TM/BB_tm.cpp
test/BAC/Member/BB_user_data.cpp:examples/BAC/Member/BB_user_data.cpp
test/BAC/Member/BB_init.cpp:examples/BAC/Member/BB_init.cpp
test/BAC/Member/BB.cpp:examples/BAC/Member/BB.cpp
test/BAC/Member/BB_cut.cpp:examples/BAC/Member/BB_cut.cpp
test/BAC/bb.par:examples/BAC/bb.par
test/BAC/bac.lp:examples/BAC/bac.lp
test/BAC/bac.mps:examples/BAC/bac.mps
test/BAC/bac.pdf:examples/BAC/bac.pdf
test/BAC/include/BB_user_data.hpp:examples/BAC/include/BB_user_data.hpp
test/BAC/include/BB_lp.hpp:examples/BAC/include/BB_lp.hpp
test/BAC/include/BB_init.hpp:examples/BAC/include/BB_init.hpp
test/BAC/include/BB_tm.hpp:examples/BAC/include/BB_tm.hpp
test/BAC/include/BB.hpp:examples/BAC/include/BB.hpp
test/BAC/include/BB_cut.hpp:examples/BAC/include/BB_cut.hpp
)

# In case this is a VPATH configuration we need to make sure that the
# input files for the examples are available in the VPATH directory.

# ToDo
#AC_COIN_VPATH_LINK(examples/Abc/abc.par)
#AC_COIN_VPATH_LINK(examples/input.130)

##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/BAC/Makefile
				 bcp_addlibs.txt])

# Here put the location and name of the configuration header file
AC_CONFIG_HEADER([inc/config_bcp.h])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE
