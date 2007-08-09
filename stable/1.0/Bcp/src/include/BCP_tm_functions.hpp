// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_FUNCTIONS_H
#define _BCP_TM_FUNCTIONS_H

#include <utility> // for pair

#include "BCP_string.hpp"
#include "BCP_vector.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_parameters.hpp"
#include "BCP_enum_tm.hpp"

#define BCP__DUMP_PROCINFO 0

class BCP_proc_id;
class BCP_buffer;
class BCP_tm_prob;
class BCP_lp_prob;
class BCP_cg_prob;
class BCP_vg_prob;
class BCP_tm_node;
class BCP_var;
class BCP_cut;
class BCP_problem_core;
class BCP_problem_core_change;
class BCP_tm_node;
class BCP_var_set_change;
class BCP_cut_set_change;
class USER_packing;

//-----------------------------------------------------------------------------
// BCP_tm_commandline.cpp
void
BCP_tm_parse_command_line(BCP_tm_prob& p,
			  const int argnum, const char* const * arglist);
//-----------------------------------------------------------------------------
// BCP_tm_main.cpp
bool BCP_tm_do_one_phase(BCP_tm_prob& p);
BCP_problem_core* BCP_tm_create_core(BCP_tm_prob& p);
BCP_tm_node* BCP_tm_create_root(BCP_tm_prob& p);
void BCP_tm_tasks_before_new_phase(BCP_tm_prob& p);

//-----------------------------------------------------------------------------
// BCP_tm_trimming.cpp
void BCP_tm_trim_tree_wrapper(BCP_tm_prob& p, const bool between_phases);
void BCP_tm_remove_explored(BCP_tm_prob& p, BCP_tm_node* node);

//-----------------------------------------------------------------------------
// BCP_tm_msgproc.cpp
void BCP_tm_idle_processes(BCP_tm_prob& p);
void BCP_tm_stop_processes(BCP_tm_prob& p);
void BCP_tm_start_processes(BCP_tm_prob& p);
bool BCP_tm_test_machine(BCP_tm_prob& p);
void BCP_tm_modify_pool_counters(BCP_tm_prob& p, BCP_tm_node* node);
void BCP_tm_remove_lp(BCP_tm_prob& p, const int index);
void BCP_tm_remove_cg(BCP_tm_prob& p, const int index);
void BCP_tm_remove_vg(BCP_tm_prob& p, const int index);
void BCP_tm_notify_about_new_phase(BCP_tm_prob& p);
void BCP_tm_notify_processes(BCP_tm_prob& p);
void BCP_tm_distribute_core(BCP_tm_prob& p);
void BCP_tm_distribute_user_info(BCP_tm_prob& p);
void BCP_tm_unpack_priced_root(BCP_tm_prob& p, BCP_buffer& buf);
void BCP_tm_free_procs_of_node(BCP_tm_prob& p, BCP_tm_node* node);

//-----------------------------------------------------------------------------
// BCP_tm_msg_node_send.cpp
void BCP_tm_send_node(BCP_tm_prob& p, const BCP_tm_node* node,
		      const BCP_message_tag msgtag);

//-----------------------------------------------------------------------------
// BCP_tm_msg_node_rec.cpp
BCP_vec<int>* BCP_tm_unpack_noncore_vars(USER_packing& user,
					 BCP_buffer& buf,
					 BCP_var_set_change& var_ch,
					 BCP_vec<BCP_var*>& varlist);
BCP_vec<int>* BCP_tm_unpack_noncore_cuts(USER_packing& user,
					 BCP_buffer& buf,
					 BCP_cut_set_change& cut_ch,
					 BCP_vec<BCP_cut*>& cutlist);

void BCP_tm_unpack_node_with_branching_info(BCP_tm_prob& p, BCP_buffer& buf);
BCP_tm_node* BCP_tm_unpack_node_no_branching_info(BCP_tm_prob& p,
						  BCP_buffer& buf);
//-----------------------------------------------------------------------------
// BCP_tm_functions.cpp
BCP_vec< std::pair<BCP_proc_id*, int> >::iterator
BCP_tm_identify_process(BCP_vec< std::pair<BCP_proc_id*, int> >& proclist,
			BCP_proc_id* proc);
bool BCP_tm_assign_processes(BCP_tm_prob& p, BCP_tm_node* node);
BCP_node_start_result BCP_tm_start_new_nodes(BCP_tm_prob& p);
void BCP_tm_list_candidates(BCP_tm_prob& p);
void BCP_check_parameters(BCP_tm_prob& p);
void BCP_sanity_checks(BCP_tm_prob& p);

//-----------------------------------------------------------------------------
// BCP_tm_statistics.cpp
void BCP_tm_save_root_cuts(BCP_tm_prob* tm);
void BCP_tm_wrapup(BCP_tm_prob* tm, BCP_lp_prob* lp,
		   BCP_cg_prob* cg, BCP_vg_prob* vg, bool final_stat);

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
void dump_procinfo(BCP_tm_prob& p, const char* str);
#endif
#endif

#endif
