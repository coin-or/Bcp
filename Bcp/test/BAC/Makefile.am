# Copyright (C) 2006 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

include $(top_srcdir)/BuildTools/Makemain.inc

########################################################################
#        unitTest: the BAC example (sources linked by configure)       #
########################################################################

noinst_PROGRAMS = unitTest

nodist_unitTest_SOURCES = 
nodist_unitTest_SOURCES += LP/BB_lp.cpp
nodist_unitTest_SOURCES += TM/BB_tm.cpp
nodist_unitTest_SOURCES += Member/BB_user_data.cpp
nodist_unitTest_SOURCES += Member/BB_packer.cpp
nodist_unitTest_SOURCES += Member/BB_init.cpp
nodist_unitTest_SOURCES += Member/BB.cpp
nodist_unitTest_SOURCES += Member/BB_cut.cpp
nodist_unitTest_SOURCES += include/BB_user_data.hpp
nodist_unitTest_SOURCES += include/BB_packer.hpp
nodist_unitTest_SOURCES += include/BB_lp.hpp
nodist_unitTest_SOURCES += include/BB_init.hpp
nodist_unitTest_SOURCES += include/BB_tm.hpp
nodist_unitTest_SOURCES += include/BB.hpp
nodist_unitTest_SOURCES += include/BB_cut.hpp

unitTest_LDADD = ../../src/libBcp.la $(BAC_LFLAGS)

AM_CPPFLAGS = -I. -Iinclude -I$(srcdir)/../../src/include \
	-I../../src/include $(BAC_CFLAGS)
AM_LDFLAGS = $(LT_LDFLAGS)

test: unitTest$(EXEEXT)
	./unitTest$(EXEEXT) -param bb.par

.PHONY: test
