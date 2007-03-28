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
AC_COIN_MAIN_SUBDIRS(Bcp)
AC_COIN_MAIN_SUBDIRS(Osi)
# The following are required for Bcp examples
AC_COIN_MAIN_SUBDIRS(Vol)
AC_COIN_MAIN_SUBDIRS(Clp)
AC_COIN_MAIN_SUBDIRS(Cgl)

#############################################################################
#                  Copy over the examples                                   #
#############################################################################

AC_COIN_CHECK_VPATH

AC_DEFUN([COIN_UPPATH],
[
if test $1 = . ; then
	$2=.
else
	t=`echo $1 | sed -e 's=/\./=/=g' -e 's=^\./==' -e 's=/\.$==' -e 's=[[^/]]*=..=g'`
	if test $t = "" ; then
		t=.
	fi
	$2=$t
fi
])

AC_DEFUN([COIN_COPY_FROM_FILELIST], 
[
if test $coin_vpath_config = yes ; then
	COIN_UPPATH([$1], updir)
	if test -e $srcdir/$2 ; then
		echo Copying example $1 ...
		fromdir=$updir/$srcdir/`dirname $2`
		mkdir -p $1
		cd $1
		for f in `cat $updir/$srcdir/$2` ; do
			if test ! -e $f ; then
		   		fdir=`dirname $f`
				mkdir -p $fdir
				COIN_UPPATH([$fdir], uppath)
				cd $fdir
				cp -p $uppath/$fromdir/$f `basename $f`
				cd $uppath
			fi
		done
		cd $updir
	fi
fi
])

COIN_COPY_FROM_FILELIST(Applications/MaxCut, 
                        Applications/MaxCut/examplefiles.txt)
COIN_COPY_FROM_FILELIST(Applications/Csp, 
                        Applications/Csp/examplefiles.txt)
#COIN_COPY_FROM_FILELIST(Applications/Mkc, 
#                        Applications/Mkc/examplefiles.txt)


##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile
                 Applications/MaxCut/Makefile
                 Applications/Csp/Makefile])
#                 Applications/Mkc/Makefile])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE
