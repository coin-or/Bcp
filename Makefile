USERROOT := $(shell pwd)

##############################################################################

ifeq ($(BCP_OPT),"")
    BCP_OPT := -O
endif

##############################################################################

ifeq ($(PROFILE),true)
    CXXFLAGS += -pg
endif

##############################################################################

ifeq ($(BCP_OPT),-O)
    ifeq ($(notdir $(CXX)),g++)
	BCPOPTFLAG = -O6
    else
	BCPOPTFLAG = -O3
    endif
else
    BCPOPTFLAG = -g
endif

ifeq ($(USER_OPT),-O)
    ifeq ($(notdir $(CXX)),g++)
	USEROPTFLAG = -O6
    else
	USEROPTFLAG = -O3
    endif
else
    USEROPTFLAG = -g
endif

###############################################################################

INCDIRS := $(IncDir)
LIBDIRS := $(LibDir)
LIBS    := $(LibName)
DEFINES := $(Define)

##############################################################################

DEFINES += PARANOID DO_TESTS BCP_STATISTICS 
INCDIRS += $(BCPROOT)/include $(USERROOT)/include

##############################################################################
##############################################################################

MSG_INCDIRS :=
PVM_LD_FLAGS :=
ifeq ($(COMM_PROTOCOL),"")
	COMM_PROTOCOL = NONE
endif
ifeq ($(COMM_PROTOCOL),PVM)
	INCDIRS += ${PVM_ROOT}/include
	LIBDIRS += ${PVM_ROOT}/lib/${PVM_ARCH} 
	LIBS    += libpvm3.so
endif

CXXFLAGS += -DBCP_COMM_PROTOCOL_$(COMM_PROTOCOL)

##############################################################################

INCDIRS += $(USER_INC_DIRS)
LIBDIRS += $(USER_LIB_DIRS)
LIBS    += $(USER_LIB_NAMES)
DEFINES += $(USER_DEFINES)

##############################################################################

LDFLAGS := $(addprefix -L,$(LIBDIRS))
LDFLAGS += $(if ${SHLINKPREFIX},$(addprefix ${SHLINKPREFIX},${LIBDIRS}),)
LDFLAGS += $(patsubst lib%,-l%,$(basename $(LIBS)))

##############################################################################
##############################################################################
# Paths
##############################################################################
##############################################################################
space = $(empty) $(empty)

BCPTARGETDIR = $(BCPROOT)/$(UNAME)$(subst $(space),_,$(BCP_OPT))
BCPDEPDIR    = $(BCPROOT)/dep
USERTARGETDIR = $(USERROOT)/$(UNAME)$(subst $(space),_,$(USER_OPT))
USERDEPDIR    = $(USERROOT)/dep

SRCDIR = \
	${BCPROOT}/Member     :${USERROOT}/Member    :\
	${BCPROOT}/OSL        :                      :\
	${BCPROOT}/TM         :${USERROOT}/TM        :\
	${BCPROOT}/LP         :${USERROOT}/LP        :\
	${BCPROOT}/CG         :${USERROOT}/CG        :\
	${BCPROOT}/VG         :${USERROOT}/VG        :\
	${BCPROOT}/CP         :${USERROOT}/CP        :\
	${BCPROOT}/VP         :${USERROOT}/VP        :\
	${BCPROOT}/include    :${USERROOT}/include   :\
	${BCPROOT}            :${USERROOT}           :\
	$(USER_SRC_PATH)

VPATH  = ${SRCDIR}

##############################################################################
##############################################################################
# Putting together DEF's, FLAGS
##############################################################################
##############################################################################

CXXFLAGS += $(USERFLAGS)

CXXFLAGS += $(addprefix -I,$(INCDIRS)) $(addprefix -D,$(DEFINES)) 

##############################################################################
##############################################################################
# Global source files
##############################################################################
##############################################################################

BCP_SRC  = 
BCP_INSTSRC  =

# Files containing member functions
#BCP_SRC +=	BCP_vector_change.cpp

BCP_SRC +=	BCP_lp_param.cpp
BCP_SRC +=	BCP_USER.cpp
BCP_SRC +=	BCP_timeout.cpp
BCP_SRC +=	BCP_node_change.cpp
BCP_SRC +=	BCP_warmstart_basis.cpp
BCP_SRC +=	BCP_warmstart_dual.cpp
BCP_SRC +=	BCP_solution.cpp
BCP_SRC +=	BCP_lp_pool.cpp
BCP_SRC +=	BCP_lp_user.cpp
BCP_SRC +=	BCP_vector_sanity.cpp
BCP_SRC +=	BCP_matrix.cpp
BCP_SRC +=	BCP_matrix_pack.cpp
BCP_SRC +=	BCP_problem_core.cpp
BCP_SRC +=	BCP_var.cpp
BCP_SRC +=	BCP_cut.cpp
BCP_SRC +=	BCP_obj_change.cpp
BCP_SRC +=	BCP_indexed_pricing.cpp
BCP_SRC +=	BCP_branch.cpp
BCP_SRC +=	BCP_lp_branch.cpp
BCP_SRC +=	BCP_lp_node.cpp
BCP_SRC +=	BCP_lp_result.cpp
BCP_SRC +=	BCP_lp.cpp
BCP_SRC +=	BCP_tm_node.cpp
BCP_SRC +=	BCP_tm_user.cpp
BCP_SRC +=	BCP_tm.cpp
BCP_SRC +=	BCP_tm_param.cpp
BCP_SRC +=	BCP_warmstart_pack.cpp
BCP_SRC +=	BCP_process.cpp
BCP_SRC +=	
# Files related to LP
BCP_SRC +=	BCP_lp_main.cpp
BCP_SRC +=	BCP_lp_main_loop.cpp
BCP_SRC +=	BCP_lp_create_lp.cpp
BCP_SRC +=	BCP_lp_convert_OsiWarmStart.cpp
BCP_SRC +=	BCP_lp_colrow.cpp
BCP_SRC +=	BCP_lp_generate_cuts.cpp
BCP_SRC +=	BCP_lp_generate_vars.cpp
BCP_SRC +=	BCP_lp_fathom.cpp
BCP_SRC +=	BCP_lp_branching.cpp
BCP_SRC +=	BCP_lp_msg_node_send.cpp
BCP_SRC +=	BCP_lp_msg_node_rec.cpp
BCP_SRC +=	BCP_lp_msgproc.cpp
BCP_SRC +=	BCP_lp_misc.cpp
BCP_SRC +=	
# Files related to CG
BCP_SRC +=	BCP_cg_main.cpp
BCP_SRC +=	BCP_cg.cpp
BCP_SRC +=	BCP_cg_param.cpp
BCP_SRC +=	BCP_cg_user.cpp
BCP_SRC +=	
# Files related to VG
BCP_SRC +=	BCP_vg_main.cpp
BCP_SRC +=	BCP_vg.cpp
BCP_SRC +=	BCP_vg_param.cpp
BCP_SRC +=	BCP_vg_user.cpp
BCP_SRC +=	
## Files related to TM
BCP_SRC +=	BCP_tm_main.cpp
BCP_SRC +=	BCP_tm_msgproc.cpp
BCP_SRC +=	BCP_tm_functions.cpp
BCP_SRC +=	BCP_tm_statistics.cpp
BCP_SRC +=	BCP_tm_trimming.cpp
BCP_SRC +=	BCP_tm_msg_node_send.cpp
BCP_SRC +=	BCP_tm_msg_node_rec.cpp
BCP_SRC +=	BCP_tm_commandline.cpp
BCP_SRC +=	

ifeq ($(COMM_PROTOCOL),PVM)
	BCP_SRC +=	BCP_message_pvm.cpp
endif
# BCP_message_single must be compiled into the code, no matter what
BCP_SRC +=	BCP_message_single.cpp


# ifeq ($(findstring COIN_USE_OSL, $(DETECTDEFINES)),COIN_USE_OSL)
#	BCP_SRC +=	oslSolver.cpp
#	BCP_SRC +=	oslvolume.cpp
#	BCP_INSTSRC  += osl_INST.cpp
# endif

# The explicit instantialization
BCP_INSTSRC +=	BCP_INST_vector_gen.cpp
BCP_INSTSRC +=	BCP_INST_vector_spec.cpp
BCP_INSTSRC +=	BCP_INST_vector_ptr.cpp
BCP_INSTSRC +=	BCP_INST_vector_ptr_purge.cpp
BCP_INSTSRC +=
BCP_INSTSRC +=	BCP_INST_system.cpp


##############################################################################

default : bcps

##############################################################################
##############################################################################
# Global rules
##############################################################################
##############################################################################

$(BCPTARGETDIR)/%.o : %.cpp ${BCPDEPDIR}/%.d
	@echo ""
	@echo Compiling $*.cpp
	@mkdir -p $(BCPTARGETDIR)
	@$(CXX) $(CXXFLAGS) $(BCPOPTFLAG) -c $< -o $@

${BCPDEPDIR}/%.d : %.cpp
	@echo ""
	@echo Creating dependency $*.d
	@mkdir -p ${BCPDEPDIR}
	@rm -f $*.d
	@set -e; $(DEPCXX) $(CXXFLAGS) $< \
	    | sed 's|\($(notdir $*)\)\.o[ :]*|$(BCPTARGETDIR)/\1.o $@ : |g' \
	    > $@; [ -s $@ ] || rm -f $@

$(USERTARGETDIR)/%.o : %.cpp ${USERDEPDIR}/%.d
	@echo ""
	@echo Compiling $*.cpp
	@mkdir -p $(USERTARGETDIR)
	@$(CXX) $(CXXFLAGS) $(USEROPTFLAG) -c $< -o $@

${USERDEPDIR}/%.d : %.cpp
	@echo ""
	@echo Creating dependency $*.d
	@mkdir -p ${USERDEPDIR}
	@rm -f $*.d
	@set -e; $(DEPCXX) $(CXXFLAGS) $< \
	    | sed 's|\($(notdir $*)\)\.o[ :]*|$(USERTARGETDIR)/\1.o $@ : |g' \
	    > $@; [ -s $@ ] || rm -f $@

##############################################################################
# The big code
##############################################################################

BCPOBJFILES  = $(addprefix $(BCPTARGETDIR)/, $(notdir $(BCP_SRC:.cpp=.o)))
USEROBJFILES = $(addprefix $(USERTARGETDIR)/, $(notdir $(USER_SRC:.cpp=.o)))
OBJFILES = $(BCPOBJFILES) $(USEROBJFILES)


BCPINSTOBJFILES  = \
	$(addprefix $(BCPTARGETDIR)/, $(notdir $(BCP_INSTSRC:.cpp=.o)))
USERINSTOBJFILES = \
	$(addprefix $(USERTARGETDIR)/, $(notdir $(USER_INSTSRC:.cpp=.o)))
INSTOBJFILES = $(BCPINSTOBJFILES) $(USERINSTOBJFILES)


ALLOBJFILES = $(OBJFILES) $(INSTOBJFILES) $(USER_OBJ)


BCPDEPFILES  = $(addprefix $(BCPDEPDIR)/, $(notdir $(BCPOBJFILES:.o=.d)))
USERDEPFILES = $(addprefix $(USERDEPDIR)/, $(notdir $(USEROBJFILES:.o=.d)))
DEPFILES = $(BCPDEPFILES) $(USERDEPFILES)


BCPINSTDEPFILES  = \
	$(addprefix $(BCPDEPDIR)/, $(notdir $(BCPINSTOBJFILES:.o=.d)))
USERINSTDEPFILES = \
	$(addprefix $(USERDEPDIR)/, $(notdir $(USERINSTOBJFILES:.o=.d)))
INSTDEPFILES = $(BCPINSTDEPFILES) $(USERINSTDEPFILES)

##############################################################################

.PHONY: doc default clean bcps ebcps bcpp ebcpp

##############################################################################

bcps : $(USERTARGETDIR)/bcps
bcpp : $(USERTARGETDIR)/bcpp
sbcps : $(USERTARGETDIR)/sbcps
sbcpp : $(USERTARGETDIR)/sbcpp

ebcps : $(USERTARGETDIR)/ebcps
ebcpp : $(USERTARGETDIR)/ebcpp
esbcps : $(USERTARGETDIR)/esbcps
esbcpp : $(USERTARGETDIR)/esbcpp

pbcps : $(USERTARGETDIR)/pbcps

$(USERTARGETDIR)/bcpp $(USERTARGETDIR)/bcps : $(ALLOBJFILES)
	@rm -rf Junk
	@echo ""
	@echo "Linking $(notdir $@) ..."
	@echo ""
	@mkdir -p $(USERTARGETDIR)
	@$(CXX) $(CXXFLAGS) -o $@ $(ALLOBJFILES) $(LDFLAGS) $(SYSLD) -lm

$(USERTARGETDIR)/sbcpp $(USERTARGETDIR)/sbcps : $(ALLOBJFILES)
	@rm -rf Junk
	@echo ""
	@echo "Linking $(notdir $@) ..."
	@echo ""
	@mkdir -p $(USERTARGETDIR)
	$(CXX) $(STATICSYSLD) $(CXXFLAGS) -o $@ $(ALLOBJFILES) $(LDFLAGS) -lm

$(USERTARGETDIR)/pbcps : $(ALLOBJFILES)
	@rm -rf Junk
	@echo ""
	@echo "Linking $(notdir $@) ..."
	@echo ""
	@mkdir -p $(USERTARGETDIR)
	@purify $(CXX) $(CXXFLAGS) -o $@ $(ALLOBJFILES) \
		$(LDFLAGS) $(SYSLD) -lm

$(USERTARGETDIR)/ebcpp $(USERTARGETDIR)/ebcps : $(ALLOBJFILES)
	@rm -rf Junk
	@echo ""
	@echo "Linking $(notdir $@) ..."
	@echo ""
	@mkdir -p $(USERTARGETDIR)
	@$(CXX) $(CXXFLAGS) -o $@ $(ALLOBJFILES) \
		$(LDFLAGS) $(SYSLD) $(EFENCE) -lm

$(USERTARGETDIR)/esbcpp $(USERTARGETDIR)/esbcps : $(ALLOBJFILES)
	@rm -rf Junk
	@echo ""
	@echo "Linking $(notdir $@) ..."
	@echo ""
	@mkdir -p $(USERTARGETDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(ALLOBJFILES) \
		$(LDFLAGS) $(STATICSYSLD) $(EFENCE) -lm

clean :
	rm -rf Junk
	rm -rf $(BCPDEPDIR) 
	rm -rf $(USERDEPDIR) 
	rm -rf $(BCPTARGETDIR) 
	rm -rf $(USERTARGETDIR)

doc:
	cd $(BCPROOT); doxygen $(MakefileDir)/doxygen.conf

###############################################################################

%::
	@mkdir -p Junk
	touch Junk/$(notdir $@)

##############################################################################
##############################################################################
# The configurator code
##############################################################################
##############################################################################

.PHONY: bcp_netconfig_pvm

bcp_netconfig_pvm: $(BCPROOT)/NetConfig/bcp_netconfig_pvm

$(BCPROOT)/NetConfig/bcp_netconfig_pvm: \
$(BCPROOT)/NetConfig/BCP_netconfig_pvm.cpp
	$(CXX) -g $(CXXFLAGS) ${PVM_ROOT}/include \
		-L${PVM_ROOT}/lib/${PVM_ARCH} -lpvm3 -o $@ $<

##############################################################################

.DELETE_ON_ERROR: ;

-include $(DEPFILES) 
-include $(INSTDEPFILES)


