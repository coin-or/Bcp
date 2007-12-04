# Copyright (C) 2006 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Common Public License.

## $Id$

AUTOMAKE_OPTIONS = foreign

########################################################################
#                               libBcp                                 #
########################################################################

# Name of the library compiled in this directory.  We want it to be installed
# in the 'lib' directory
lib_LTLIBRARIES = libBcp.la

# List all source files for this library, including headers
libBcp_la_SOURCES = \
	CG/BCP_cg_user.cpp \
	CG/BCP_cg_param.cpp \
	CG/BCP_cg.cpp \
	CG/BCP_cg_main.cpp \
	LP/BCP_lp_fathom.cpp \
	LP/BCP_lp_branch.cpp \
	LP/BCP_lp_branching.cpp \
	LP/BCP_lp_misc.cpp \
	LP/BCP_lp_main_loop.cpp \
	LP/BCP_lp_msgproc.cpp \
	LP/BCP_lp_user.cpp \
	LP/BCP_lp_generate_vars.cpp \
	LP/BCP_lp_msg_node_rec.cpp \
	LP/BCP_lp_node.cpp \
	LP/BCP_lp_create_lp.cpp \
	LP/BCP_lp_msg_node_send.cpp \
	LP/BCP_lp_convert_OsiWarmStart.cpp \
	LP/BCP_lp_result.cpp \
	LP/BCP_lp_generate_cuts.cpp \
	LP/BCP_lp_colrow.cpp \
	LP/BCP_lp.cpp \
	LP/BCP_lp_main.cpp \
	LP/BCP_lp_pool.cpp \
	LP/BCP_lp_param.cpp \
	TM/BCP_tm_trimming.cpp \
	TM/BCP_tm_msg_node_send.cpp \
	TM/BCP_tm_msg_node_rec.cpp \
	TM/BCP_tm_param.cpp \
	TM/BCP_tm_user.cpp \
	TM/BCP_tm_commandline.cpp \
	TM/BCP_tm_node.cpp \
	TM/BCP_tm_nodes_to_storage.cpp \
	TM/BCP_tm_msgproc.cpp \
	TM/BCP_tm_functions.cpp \
	TM/BCP_tm_main.cpp \
	TM/BCP_tm_statistics.cpp \
	TM/BCP_tm.cpp \
	TM/BCP_tmstorage.cpp \
	VG/BCP_vg_param.cpp \
	VG/BCP_vg_user.cpp \
	VG/BCP_vg_main.cpp \
	VG/BCP_vg.cpp \
	Member/BCP_solution.cpp \
	Member/BCP_node_change.cpp \
	Member/BCP_indexed_pricing.cpp \
	Member/BCP_USER.cpp \
	Member/BCP_warmstart_basis.cpp \
	Member/BCP_branch.cpp \
	Member/BCP_vector_sanity.cpp \
	Member/BCP_cut.cpp \
	Member/BCP_obj_change.cpp \
	Member/BCP_warmstart_dual.cpp \
	Member/BCP_warmstart_primaldual.cpp \
	Member/BCP_matrix.cpp \
	Member/BCP_problem_core.cpp \
	Member/BCP_process.cpp \
	Member/BCP_warmstart_pack.cpp \
	Member/BCP_matrix_pack.cpp \
	Member/BCP_var.cpp \
	Member/BCP_vector_bool.cpp \
	Member/BCP_vector_char.cpp \
	Member/BCP_vector_double.cpp \
	Member/BCP_vector_int.cpp \
	Member/BCP_vector_short.cpp \
	include/BCP_math.hpp \
	include/BCP_vg_param.hpp \
	include/BCP_message.hpp \
	include/BCP_solution.hpp \
	include/BCP_vector_change.hpp \
	include/BCP_lp_branch.hpp \
	include/BCP_process.hpp \
	include/BCP_node_change.hpp \
	include/BCP_indexed_pricing.hpp \
	include/BCP_vector.hpp \
	include/BCP_vector_char.hpp \
	include/BCP_tm_param.hpp \
	include/BCP_USER.hpp \
	include/BCP_vg_user.hpp \
	include/BCP_lp_main_loop.hpp \
	include/BCP_warmstart_basis.hpp \
	include/BCP_tm_user.hpp \
	include/BCP_lp_user.hpp \
	include/BCP_branch.hpp \
	include/BCP_warmstart.hpp \
	include/BCP_vector_sanity.hpp \
	include/BCP_cut.hpp \
	include/BCP_message_tag.hpp \
	include/BCP_tm_node.hpp \
	include/BCP_cg_user.hpp \
	include/BCP_set_intersects.hpp \
	include/BCP_cg_param.hpp \
	include/BCP_obj_change.hpp \
	include/BCP_warmstart_dual.hpp \
	include/BCP_warmstart_primaldual.hpp \
	include/BCP_vector_int.hpp \
	include/BCP_lp_node.hpp \
	include/BCP_string.hpp \
	include/BCP_matrix.hpp \
	include/BCP_parameters.hpp \
	include/BCP_enum.hpp \
	include/BCP_enum_branch.hpp \
	include/BCP_vector_general.hpp \
	include/BCP_lp_functions.hpp \
	include/BCP_functions.hpp \
	include/BCP_vector_short.hpp \
	include/BCP_problem_core.hpp \
	include/BCP_vector_double.hpp \
	include/BCP_lp_result.hpp \
	include/BCP_main_fun.hpp \
	include/BCP_tm_functions.hpp \
	include/BCP_error.hpp \
	include/BCP_enum_tm.hpp \
	include/BCP_enum_process_t.hpp \
	include/BCP_cg.hpp \
	include/BCP_vector_bool.hpp \
	include/BCP_lp.hpp \
	include/BCP_mempool.hpp \
	include/BCP_var.hpp \
	include/BCP_lp_pool.hpp \
	include/BCP_os.hpp \
	include/BCP_buffer.hpp \
	include/BCP_lp_param.hpp \
	include/BCP_tm.hpp \
	include/BCP_vg.hpp \
	include/BCP_vector_bool.hpp \
	include/BCP_vector_char.hpp \
	include/BCP_vector_double.hpp \
	include/BCP_vector_int.hpp \
	include/BCP_vector_short.hpp \
	include/BCP_tmstorage.hpp \
	Member/BCP_message_single.cpp \
	include/BCP_message_single.hpp \
	Member/BCP_message_mpi.cpp \
	include/BCP_message_mpi.hpp \
	Member/BCP_message_pvm.cpp \
	include/BCP_message_pvm.hpp \
	include/BcpConfig.h

