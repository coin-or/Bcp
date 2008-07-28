// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <queue>
#ifdef _MSC_VER
#include <process.h>
#endif 

#include "CoinTime.hpp"

#include "BcpConfig.h"
#include "BCP_os.hpp"

#include "BCP_USER.hpp"
#include "BCP_string.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_node_change.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_functions.hpp"
#include "BCP_main_fun.hpp"

#include "BCP_tm_user.hpp"
#include "BCP_lp_user.hpp"

#include "BCP_message_single.hpp"
#include "BCP_message_mpi.hpp"
#include "BCP_message_pvm.hpp"

//#############################################################################

int bcp_main(int argc, char* argv[], USER_initialize* user_init)
{
    BCP_message_environment* msg_env = user_init->msgenv_init(argc, argv);
    BCP_single_environment* single_env =
	dynamic_cast<BCP_single_environment*>(msg_env);
    if (single_env) {
	// this way when register_process takes over the execution the single
	// environment will have access to the command line arguments and can
	// parse it.
	single_env->set_arguments(argc, argv);
    }

    int my_id = msg_env->register_process(user_init);

    if (single_env) {
	// register process did everything. We just return.
	delete msg_env;
	return 0;
    }

    int parent = msg_env->parent_process();

    if (parent == -1) {
	//We compute the real numeber of arguments because Mpi can change
	//list of arguments
	int cnt;
	for (cnt = 0; cnt < argc; ++cnt) {
	    if (argv[cnt] == NULL)
		break;
	}
	BCP_tm_main(msg_env, user_init, cnt, argv);
    } else {
	// In MPI all processes get the argument list, so we must not check
	// this
#if defined(COIN_HAS_MPI)
	BCP_mpi_environment* mpi_env =
	    dynamic_cast<BCP_mpi_environment*>(msg_env);
	if (!mpi_env && argc != 1) {
	    throw BCP_fatal_error("The slaves do not take any argument!\n");
	}
#endif
	BCP_buffer msg_buf;
	msg_env->receive(parent, BCP_Msg_AnyMessage, msg_buf, -1);
	if (msg_buf.msgtag() != BCP_Msg_ProcessType) {
	    throw BCP_fatal_error("The first message is not ProcessType!!!\n");
	}
	// got a new identity, act on it
	BCP_process_t ptype;
	double ub;
	msg_buf.unpack(ptype);
	msg_buf.unpack(ub);
	while (ptype != BCP_ProcessType_EndProcess) {
	    const bool maxheap_set = false;
	    switch (ptype) {
	    case BCP_ProcessType_LP:
	      if (maxheap_set) {
		 printf("usedheap before LP: %li\n", BCP_used_heap());
	      }
	      ptype = BCP_lp_main(msg_env, user_init, my_id, parent, ub);
	      if (maxheap_set) {
		  printf("usedheap after LP: %li\n", BCP_used_heap());
	      }
	      break;
	    case BCP_ProcessType_CP:
	      // BCP_cp_main(msg_env, user_init, my_id, parent, ub);
	      break;
	    case BCP_ProcessType_VP:
	      // BCP_vp_main(msg_env, user_init, my_id, parent, ub);
	      break;
	    case BCP_ProcessType_CG:
	      ptype = BCP_cg_main(msg_env, user_init, my_id, parent, ub);
	      break;
	    case BCP_ProcessType_VG:
	      ptype = BCP_vg_main(msg_env, user_init, my_id, parent, ub);
	      break;
	    case BCP_ProcessType_TS:
	      if (maxheap_set) {
		  printf("usedheap before TS: %li\n", BCP_used_heap());
	      }
	      ptype = BCP_tmstorage_main(msg_env, user_init, my_id, parent, ub);
	      if (maxheap_set) {
		  printf("usedheap after TS: %li\n", BCP_used_heap());
	      }
	      break;
	    case BCP_ProcessType_Any:
	      throw BCP_fatal_error("\
New process identity is BCP_ProcessType_Any!\n");
	    case BCP_ProcessType_TM:
	      throw BCP_fatal_error("\
New process identity is BCP_ProcessType_TM!\n");
	    case BCP_ProcessType_EndProcess:
	      break;
	    }
	}
    }
    delete msg_env;

    return 0;
}

//#############################################################################

void
BCP_tm_main(BCP_message_environment* msg_env,
	    USER_initialize* user_init,
	    const int argnum, const char* const * arglist)
{
    // If we ever get here then the environment is parallel
    
    // Start to create the universe... (we don't have a user universe yet).
    BCP_tm_prob p;

    // If we ever get here then the environment is parallel
    p.par.set_entry(BCP_tm_par::MessagePassingIsSerial,false);
    p.slave_pars.lp.set_entry(BCP_lp_par::MessagePassingIsSerial,false);
    p.slave_pars.cg.set_entry(BCP_cg_par::MessagePassingIsSerial,false);
    p.slave_pars.vg.set_entry(BCP_vg_par::MessagePassingIsSerial,false);
    /*
      p.slave_pars.cp.set_entry(BCP_cp_par::MessagePassingIsSerial,false);
      p.slave_pars.vp.set_entry(BCP_vp_par::MessagePassingIsSerial,false);
    */

    // this also reads in the parameters from a file
    BCP_tm_parse_command_line(p, argnum, arglist);
   
    BCP_buffer msg_buf;
    p.msg_env = msg_env;

    //We check if the number of BCP processes is the same as in MPI
#if defined(COIN_HAS_MPI)
    BCP_mpi_environment* mpi_env = dynamic_cast<BCP_mpi_environment*>(msg_env);
    if (mpi_env) {
	const int n_proc =
	    p.param(BCP_tm_par::LpProcessNum) +
	    p.param(BCP_tm_par::CgProcessNum) +
	    p.param(BCP_tm_par::VgProcessNum) +
	    p.param(BCP_tm_par::CpProcessNum) +
	    p.param(BCP_tm_par::VpProcessNum) + 1;
	if (p.msg_env->num_procs() < n_proc) {
	    throw BCP_fatal_error("\
Number of process in parameter file %d > n_proc in mpirun -np %d!\n",
				  n_proc, p.msg_env->num_procs());
	}
    }
#endif

    p.stat.set_num_lp(p.param(BCP_tm_par::LpProcessNum));

    p.start_time = CoinWallclockTime();

    FILE* logfile = 0;

    const BCP_string& log = p.par.entry(BCP_tm_par::LogFileName);
    if (! (p.par.entry(BCP_tm_par::LogFileName) == "")) {
	int len = log.length();
	char *logname = new char[len + 300];
	memcpy(logname, log.c_str(), len);
	memcpy(logname + len, "-tm-", 4);
	len += 4;
	gethostname(logname + len, 255);
	len = strlen(logname);
	logname[len++] = '-';
	sprintf(logname + len, "%i", static_cast<int>(GETPID));
	logfile = freopen(logname, "a", stdout);
	if (logfile == 0) {
	    fprintf(stderr, "Error while redirecting stdout: %i\n", errno);
	    abort();
	}
	setvbuf(logfile, NULL, _IOLBF, 0); // make it line buffered
	delete[] logname;
    } else {
	setvbuf(stdout, NULL, _IOLBF, 0); // make it line buffered
    }

    // BCP_tm_user_init() returns a BCP_tm_user* and that will be part of p.
    // Also, it should take care of every I/O, heuristic startup, etc. the user
    // wants to do. Wreak havoc in p if (s)he wants.
    // BUT: once this function returns, the processes designated to be part of
    // BCP must be idle and waiting for a message. See the
    // BCP_slave_process_stub() function below. 
    p.user = user_init->tm_init(p, argnum, arglist);
    p.user->setTmProblemPointer(&p);
    p.packer = user_init->packer_init(p.user);
    p.packer->user_class = p.user;

    // Set the core (variables & cuts)
    p.core = BCP_tm_create_core(p);
    p.core_as_change = new BCP_problem_core_change;
    *p.core_as_change = *p.core;

    // Fire up the LP/CG/CP/VG/VP processes
    // Actually, this is firing up enough copies of self.
    BCP_tm_start_processes(p);
    p.lp_scheduler.
      setParams(p.param(BCP_tm_par::LPscheduler_OverEstimationStatic),
		p.param(BCP_tm_par::LPscheduler_SwitchToRateThreshold),
		10.0, /* estimated root time */
		p.param(BCP_tm_par::LPscheduler_FactorTimeHorizon),
		p.param(BCP_tm_par::LPscheduler_OverEstimationRate),
		p.param(BCP_tm_par::LPscheduler_MaxNodeIdRatio),
		p.param(BCP_tm_par::LPscheduler_MaxNodeIdNum),
		p.param(BCP_tm_par::LPscheduler_MaxSbIdNum),
		p.param(BCP_tm_par::LPscheduler_MinSbIdNum));

    // Notify the LP/CG/CP/VG/VP processes about their identity. Also, send out
    // their parameters, core and user info.
    BCP_tm_notify_processes(p);

#if ! defined(BCP_ONLY_LP_PROCESS_HANDLING_WORKS)
    // Initialize the number of leaves assigned to CP's and VP's as 0
    if (p.param(BCP_tm_par::CpProcessNum) > 0) {
      for (int i = p.slaves.cp->procs().size() - 1; i >= 0; --i)
	p.leaves_per_cp.push_back(std::make_pair(p.slaves.cp->procs()[i], 0));
    }
    if (p.param(BCP_tm_par::VpProcessNum) > 0) {
      for (int i = p.slaves.vp->procs().size() - 1; i >= 0; --i)
	p.leaves_per_vp.push_back(std::make_pair(p.slaves.vp->procs()[i], 0));
    }
#endif

    // Initialize the root of the search tree (can't invoke directly
    // p.user->create_root(), b/c the root might contain extra vars/cuts and
    // it's better if we take care of inserting them into the appropriate data
    // structures.
    BCP_tm_node* root = BCP_tm_create_root(p);

    p.next_phase_nodes.push_back(root);
    p.search_tree.insert(root);

    BCP_sanity_checks(p);

    //-------------------------------------------------------------------------
    // The main loop
    //-------------------------------------------------------------------------
    bool something_died = false;
    for ( p.phase = 0; true ; ++p.phase) {
	// insert the nodes in next_phase_nodes into candidates, print out some
	// statistics about the previous phase (if there was one) and do some
	// other stuff, too.
	BCP_tm_tasks_before_new_phase(p);
	// do one phase (return true/false depending on success)
	something_died = BCP_tm_do_one_phase(p);
	// If nothing is left for the next phase or if something has died then
	// quit the infinite loop.
	if (p.next_phase_nodes.size() == 0 || something_died)
	    break;
    }

    //-------------------------------------------------------------------------
    // Everything is done
    //-------------------------------------------------------------------------
    // first let the processes know that they're not needed in BCP any more,
    // they can start idling (this will be used when we'll loop around in TM).
    // The processes will respond by sending statistics.
    p.lp_scheduler.update_idle_times();
    BCP_tm_idle_processes(p);

    BCP_tm_wrapup(&p, 0, 0, 0, true);

    // Finally stop all the processes.
    BCP_tm_stop_processes(p);

    if (logfile)
	fclose(logfile);
}

//#############################################################################

bool BCP_tm_do_one_phase(BCP_tm_prob& p)
{
    BCP_buffer& buf = p.msg_buf;
    // While there are nodes waiting to be processed (or being processed) we
    // don't go to the next phase
    while (!p.candidate_list.empty() > 0 || p.lp_scheduler.numNodeIds() > 0){
	// Fill up as many free LP processes as we can
	if (BCP_tm_start_new_nodes(p) == BCP_NodeStart_Error)
	    // Error indicates that something has died
	    return true;
	buf.clear();
	// Check if need to balance data
	p.need_a_TS = ! BCP_tm_is_data_balanced(p);
	p.need_a_TS = BCP_tm_balance_data(p);

	// Process incoming messages. If there are no active nodes left then
	// timeout is set to 0, so we just check the queue, but not wait.
	const int numNodeIds = p.lp_scheduler.numNodeIds();
	const double t0 = CoinWallclockTime();
	const double timeout = (p.lp_scheduler.numNodeIds() == 0 ?
				0 : p.param(BCP_tm_par::TmTimeout));
	p.msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage, buf, timeout);
	const double t1 = CoinWallclockTime();
	p.stat.update_wait_time(numNodeIds, t1-t0);
#ifdef COIN_HAS_MPI
	p.stat.update_queue_length(numNodeIds, MPIDI_BGLTS_get_num_messages());
#endif
	p.stat.print(false /* not final */, t1 - p.start_time);
	try {
	    p.process_message();
	}
	catch (BCP_fatal_error& err) {
	    // something is baaaad... e.g. timeout
	    return true;
	}
    }
    return false;
}

//#############################################################################

BCP_problem_core* BCP_tm_create_core(BCP_tm_prob& p)
{
    BCP_vec<BCP_var_core*> bvars;
    BCP_vec<BCP_cut_core*> bcuts;
    BCP_lp_relax* matrix = 0;

    p.user->initialize_core(bvars, bcuts, matrix);

    const int bvarnum = bvars.size();
    if (bvarnum > 0) {
	int i = p.next_var_index_set_start;
	for (i = 0; i < bvarnum; ++i) {
	    BCP_var_core* var = bvars[i];
	    // make certain that the bounds of integer vars is integer...
	    if (var->var_type() != BCP_ContinuousVar) {
		var->set_lb(ceil(var->lb()-1e-8));
		var->set_ub(floor(var->ub()+1e-8));
	    }
	    var->set_bcpind(i);
	    p.vars_local[i] = new BCP_var_core(*var);
	}
	p.next_var_index_set_start = i;
    }

    const int bcutnum = bcuts.size();
    if (bcutnum > 0) {
	int i = p.next_cut_index_set_start;
	for (i = 0; i < bcutnum; ++i) {
	    BCP_cut_core* cut = bcuts[i];
	    cut->set_bcpind(i);
	    p.cuts_local[i] = new BCP_cut_core(*cut);
	}
	p.next_cut_index_set_start = i;
    }

    if (!matrix)
	matrix = new BCP_lp_relax;

    return new BCP_problem_core(bvars, bcuts, matrix);
}
   
//#############################################################################

static inline BCP_cut*
BCP_tm_unpack_root_cut(BCP_tm_prob& tm)
{
    BCP_cut* cut;
    BCP_buffer& buf = tm.msg_buf;
    BCP_object_t obj_t;
    double lb, ub;
    BCP_obj_status stat;
    buf.unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);
    switch (obj_t) {
    case BCP_CoreObj:
	cut = new BCP_cut_core(lb, ub);
	break;
    case BCP_AlgoObj:
	cut = tm.packer->unpack_cut_algo(buf);
	cut->change_bounds(lb, ub);
	break;
    default:
	throw BCP_fatal_error("BCP_tm_unpack_root_cut: unexpected obj_t.\n");
    }
    cut->set_bcpind(0);
    cut->set_status(stat);

    return cut;
}

//#############################################################################

BCP_tm_node* BCP_tm_create_root(BCP_tm_prob& p)
{
    BCP_vec<BCP_var*> added_vars;
    BCP_vec<BCP_cut*> added_cuts;
    BCP_user_data* user_data = 0;

    // If the root cuts are saved then read them in
    const BCP_string& cutfile = p.param(BCP_tm_par::ReadRootCutsFrom);
    if (cutfile.length() > 0) {
	BCP_buffer& buf = p.msg_buf;
	buf.clear();
	FILE* f = fopen(cutfile.c_str(), "r");
	size_t size;
	if (fread(&size, 1, sizeof(size), f) != sizeof(size))
	    throw BCP_fatal_error("ReadRootCutsFrom read error.\n");
	char * data = new char[size];
	if (fread(data, 1, size, f) != size)
	    throw BCP_fatal_error("ReadRootCutsFrom read error.\n");
	fclose(f);
	buf.set_content(data, size, 0, BCP_Msg_NoMessage);
	delete[] data;
      
	int num;
	buf.unpack(num);
	added_cuts.reserve(added_cuts.size() + num);
	for (int i = 0; i < num; ++i) {
	    added_cuts.unchecked_push_back(BCP_tm_unpack_root_cut(p));
	}
    }

    p.user->create_root(added_vars, added_cuts, user_data);

    BCP_node_change* root_changes = new BCP_node_change;
    root_changes->core_change._storage = BCP_Storage_WrtCore;

    if (added_vars.size() > 0) {
	const int num = added_vars.size();
	BCP_obj_set_change& vc = root_changes->var_change;
	vc._change.reserve(num);
	vc._new_objs.reserve(num);
	int ind = p.next_var_index_set_start;
	for (int i = 0; i < num; ++i) {
	    BCP_var* var = added_vars[i];
	    vc._new_objs.unchecked_push_back(ind);
	    p.vars_local[ind] = var;
	    var->set_bcpind(ind++);
	    vc._change.unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
							  var->status()));
	}
	p.next_var_index_set_start = ind;
    }

    if (added_cuts.size() > 0) {
	const int num = added_cuts.size();
	BCP_obj_set_change& cc = root_changes->cut_change;
	cc._change.reserve(num);
	cc._new_objs.reserve(num);
	int ind = p.next_cut_index_set_start;
	for (int i = 0; i < num; ++i) {
	    BCP_cut* cut = added_cuts[i];
	    cc._new_objs.unchecked_push_back(ind);
	    p.cuts_local[ind] = cut;
	    cut->set_bcpind(ind++);
	    cc._change.unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
							  cut->status()));
	}
	p.next_cut_index_set_start = ind;
    }

    BCP_tm_node* root = new BCP_tm_node(0, root_changes);

    root->_data._user = user_data;

    root->_core_storage = root->_data._desc->core_change.storage();
    root->_var_storage = BCP_Storage_Explicit;
    root->_cut_storage = BCP_Storage_Explicit;
    root->_ws_storage = BCP_Storage_NoData;

    return root;
}

//#############################################################################

void BCP_tm_tasks_before_new_phase(BCP_tm_prob& p)
{
    if (p.param(BCP_tm_par::TmVerb_NewPhaseStart)) {
	printf("##########################################################\n");
	printf("TM: Starting phase %i\n", p.phase);
	printf("##########################################################\n");
    }

    if (p.phase > 0) {
	// Notify the LPs about the start of the new phase and get back the
	// timing data for the previous phase
	BCP_tm_notify_about_new_phase(p);

	// print statistics about the previous phase
	BCP_tm_wrapup(&p, 0, 0, 0, false); // false refers to not being final

	// trim the tree if needed and possible
	if (p.param(BCP_tm_par::TrimTreeBeforeNewPhase) && p.has_ub())
	    BCP_tm_trim_tree_wrapper(p, true /* called between phases */);
    }

    if (p.candidate_list.getTree() && !p.candidate_list.empty()) {
	throw BCP_fatal_error("\
BCP_tm_tasks_before_new_phase: candidate_list should be empty!\n");
    }

    // build up candidates. initialize the candidate list comparison function.
    p.current_phase_colgen = BCP_DoNotGenerateColumns_Fathom;
    p.candidate_list.setTree(NULL);
    CoinSearchTreeBase* candidates = NULL;
    p.user->init_new_phase(p.phase, p.current_phase_colgen, candidates);
    if (candidates == NULL) {
	candidates = new CoinSearchTree<CoinSearchTreeCompareBest>;
    }
    p.candidate_list.setTree(candidates);
   
    for (int i = p.next_phase_nodes.size() - 1; i >= 0; --i) {
	p.candidate_list.push(p.next_phase_nodes[i]);
    }

    p.next_phase_nodes.clear();
}

//#############################################################################


