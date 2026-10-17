# Copyright (C) 2006 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

include BuildTools/Makemain.inc

########################################################################
#                          Subdirectories                              #
########################################################################

SUBDIRS = src

########################################################################
#                           Extra Targets                              #
########################################################################

# The unit test builds and runs the BAC example
test: all
	cd test/BAC; $(MAKE) test

unitTest: test

# Doxygen documentation

clean-local: clean-doxygen-docs
	if test -r test/BAC/Makefile; then cd test/BAC; $(MAKE) clean; fi

distclean-local:
	if test -r test/BAC/Makefile; then cd test/BAC; $(MAKE) distclean; fi

install-exec-local: install-doc

uninstall-local: uninstall-doc uninstall-doxygen-docs

.PHONY: test unitTest

########################################################################
#                         Installation of .pc files                    #
########################################################################

pkgconfiglib_DATA = bcp.pc
