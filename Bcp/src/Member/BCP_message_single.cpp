// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>

#include "CoinTime.hpp"

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

std::map<BCP_single_id, BCP_process*> BCP_single_environment::processes;

//#############################################################################

BCP_process::~BCP_process()
{
    delete my_id;
    delete parent_id;
}
    
//#############################################################################

BCP_single_id*
BCP_is_single_id(const BCP_proc_id* pid, const char* str)
{
    BCP_single_id* id =
	dynamic_cast<BCP_single_id*>(const_cast<BCP_proc_id*>(pid));
    if (id == 0)
	throw BCP_fatal_error("Non-`single_id' used in `single_environment.\n");
    return id;
}

//#############################################################################

bool
BCP_single_id::is_same_process(const BCP_proc_id* other_process) const
{
    return pid() == BCP_is_single_id(other_process, "is_same_process()")->pid();
}

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

BCP_proc_id*
BCP_single_environment::register_process(USER_initialize* user_init)
{
    // OK, the TM invoked registration. Now this function takes over and
    // becomes the driver for ALL `virtual' processes.
    //--------------------------------------------------------------------------
    // We start as it is in tm_main.cpp
    BCP_single_id _tm_id(0);
    BCP_single_id _lp_id(1);
    BCP_single_id _cg_id(2);
    BCP_single_id _vg_id(3);
    //     BCP_single_id _cp_id(4);
    //     BCP_single_id _vp_id(5);
    BCP_single_id* id = _tm_id.clone();
    BCP_tm_prob* _tm_prob = new BCP_tm_prob(id, NULL);
    processes[*id] = _tm_prob;

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

    // Initialize the number of leaves assigned to CP's and VP's as 0
    if (_tm_prob->param(BCP_tm_par::CpProcessNum) > 0) {
	_tm_prob->leaves_per_cp.reserve(_tm_prob->slaves.cp->size());
	for (int i = _tm_prob->slaves.cp->size() - 1; i >= 0; --i)
	    _tm_prob->leaves_per_cp.unchecked_push_back
		(std::make_pair(_tm_prob->slaves.cp->procs()[i]->clone(), 0));
    }
    if (_tm_prob->param(BCP_tm_par::VpProcessNum) > 0) {
	_tm_prob->leaves_per_vp.reserve(_tm_prob->slaves.vp->size());
	for (int i = _tm_prob->slaves.vp->size() - 1; i >= 0; --i)
	    _tm_prob->leaves_per_vp.unchecked_push_back
		(std::make_pair(_tm_prob->slaves.vp->procs()[i]->clone(), 0));
    }

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
    // Create the slave "processes" (also fill up _tm_prob->slaves)
    //=========================================================================
    // All processes
    BCP_vec<BCP_proc_id*> allprocs;
    allprocs.push_back(_lp_id.clone());
    if (_tm_prob->param(BCP_tm_par::CgProcessNum) > 0)
	allprocs.push_back(_cg_id.clone());
    if (_tm_prob->param(BCP_tm_par::VgProcessNum) > 0)
	allprocs.push_back(_vg_id.clone());
    //    if (_tm_prob->param(BCP_tm_par::CpProcessNum) > 0)
    //       allprocs.push_back(_cp_id.clone());
    //    if (_tm_prob->param(BCP_tm_par::VpProcessNum) > 0)
    //       allprocs.push_back(_vp_id.clone());

    _tm_prob->slaves.all = new BCP_proc_array;
    _tm_prob->slaves.all->add_procs(allprocs.begin(), allprocs.end());
    BCP_vec<BCP_proc_id*>::iterator first =
	_tm_prob->slaves.all->procs().begin();
    BCP_vec<BCP_proc_id*>::iterator last = first + 1;
    _tm_prob->active_nodes.insert(_tm_prob->active_nodes.end(), 1, 0);
    //-------------------------------------------------------------------------
    // LP
    id = _lp_id.clone();
    BCP_lp_prob* _lp_prob = new BCP_lp_prob(id, _tm_id.clone());
    processes[*id] = _lp_prob;
    _lp_prob->msg_env = new BCP_single_environment(*id);
    _tm_prob->slaves.lp = new BCP_proc_array;
    _tm_prob->slaves.lp->add_procs(first++, last++);
    //-------------------------------------------------------------------------
    // CG
    BCP_cg_prob* _cg_prob = 0;
    if (_tm_prob->param(BCP_tm_par::CgProcessNum) > 0) {
	id = _cg_id.clone();
	_cg_prob = new BCP_cg_prob(id, _tm_id.clone());
	processes[*id] = _cg_prob;
	_cg_prob->msg_env = new BCP_single_environment(*id);
	_tm_prob->slaves.cg = new BCP_proc_array;
	_tm_prob->slaves.cg->add_procs(first++, last++);
    }
    //-------------------------------------------------------------------------
    // VG
    BCP_vg_prob* _vg_prob = 0;
    if (_tm_prob->param(BCP_tm_par::VgProcessNum) > 0) {
	id = _vg_id.clone();
	_vg_prob = new BCP_vg_prob(id, _tm_id.clone());
	processes[*id] = _vg_prob;
	_vg_prob->msg_env = new BCP_single_environment(*id);
	_tm_prob->slaves.vg = new BCP_proc_array;
	_tm_prob->slaves.vg->add_procs(first++, last++);
    }
    //-------------------------------------------------------------------------
    // CP
    //    id = _cp_id.clone();
    //    _cp_prob = new BCP_cp_prob;(id, _tm_id.clone())
    //    processes[*id] = _cp_prob;
    //    _cp_prob->msg_env = new BCP_single_environment(*i);
    //    _tm_prob->slaves.cp = new BCP_proc_array;
    //    _tm_prob->slaves.cp->add_procs(first++, last++);
    //-------------------------------------------------------------------------
    // VP
    //    id = _vp_id.clone();
    //    _vp_prob = new BCP_vp_prob(id, _tm_id.clone());
    //    processes[*id] = _vp_prob;
    //    _vp_prob->msg_env = new BCP_single_environment(*id);
    //    _tm_prob->slaves.vp = new BCP_proc_array;
    //    _tm_prob->slaves.vp->add_procs(first++, last++);

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

    // copy over the user info
    _tm_prob->msg_buf.clear();
    _tm_prob->user->pack_module_data(_tm_prob->msg_buf, BCP_ProcessType_LP);
    _lp_prob->user->unpack_module_data(_tm_prob->msg_buf);
    _lp_prob->master_lp = _lp_prob->user->initialize_solver_interface();

    _lp_prob->upper_bound = std::min<double>(_tm_prob->ub(), 1e100);
    //-------------------------------------------------------------------------
    if (_cg_prob) {
	// CG (it already has the core)
	// copy over the parameter structure
	_cg_prob->par = _tm_prob->slave_pars.cg;
	// create the user universe
	_cg_prob->user = user_init->cg_init(*_cg_prob);
	_cg_prob->user->setCgProblemPointer(_cg_prob);
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
		   _tm_prob->slaves.lp->busy_num() > 0){
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

BCP_proc_id*
BCP_single_environment::parent_process() {
    throw BCP_fatal_error("\
BCP_single_environment::parent_process() invoked.\n");
    return 0; // to satisfy aCC on HP-UX
}

bool
BCP_single_environment::alive(const BCP_proc_id* pid)
{
    return true;
}

BCP_vec<BCP_proc_id*>::iterator
BCP_single_environment::alive(const BCP_proc_array& parray)
{
    return const_cast<BCP_vec<BCP_proc_id*>::iterator>(parray.procs().end());
}

//-----------------------------------------------------------------------------

void
BCP_single_environment::send(const BCP_proc_id* const target,
			     const BCP_message_tag tag) {
    BCP_single_id* id = BCP_is_single_id(target, "send(2)");
    BCP_process* target_process = processes[*id];
    BCP_buffer& target_buf = target_process->get_message_buffer();
    target_buf.clear();
    target_buf._sender = _my_id.clone();
    target_buf._msgtag = tag;
    target_process->process_message();
}

void
BCP_single_environment::send(const BCP_proc_id* const target,
			     const BCP_message_tag tag,
			     const BCP_buffer& buf) {
    BCP_single_id* id = BCP_is_single_id(target, "send(3)");
    BCP_process* target_process = processes[*id];
    BCP_buffer& target_buf = target_process->get_message_buffer();
    target_buf = buf;
    target_buf._sender = _my_id.clone();
    target_buf._msgtag = tag;
    target_process->process_message();
}
   
//-----------------------------------------------------------------------------

void
BCP_single_environment::multicast(const BCP_proc_array* const target,
				  const BCP_message_tag tag) {
    BCP_vec<BCP_proc_id*>::const_iterator pi = target->procs().begin();
    BCP_vec<BCP_proc_id*>::const_iterator lastpi = target->procs().end();
    while (pi != lastpi) {
	send(*pi, tag);
	++pi;
    }
}

void
BCP_single_environment::multicast(const BCP_proc_array* const target,
				  const BCP_message_tag tag,
				  const BCP_buffer& buf) {
    BCP_vec<BCP_proc_id*>::const_iterator pi = target->procs().begin();
    BCP_vec<BCP_proc_id*>::const_iterator lastpi = target->procs().end();
    while (pi != lastpi) {
	send(*pi, tag, buf);
	++pi;
    }
}

void
BCP_single_environment::multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
				  BCP_vec<BCP_proc_id*>::const_iterator end,
				  const BCP_message_tag tag) {
    while (beg != end) {
	send(*beg, tag);
	++beg;
    }
}

void
BCP_single_environment::multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
				  BCP_vec<BCP_proc_id*>::const_iterator end,
				  const BCP_message_tag tag,
				  const BCP_buffer& buf) {
    while (beg != end) {
	send(*beg, tag, buf);
	++beg;
    }
}

//-----------------------------------------------------------------------------

void
BCP_single_environment::receive(const BCP_proc_id* const source,
				const BCP_message_tag tag,
				BCP_buffer& buf, const double timeout) {
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
BCP_single_environment::probe(const BCP_proc_id* const source,
			      const BCP_message_tag tag) {
    // In the single_environment this is not needed.
    return false;
}

//-----------------------------------------------------------------------------

BCP_proc_id*
BCP_single_environment::unpack_proc_id(BCP_buffer& buf) {
    int pid;
    buf.unpack(pid);
    return new BCP_single_id(pid);
}

void
BCP_single_environment::pack_proc_id(BCP_buffer& buf, const BCP_proc_id* pid) {
    BCP_single_id* id = BCP_is_single_id(pid, "pack_proc_id()");
    buf.pack(id->pid());
}
	    
//-----------------------------------------------------------------------------
// These should never be invoked in a BCP_single_environment

BCP_proc_id*
BCP_single_environment::start_process(const BCP_string& exe,
				      const bool debug) {
    throw BCP_fatal_error("start_process() called!\n");
    //   return new BCP_single_id(pid);
    return 0; // to satisfy aCC on HP-UX
}

BCP_proc_id*
BCP_single_environment::start_process(const BCP_string& exe,
				      const BCP_string& machine,
				      const bool debug){
    throw BCP_fatal_error("start_process() called!\n");
    //   return new BCP_single_id(pid);
    return 0; // to satisfy aCC on HP-UX
}

BCP_proc_array*
BCP_single_environment::start_processes(const BCP_string& exe,
					const int proc_num,
					const bool debug) {
    throw BCP_fatal_error("start_processes() called!\n");
    //   return new BCP_proc_array(procs);
    return 0; // to satisfy aCC on HP-UX
}

BCP_proc_array*
BCP_single_environment::start_processes(const BCP_string& exe,
					const int proc_num,
					const BCP_vec<BCP_string>& machines,
					const bool debug){
    throw BCP_fatal_error("start_processes() called!\n");
    //   return new BCP_proc_array(procs);
    return 0; // to satisfy aCC on HP-UX
}

//-----------------------------------------------------------------------------

// void BCP_single_environment::stop_process(const BCP_proc_id* process) {
//    check_error( pvm_kill(BCP_is_single_id(process, "stop_process()")),
// 		"stop_process()");
// }

// void BCP_single_environment::stop_processes(const BCP_proc_array* processes){
//    BCP_vec<BCP_proc_id*>::const_iterator first = processes.procs.begin();
//    BCP_vec<BCP_proc_id*>::const_iterator last = pprocesses.procs.end();
//    while (first != last)
//       check_error( pvm_kill(BCP_is_single_id(*first, "stop_processes()")),
// 		   "stop_processes()");
//       ++first;
//    }
// }
