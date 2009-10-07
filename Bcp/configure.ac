# Copyright (C) 2006, 2007 International Business Machines.
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
AC_COIN_DEBUG_COMPILE(Bcp)

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

AC_COIN_HAS_USER_LIBRARY([Mpi], [MPI], [mpi.h], )
# ToDo: Automatically choose MPI compiler
# AC_COIN_HAS_USER_LIBRARY([Pvm], [PVM], [pvm3.h], [???])

AC_COIN_CHECK_HEADER(sys/resource.h)
AC_COIN_CHECK_HEADER(sys/sysinfo.h)
AC_COIN_CHECK_HEADER(unistd.h)
AC_COIN_CHECK_HEADER(process.h)
AC_CHECK_FUNCS(gethostname getpid mallinfo)

# Check if sysinfo is present and does what we expect it to do
if test x$ac_cv_header_sys_stat_h = xyes; then
  AC_MSG_CHECKING([whether we have sysinfo that we can use])
  AC_LANG_PUSH(C++)
  AC_COMPILE_IFELSE([
#include <sys/sysinfo.h>
long bla()
{
  struct sysinfo info;
  sysinfo(&info);
  return info.mem_unit*info.freeram;
}],
                    [AC_MSG_RESULT([yes])
                     AC_DEFINE([HAVE_SYSINFO], [1], [Define to 1 if sysinfo function exists and runs with Bcp code])],
                    [AC_MSG_RESULT([no])])
  AC_LANG_POP(C++)
fi

##############################################################################
#                   VPATH links for example input files                      #
##############################################################################
AC_PROG_LN_S
AC_COIN_CHECK_VPATH

AC_COIN_LINK_FROM_FILELIST(test/BAC, examples/BAC/testfiles.txt)
AC_COIN_COPY_FROM_FILELIST(examples/BAC, examples/BAC/examplefiles.txt)

AC_COIN_COPY_FROM_FILELIST(examples/MCF-1, examples/MCF-1/examplefiles.txt)
AC_COIN_COPY_FROM_FILELIST(examples/MCF-2, examples/MCF-2/examplefiles.txt)
AC_COIN_COPY_FROM_FILELIST(examples/MCF-3, examples/MCF-3/examplefiles.txt)

# In case this is a VPATH configuration we need to make sure that the
# input files for the examples are available in the VPATH directory.

# ToDo
#AC_COIN_VPATH_LINK(examples/Abc/abc.par)
#AC_COIN_VPATH_LINK(examples/input.130)

##############################################################################
#                   Copy over the examples                                   #
##############################################################################

AC_DEFUN([AC_COPY_FROM_FILE], 
[
	if test -f $2 ; then 
		AC_MSG_CHECKING(Creating target dir $1)
	    mkdir -p $1
		cd $1
		for f in `cat $srcdir/$2` ; do
			if test ! -e $srcdir/`dirname $2`/$f ; then
			    mkdir -p `dirname $f`
			    $LN_S $srcdir/`dirname $2`/$f $f
			fi
		done
	fi
])

##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/BAC/Makefile
                 examples/BAC/Makefile
                 examples/MCF-1/Makefile
                 examples/MCF-2/Makefile
                 examples/MCF-3/Makefile
		 bcp_addlibs.txt])

# Here put the location and name of the configuration header file
AC_CONFIG_HEADER([inc/config_bcp.h])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE
