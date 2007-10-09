// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>

#include "CoinTime.hpp"

#include "BCP_math.hpp"

#include "BCP_USER.hpp"

#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_vector.hpp"
#include "BCP_message_single.hpp"

#include "BCP_problem_core.hpp"
#include "BCP_node_change.hpp"

#include "BCP_tm_functions.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp.hpp"

#include "BCP_cg_user.hpp"
#include "BCP_cg.hpp"

#include "BCP_vg_user.hpp"
#include "BCP_vg.hpp"

#include "BCP_tm.hpp"

std::map<int, BCP_process*> BCP_single_environment::processes;

//#############################################################################

void
BCP_single_environment::set_arguments(const int argnum,
				      const char* const * args)
{
    int i;
    for (i = _argnum - 1; i >= 0; --i) {
	free(_arglist[i]);
    }
    _argnum = argnum;
    _arglist = new char*[argnum];
    for (i = _argnum - 1; i >= 0; --i) {
	_arglist[i] = strdup(args[i]);
    }
}

//#############################################################################

BCP_single_environment::~BCP_single_environment()
{
    for (int i = _argnum - 1; i >= 0; --i) {
	free(_arglist[i]);
    }
    delete[] _arglist;
}

//-----------------------------------------------------------------------------

int
BCP_single_environment::register_process(USER_initialize* user_init)
{
    // OK, the TM invoked registration. Now this function takes over and
    // becomes the driver for ALL `virtual' processes.
    //--------------------------------------------------------------------------
    // We start as it is in tm_main.cpp
    int _tm_id(0);
    int _lp_id(1);
#if ! defined(BCP_ONLY_LP_PROCESS_HANDLING_WORKS)
    int _cg_id(2);
    int _vg_id(3);
#endif
    //     int _cp_id(4);
    //     int _vp_id(5);
    BCP_tm_prob* _tm_prob = new BCP_tm_prob();
    processes[0] = _tm_prob;

    _tm_prob->par.set_entry(BCP_tm_par::MessagePassingIsSerial,true);
    _tm_prob->slave_pars.lp.set_entry(BCP_lp_par::MessagePassingIsSerial,true);
    _tm_prob->slave_pars.cg.set_entry(BCP_cg_par::MessagePassingIsSerial,true);
    _tm_prob->slave_pars.vg.set_entry(BCP_vg_par::MessagePassingIsSerial,true);
    /*
      _tm_prob->slave_pars.cp.set_entry(BCP_cp_par::MessagePassingIsSerial,true);
      _tm_prob->slave_pars.vp.set_entry(BCP_vp_par::MessagePassingIsSerial,true);
    */
    BCP_tm_parse_command_line(*_tm_prob, _argnum, _arglist);
    
    _tm_prob->msg_env = this;
    _tm_prob->start_time = CoinCpuTime();
    _my_id = _tm_id;

    // BCP_tm_user_init() returns a BCP_tm_user* and that will be part of p.
    // Also, it should take care of every I/O, heuristic startup, etc. the user
    // wants to do. Wreak havoc in p if (s)he wants.
    // BUT: once this function returns, the processes designated to be part of
    // BCP must be idle and waiting for a message. See the
    // BCP_slave_process_stub() function below. 
    _tm_prob->user = user_init->tm_init(*_tm_prob, _argnum, _arglist);
    _tm_prob->user->setTmProblemPointer(_tm_prob);
    _tm_prob->packer = user_init->packer_init(_tm_prob->user);
    _tm_prob->packer->user_class = _tm_prob->user;

    // Initialize the number of leaves assigned to CP's and VP's as 0
#if ! defined(BCP_ONLY_LP_PROCESS_HANDLING_WORKS)
    if (_tm_prob->param(BCP_tm_par::CpProcessNum) > 0) {
       _tm_prob->leaves_per_cp.reserve(_tm_prob->slaves.cp->procs().size());
       for (int i = _tm_prob->slaves.cp->procs().size() - 1; i >= 0; --i)
	    _tm_prob->leaves_per_cp.unchecked_push_back
		(std::make_pair(_tm_prob->slaves.cp->procs()[i], 0));
    }
    if (_tm_prob->param(BCP_tm_par::VpProcessNum) > 0) {
        _tm_prob->leaves_per_vp.reserve(_tm_prob->slaves.vp->procs().size());
	for (int i = _tm_prob->slaves.vp->procs().size() - 1; i >= 0; --i)
	    _tm_prob->leaves_per_vp.unchecked_push_back
		(std::make_pair(_tm_prob->slaves.vp->procs()[i], 0));
    }
#endif

    // Set the core (variables & cuts)
    _tm_prob->core = BCP_tm_create_core(*_tm_prob);
    _tm_prob->core_as_change = new BCP_problem_core_change;
    *_tm_prob->core_as_change = *_tm_prob->core;

    // Initialize the root of the search tree (can't invoke directly
    // p.user->create_root(), b/c the root might contain extra vars/cuts and
    // it's better if we take care of inserting them into the appropriate data
    // structures.
    BCP_tm_node* root = BCP_tm_create_root(*_tm_prob);

    _tm_prob->next_phase_nodes.push_back(root);
    _tm_prob->search_tree.insert(root);

    BCP_sanity_checks(*_tm_prob);

    // The TM is ready to roll

    //=========================================================================
    // Create the slave "processes"
    //=========================================================================
    // LP
    BCP_lp_prob* _lp_prob = new BCP_lp_prob(_lp_id, _tm_id);
    processes[_lp_id] = _lp_prob;
    _lp_prob->msg_env = new BCP_single_environment(_lp_id);
    _tm_prob->lp_procs.push_back(_lp_id);
    _tm_prob->lp_scheduler.add_free_ids(_tm_prob->lp_procs.begin(),
					_tm_prob->lp_procs.end());
    _tm_prob->lp_scheduler.
      setParams(_tm_prob->param(BCP_tm_par::LPscheduler_OverEstimationStatic),
		_tm_prob->param(BCP_tm_par::LPscheduler_SwitchToRateThreshold),
		10.0, /* estimated root time */
		_tm_prob->param(BCP_tm_par::LPscheduler_FactorTimeHorizon),
		_tm_prob->param(BCP_tm_par::LPscheduler_OverEstimationRate),
		_tm_prob->param(BCP_tm_par::LPscheduler_MaxNodeIdRatio),
		_tm_prob->param(BCP_tm_par::LPscheduler_MaxNodeIdNum));

    //-------------------------------------------------------------------------
    BCP_cg_prob* _cg_prob = 0;
    BCP_vg_prob* _vg_prob = 0;
//     BCP_cp_prob* _cp_prob = 0;
//     BCP_vp_prob* _vp_prob = 0;
    //-------------------------------------------------------------------------
    // CG
#if ! defined(BCP_ONLY_LP_PROCESS_HANDLING_WORKS)
    if (_tm_prob->param(BCP_tm_par::CgProcessNum) > 0) {
	_cg_prob = new BCP_cg_prob(_cg_id, _tm_id);
	processes[_cg_id] = _cg_prob;
	_cg_prob->msg_env = new BCP_single_environment(_cg_id);
	_tm_prob->slaves.cg = new BCP_proc_array;
	_tm_prob->slaves.cg->add_proc(_cg_id);
	_tm_prob->slaves.all->add_proc(_cg_id);
    }
    //-------------------------------------------------------------------------
    // VG
    if (_tm_prob->param(BCP_tm_par::VgProcessNum) > 0) {
	_vg_prob = new BCP_vg_prob(_vg_id, _tm_id);
	processes[_vg_id] = _vg_prob;
	_vg_prob->msg_env = new BCP_single_environment(_vg_id);
	_tm_prob->slaves.vg = new BCP_proc_array;
	_tm_prob->slaves.vg->add_proc(_vg_id);
	_tm_prob->slaves.all->add_proc(_vg_id);
    }
    //-------------------------------------------------------------------------
    // CP
    if (_tm_prob->param(BCP_tm_par::CpProcessNum) > 0) {
	_cp_prob = new BCP_cp_prob(_cp_id, _tm_id);
	processes[_cp_id] = _cp_prob;
	_cp_prob->msg_env = new BCP_single_environment(_cp_id);
	_tm_prob->slaves.cp = new BCP_proc_array;
	_tm_prob->slaves.cp->add_proc(_cp_id);
	_tm_prob->slaves.all->add_proc(_cp_id);
    }
    //-------------------------------------------------------------------------
    // VP
    if (_tm_prob->param(BCP_tm_par::VpProcessNum) > 0) {
	_vp_prob = new BCP_vp_prob(_vp_id, _tm_id);
	processes[_vp_id] = _vp_prob;
	_vp_prob->msg_env = new BCP_single_environment(_vp_id);
	_tm_prob->slaves.vp = new BCP_proc_array;
	_tm_prob->slaves.vp->add_proc(_vp_id);
	_tm_prob->slaves.all->add_proc(_vp_id);
    }
#endif

    //=========================================================================
    // distribute the core to the slave processes
    // LP
    _tm_prob->msg_buf.clear();
    _tm_prob->core->pack(_tm_prob->msg_buf);
    BCP_lp_process_core(*_lp_prob, _tm_prob->msg_buf);

    // CG
    if (_cg_prob) {
	_tm_prob->msg_buf.clear();
	_tm_prob->core->pack(_tm_prob->msg_buf);
	_cg_prob->core->unpack(_tm_prob->msg_buf);
    }
    // VG
    if (_vg_prob) {
	_tm_prob->msg_buf.clear();
	_tm_prob->core->pack(_tm_prob->msg_buf);
	_vg_prob->core->unpack(_tm_prob->msg_buf);
    }
    // CP
    // VP

    //=========================================================================
    // Initialize the slave "processes"
    //=========================================================================
    // LP (it already has the core)
    // copy over the parameter structure
    _lp_prob->par = _tm_prob->slave_pars.lp;
    // create the user universe
    _lp_prob->user = user_init->lp_init(*_lp_prob);
    _lp_prob->user->setLpProblemPointer(_lp_prob);
    _lp_prob->packer = user_init->packer_init(_lp_prob->user);
    _lp_prob->packer->user_class = _lp_prob->user;

    // copy over the user info
    _tm_prob->msg_buf.clear();
    _tm_prob->user->pack_module_data(_tm_prob->msg_buf, BCP_ProcessType_LP);
    _lp_prob->user->unpack_module_data(_tm_prob->msg_buf);
    _lp_prob->master_lp = _lp_prob->user->initialize_solver_interface();

    _lp_prob->upper_bound = std::min<double>(_tm_prob->ub(), BCP_DBL_MAX);
    //-------------------------------------------------------------------------
    if (_cg_prob) {
	// CG (it already has the core)
	// copy over the parameter structure
	_cg_prob->par = _tm_prob->slave_pars.cg;
	// create the user universe
	_cg_prob->user = user_init->cg_init(*_cg_prob);
	_cg_prob->user->setCgProblemPointer(_cg_prob);
	_cg_prob->packer = user_init->packer_init(_cg_prob->user);
	_cg_prob->packer->user_class = _cg_prob->user;
	// copy over the user info
	_tm_prob->msg_buf.clear();
	_tm_prob->user->pack_module_data(_tm_prob->msg_buf,
					 BCP_ProcessType_CG);
	if (_cg_prob->user)
	    _cg_prob->user->unpack_module_data(_tm_prob->msg_buf);
    }
    //-------------------------------------------------------------------------
    // VG
    if (_vg_prob) {
	// VG (it already has the core)
	// copy over the parameter structure
	_vg_prob->par = _tm_prob->slave_pars.vg;
	// create the user universe
	_vg_prob->user = user_init->vg_init(*_vg_prob);
	_vg_prob->user->setVgProblemPointer(_vg_prob);
	_vg_prob->packer = user_init->packer_init(_vg_prob->user);
	_vg_prob->packer->user_class = _vg_prob->user;
	// copy over the user info
	_tm_prob->msg_buf.clear();
	_tm_prob->user->pack_module_data(_tm_prob->msg_buf,
					 BCP_ProcessType_VG);
	if (_vg_prob->user)
	    _vg_prob->user->unpack_module_data(_tm_prob->msg_buf);
    }
    //-------------------------------------------------------------------------
    // CP
    // VP

    //=========================================================================
    // Back to TM
    bool something_died = false;
    try {
	for ( _tm_prob->phase = 0; true ; ++_tm_prob->phase) {
	    // insert the nodes in next_phase_nodes into candidates, print out
	    // some statistics about the previous phase (if there was one) and
	    // do some other stuff, too.
	    BCP_tm_tasks_before_new_phase(*_tm_prob);
	    // do one phase 
	    // While there are nodes waiting to be processed (or being
	    // processed) we don't go to the next phase
	    something_died = false;
	    while (! _tm_prob->candidate_list.empty() ||
		   _tm_prob->lp_scheduler.numNodeIds() > 0){
		// Fill up as many free LP processes as we can
		if (BCP_tm_start_new_nodes(*_tm_prob) == BCP_NodeStart_Error) {
		    // Error indicates that something has died
		    something_died = true;
		    break;
		}
		// No need to process messages, they have all been processed
		// (single environment!) Also all the "processes" are alive.
	    }
	    // If nothing is left for the next phase or if something has died
	    // then quit the infinite loop.
	    if (_tm_prob->next_phase_nodes.size() == 0 || something_died)
		break;
	}
    }
    catch (BCP_fatal_error& err) {
	// there can be only one, a timeout
    }

    // everything is done
    BCP_tm_wrapup(_tm_prob, _lp_prob, _cg_prob, _vg_prob, true);
    // sleep(7200);

    delete _lp_prob;

    if (_cg_prob) {
	delete _cg_prob;
    }
    if (_vg_prob) {
	delete _vg_prob;
    }
      
    //    if (_cp_prob)
    //       delete _cp_prob;
    //    if (_vp_prob)
    //       delete _vp_prob;
      
    delete _tm_prob;

    exit(0);

    return 0; //fake, just to quiet the compiler
}

//-----------------------------------------------------------------------------

int
BCP_single_environment::parent_process() {
    throw BCP_fatal_error("\
BCP_single_environment::parent_process() invoked.\n");
    return 0; // to satisfy aCC on HP-UX
}

bool
BCP_single_environment::alive(const int pid)
{
    return true;
}

const int* 
BCP_single_environment::alive(int num, const int* pids)
{
  return NULL;
}

//-----------------------------------------------------------------------------

void
BCP_single_environment::send(const int target,
			     const BCP_message_tag tag)
{
    BCP_process* target_process = processes[target];
    BCP_buffer& target_buf = target_process->get_message_buffer();
    target_buf.clear();
    target_buf._sender = _my_id;
    target_buf._msgtag = tag;
    target_process->process_message();
}

void
BCP_single_environment::send(const int target,
			     const BCP_message_tag tag,
			     const BCP_buffer& buf)
{
    BCP_process* target_process = processes[target];
    BCP_buffer& target_buf = target_process->get_message_buffer();
    target_buf = buf;
    target_buf._sender = _my_id;
    target_buf._msgtag = tag;
    target_process->process_message();
}
   
//-----------------------------------------------------------------------------

void
BCP_single_environment::multicast(int num, const int* targets,
				  const BCP_message_tag tag)
{
  for (int i = 0; i < num; ++i) {
    send(targets[i], tag);
  }
}

void
BCP_single_environment::multicast(int num, const int* targets,
				  const BCP_message_tag tag,
				  const BCP_buffer& buf)
{
  for (int i = 0; i < num; ++i) {
    send(targets[i], tag, buf);
  }
}

//-----------------------------------------------------------------------------

void
BCP_single_environment::receive(const int source,
				const BCP_message_tag tag,
				BCP_buffer& buf, const double timeout)
{
    // In the single_environment this is not really needed. Report if it's
    // called with anything but:
    //   BCP_UpperBound, BCP_Msg_DivingInfo or BCP_Msg_PricedRoot.
    if (tag != BCP_Msg_UpperBound &&
	tag != BCP_Msg_DivingInfo)
	printf("BCP_single_environment::receive() is called with %i as tag.\n",
	       tag);
}

//-----------------------------------------------------------------------------

bool
BCP_single_environment::probe(const int source,
			      const BCP_message_tag tag)
{
    // In the single_environment this is not needed.
    return false;
}

//-----------------------------------------------------------------------------
// These should never be invoked in a BCP_single_environment

int
BCP_single_environment::start_process(const BCP_string& exe,
				      const bool debug)
{
    throw BCP_fatal_error("start_process() called!\n");
    return 0; // to satisfy aCC on HP-UX
}

int
BCP_single_environment::start_process(const BCP_string& exe,
				      const BCP_string& machine,
				      const bool debug)
{
    throw BCP_fatal_error("start_process() called!\n");
    return 0; // to satisfy aCC on HP-UX
}

bool
BCP_single_environment::start_processes(const BCP_string& exe,
					const int proc_num,
					const bool debug,
					int* ids)
{
    throw BCP_fatal_error("start_processes() called!\n");
    return false; // to satisfy aCC on HP-UX
}

bool
BCP_single_environment::start_processes(const BCP_string& exe,
					const int proc_num,
					const BCP_vec<BCP_string>& machines,
					const bool debug,
					int* ids)
{
    throw BCP_fatal_error("start_processes() called!\n");
    return false; // to satisfy aCC on HP-UX
}

//-----------------------------------------------------------------------------

// void BCP_single_environment::stop_process(const int process) {
//    check_error( pvm_kill(BCP_is_single_id(process, "stop_process()")),
// 		"stop_process()");
// }

// void BCP_single_environment::stop_processes(const BCP_proc_array* processes){
//    BCP_vec<int>::const_iterator first = processes.procs.begin();
//    BCP_vec<int>::const_iterator last = pprocesses.procs.end();
//    while (first != last)
//       check_error( pvm_kill(BCP_is_single_id(*first, "stop_processes()")),
// 		   "stop_processes()");
//       ++first;
//    }
// }