# This is for libtool (on Windows)
libBcp_la_LDFLAGS = $(LT_LDFLAGS)

########################################################################
#                            Additional flags                          #
########################################################################

# Here list all include flags, relative to this "srcdir" directory.  This
# "cygpath" stuff is necessary to compile with native compilers on Windows.
AM_CPPFLAGS = 
AM_CPPFLAGS += -I`$(CYGPATH_W) $(COINUTILSSRCDIR)/src`
AM_CPPFLAGS += -I`$(CYGPATH_W) $(COINUTILSOBJDIR)/inc`
AM_CPPFLAGS += -I`$(CYGPATH_W) $(OSISRCDIR)/src`

# Include MPI header directory if MPI is used
if COIN_HAS_MPI
AM_CPPFLAGS +=  -I`$(CYGPATH_W) $(MPIINCDIR)`
endif

# Include PVM header directory if PVM is used
#if COIN_HAS_PVM
#AM_CPPFLAGS +=  -I`$(CYGPATH_W) $(PVMINCDIR)`
#endif

# This line is necessary to allow VPATH compilation with MS compilers
# on Cygwin
DEFAULT_INCLUDES = -I. -I`$(CYGPATH_W) $(srcdir)/include` -I$(top_builddir)/inc

########################################################################
#                Headers that need to be installed                     #
########################################################################

# Here list all the header files that are required by a user of the library,
# and that therefore should be installed in 'install'
include_HEADERS = \
	../inc/config_bcp.h \
	include/BCP_branch.hpp \
	include/BCP_buffer.hpp \
	include/BCP_cg.hpp \
	include/BCP_cg_param.hpp \
	include/BCP_cg_user.hpp \
	include/BCP_cut.hpp \
	include/BCP_enum_branch.hpp \
	include/BCP_enum.hpp \
	include/BCP_enum_process_t.hpp \
	include/BCP_enum_tm.hpp \
	include/BCP_error.hpp \
	include/BCP_functions.hpp \
	include/BCP_indexed_pricing.hpp \
	include/BCP_lp_branch.hpp \
	include/BCP_lp_functions.hpp \
	include/BCP_lp.hpp \
	include/BCP_lp_main_loop.hpp \
	include/BCP_lp_node.hpp \
	include/BCP_lp_param.hpp \
	include/BCP_lp_pool.hpp \
	include/BCP_lp_result.hpp \
	include/BCP_lp_user.hpp \
	include/BCP_main_fun.hpp \
	include/BCP_math.hpp \
	include/BCP_matrix.hpp \
	include/BCP_mempool.hpp \
	include/BCP_message.hpp \
	include/BCP_message_mpi.hpp \
	include/BCP_message_pvm.hpp \
	include/BCP_message_single.hpp \
	include/BCP_message_tag.hpp \
	include/BCP_node_change.hpp \
	include/BCP_obj_change.hpp \
	include/BCP_os.hpp \
	include/BCP_parameters.hpp \
	include/BCP_problem_core.hpp \
	include/BCP_process.hpp \
	include/BCP_set_intersects.hpp \
	include/BCP_solution.hpp \
	include/BCP_string.hpp \
	include/BCP_tm_functions.hpp \
	include/BCP_tm.hpp \
	include/BCP_tm_node.hpp \
	include/BCP_tm_param.hpp \
	include/BCP_tmstorage.hpp \
	include/BCP_tm_user.hpp \
	include/BCP_USER.hpp \
	include/BCP_var.hpp \
	include/BCP_vector_bool.hpp \
	include/BCP_vector_bool.hpp \
	include/BCP_vector_change.hpp \
	include/BCP_vector_char.hpp \
	include/BCP_vector_char.hpp \
	include/BCP_vector_double.hpp \
	include/BCP_vector_double.hpp \
	include/BCP_vector_general.hpp \
	include/BCP_vector.hpp \
	include/BCP_vector_int.hpp \
	include/BCP_vector_int.hpp \
	include/BCP_vector_sanity.hpp \
	include/BCP_vector_short.hpp \
	include/BCP_vector_short.hpp \
	include/BCP_vg.hpp \
	include/BCP_vg_param.hpp \
	include/BCP_vg_user.hpp \
	include/BCP_warmstart_basis.hpp \
	include/BCP_warmstart_dual.hpp \
	include/BCP_warmstart_primaldual.hpp \
	include/BCP_warmstart.hpp

#############################################################################
# Create the Config.h file that always defines HAVE_CONFIG_H and install it #
#############################################################################

# You only need to adapt the following line
ConfigHeader = BcpConfig.h

install-exec-local:
	echo "#ifndef HAVE_CONFIG_H" >bla
	echo "#define HAVE_CONFIG_H" >>bla
	echo "#endif" >> bla
	cat $(srcdir)/include/$(ConfigHeader) >> bla
	$(install_sh_DATA) bla $(DESTDIR)$(includedir)/$(ConfigHeader)
	rm -f bla

uninstall-local:
	rm -f $(DESTDIR)$(includedir)/$(ConfigHeader)
