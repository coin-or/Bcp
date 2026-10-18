# Copyright (C) 2006 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_INIT([Bcp],[devel],[https://github.com/coin-or/Bcp/issues/new],[coin-or-bcp],[https://github.com/coin-or/Bcp])

AC_COPYRIGHT([
Copyright 2006 International Business Machines and others.
All Rights Reserved.
This file is part of the open source package Coin which is distributed
under the Eclipse Public License.])

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(src/include/BCP_tm.hpp)

# Do some initialization work (version numbers, change prefix default, ...)
AC_COIN_INITIALIZE

#############################################################################
#                         Standard build tool stuff                         #
#############################################################################

# Get the name of the C++ compiler and appropriate compiler options
AC_COIN_PROG_CXX

# This is a C++ package, set the language accordingly.
AC_LANG_PUSH(C++)

# Initialize libtool
AC_COIN_PROG_LIBTOOL

# set RPATH_FLAGS to the compiler link flags required to hardcode location
# of the shared objects (expanded_libdir is set somewhere in configure before)
# (used in the examples)
AC_COIN_RPATH_FLAGS([$expanded_libdir])

#############################################################################
#                              COIN-OR components                           #
#############################################################################

m4_foreach_w([myvar],[CoinUtils Osi],
  [ AC_COIN_CHK_PKG(myvar,[BcpLib Bac])
    if test $coin_has_[]m4_tolower(myvar) != yes ; then
      AC_MSG_ERROR([Required package myvar is not available.])
    fi
  ])

# LP solvers (through their Osi interfaces); Bcp needs at least one
AC_COIN_CHK_PKG(Clp,[BcpLib Bac],[osi-clp])
AC_COIN_CHK_PKG(Cpx,[BcpLib Bac],[osi-cplex])
if test $coin_has_clp != yes && test $coin_has_cpx != yes ; then
  AC_MSG_ERROR([No LP solver available. Bcp requires an LP solver])
fi

# Vol can be used to solve the LP relaxations
AC_COIN_CHK_PKG(Vol,[BcpLib Bac],[osi-vol])

# Cgl is used by the branch-and-cut example (BAC), which is also the test
AC_COIN_CHK_PKG(Cgl,[Bac])

#############################################################################
#                             Other dependencies                            #
#############################################################################

# MPI for parallel runs; only used if requested with --with-mpi,
# --with-mpi-lflags, or --with-mpi-cflags
AC_COIN_CHK_LIB(MPI,[BcpLib],[-lmpi],[],[],[MPI_Irecv],[#include "mpi.h"],
                [no])

AC_CHECK_HEADERS([sys/resource.h sys/sysinfo.h unistd.h process.h])
AC_CHECK_FUNCS(gethostname getpid mallinfo)

# Check if sysinfo is present and does what we expect it to do
if test x$ac_cv_header_sys_stat_h = xyes; then
  AC_MSG_CHECKING([whether we have sysinfo that we can use])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <sys/sysinfo.h>
long bla()
{
  struct sysinfo info;
  sysinfo(&info);
  return info.mem_unit*info.freeram;
}])],
    [AC_MSG_RESULT([yes])
     AC_DEFINE([HAVE_SYSINFO], [1], [Define to 1 if sysinfo function exists and runs with Bcp code])],
    [AC_MSG_RESULT([no])])
fi

##############################################################################
#                   Links and copies of the example files                    #
##############################################################################

AC_PROG_LN_S

# test/BAC builds the BAC example as the unit test, so it gets links to the
# files listed in examples/BAC/testfiles.txt. In a VPATH configuration, the
# example directories get copies of the files listed in their
# examplefiles.txt.
abs_source_dir=`cd $srcdir; pwd`
AC_COIN_CHECK_VPATH
m4_define([BCP_LINKCOPY_FROM_FILELIST],
[if test -r $srcdir/$2 ; then
  AC_MSG_NOTICE([Creating $3 in $1])
  bcp_dirname=`AS_DIRNAME([$2])`
  # strip CRs, in case git converted the file list to CRLF (Windows)
  for i in `tr -d '\r' < $srcdir/$2` ; do
    if test ! -e $1/$i ; then
      bcp_dirn2=`AS_DIRNAME([$1/$i])`
      AS_MKDIR_P([$bcp_dirn2])
      m4_if([$3],[links],[$LN_S],[cp]) $abs_source_dir/$bcp_dirname/$i $1/$i
    fi
  done
else
  AC_MSG_WARN([File list file $2 missing!])
fi
])
BCP_LINKCOPY_FROM_FILELIST([test/BAC],[examples/BAC/testfiles.txt],[links])
if test $coin_vpath_config = yes ; then
  BCP_LINKCOPY_FROM_FILELIST([examples/BAC],[examples/BAC/examplefiles.txt],[copies])
  BCP_LINKCOPY_FROM_FILELIST([examples/MaxCut],[examples/MaxCut/examplefiles.txt],[copies])
  BCP_LINKCOPY_FROM_FILELIST([examples/MCF-1],[examples/MCF-1/examplefiles.txt],[copies])
  BCP_LINKCOPY_FROM_FILELIST([examples/MCF-2],[examples/MCF-2/examplefiles.txt],[copies])
  BCP_LINKCOPY_FROM_FILELIST([examples/MCF-3],[examples/MCF-3/examplefiles.txt],[copies])
fi

##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Doxygen documentation
AC_COIN_DOXYGEN([CoinUtils Osi Vol Clp Cgl])

# Finalize libs and cflags.
AC_COIN_FINALIZE_FLAGS([BcpLib Bac])

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/BAC/Makefile
                 examples/BAC/Makefile
                 examples/MCF-1/Makefile
                 examples/MCF-2/Makefile
                 examples/MCF-3/Makefile
                 examples/MaxCut/Makefile
                 bcp.pc])

AC_CONFIG_FILES([doxydoc/doxygen.conf])

# Here put the location and name of the configuration header file
AC_CONFIG_HEADERS([src/include/config.h src/include/config_bcp.h])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE
