// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_FUNCTIONS_H
#define _BCP_LP_FUNCTIONS_H

#include "BcpConfig.h"

// This file is fully docified.
// Actually, there's nothing to docify here...

#include <vector>

#include "BCP_enum.hpp"
#include "BCP_enum_branch.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_vector.hpp"

class OsiSolverInterface;
class CoinWarmStart;

class BCP_buffer;
class BCP_lp_prob;
class BCP_lp_result;
class BCP_lp_branching_object;
class BCP_presolved_lp_brobj;
class BCP_solution;
class BCP_col;
class BCP_row;
class BCP_warmstart;
class BCP_var;

//-----------------------------------------------------------------------------
// BCP_lp_main.cpp
BCPLIB_EXPORT void
BCP_lp_process_core(BCP_lp_prob& p, BCP_buffer& buf);

//-----------------------------------------------------------------------------
// BCP_lp_repricing.cpp
void BCP_lp_repricing(BCP_lp_prob& p);

//-----------------------------------------------------------------------------
// BCP_lp_main_loop.cpp
BCPLIB_EXPORT void BCP_lp_main_loop(BCP_lp_prob& p);

//-----------------------------------------------------------------------------
// BCP_lp_fathom.cpp
BCPLIB_EXPORT void BCP_price_vars(BCP_lp_prob& p, const bool from_fathom,
		    BCP_vec<BCP_var*>& vars_to_add,
		    BCP_vec<BCP_col*>& cols_to_add);
BCPLIB_EXPORT void BCP_restore_feasibility(BCP_lp_prob& p,
			     const std::vector<double*> dual_rays,
			     BCP_vec<BCP_var*>& vars_to_add,
			     BCP_vec<BCP_col*>& cols_to_add);
BCPLIB_EXPORT void BCP_lp_perform_fathom(BCP_lp_prob& p, const char* msg,
			   BCP_message_tag msgtag);
BCPLIB_EXPORT bool BCP_lp_fathom(BCP_lp_prob& p, const bool from_repricing);

//-----------------------------------------------------------------------------
// BCP_lp_generate_cuts.cpp
BCPLIB_EXPORT int BCP_lp_generate_cuts(BCP_lp_prob& p,
			 bool first_in_loop, const bool from_repricing);

//-----------------------------------------------------------------------------
// BCP_lp_generate_vars.cpp
BCPLIB_EXPORT int BCP_lp_generate_vars(BCP_lp_prob& p,
			 bool first_in_loop, const bool from_repricing);

//-----------------------------------------------------------------------------
// BCP_lp_misc.cpp
BCPLIB_EXPORT void BCP_lp_process_result(BCP_lp_prob& p, const BCP_lp_result& lpres);
BCPLIB_EXPORT void BCP_lp_purge_slack_pool(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_test_feasibility(BCP_lp_prob& p, const BCP_lp_result& lpres);
BCPLIB_EXPORT double BCP_lp_compute_lower_bound(BCP_lp_prob& p, const BCP_lp_result& lpres);
BCPLIB_EXPORT void BCP_lp_clean_up_node(BCP_lp_prob& p);
BCPLIB_EXPORT BCP_message_tag BCP_lp_pack_for_cg(BCP_lp_prob& p);
BCPLIB_EXPORT BCP_message_tag BCP_lp_pack_for_vg(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_prepare_for_new_node(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_add_cols_to_lp(const BCP_vec<BCP_col*>& cols,
			   OsiSolverInterface* lp);
BCPLIB_EXPORT void BCP_lp_add_rows_to_lp(const BCP_vec<BCP_row*>& rows,
			   OsiSolverInterface* lp);
//-----------------------------------------------------------------------------
// BCP_lp_msgproc.cpp
BCPLIB_EXPORT void BCP_lp_check_ub(BCP_lp_prob& p);
BCPLIB_EXPORT int BCP_lp_next_var_index(BCP_lp_prob& p);
BCPLIB_EXPORT int BCP_lp_next_cut_index(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_process_ub_message(BCP_lp_prob& p, BCP_buffer& buf);
BCPLIB_EXPORT void BCP_lp_send_cuts_to_cp(BCP_lp_prob& p, const int eff_cnt_limit);
BCPLIB_EXPORT void BCP_lp_unpack_diving_info(BCP_lp_prob& p, BCP_buffer& buf);

//-----------------------------------------------------------------------------
// BCP_lp_branch.cpp
BCPLIB_EXPORT BCP_branching_result
BCP_lp_branch(BCP_lp_prob& p);

//-----------------------------------------------------------------------------
// BCP_lp_colrow.cpp
BCPLIB_EXPORT bool BCP_lp_fix_vars(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_adjust_row_effectiveness(BCP_lp_prob& p);
BCPLIB_EXPORT void BCP_lp_delete_cols_and_rows(BCP_lp_prob& p,
				 BCP_lp_branching_object* can,
				 const int added_colnum,
				 const int added_rownum,
				 const bool from_fathom,
				 const bool force_delete);
BCPLIB_EXPORT int BCP_lp_add_from_local_cut_pool(BCP_lp_prob& p);
BCPLIB_EXPORT int BCP_lp_add_from_local_var_pool(BCP_lp_prob& p);

//-----------------------------------------------------------------------------
// BCP_lp_msg_node_send.cpp
// brobj is 0, msgtag is 'real' when invoked from fathom().
// brobj is 'real', msgtag is BCP_Msg_NoMessage when invoked from branch()
BCPLIB_EXPORT int BCP_lp_send_node_description(BCP_lp_prob& p,
				 BCP_presolved_lp_brobj* brobj,
				 BCP_message_tag msgtag);

//-----------------------------------------------------------------------------
// BCP_lp_msg_node_rec.cpp
BCPLIB_EXPORT void BCP_lp_unpack_active_node(BCP_lp_prob& p, BCP_buffer& buf);

//-----------------------------------------------------------------------------
// BCP_lp_create_lp.cpp
BCPLIB_EXPORT void BCP_lp_create_lp(BCP_lp_prob& p);

//-----------------------------------------------------------------------------
// BCP_lp_create_warmstart.cpp
// The calling functions will consider ws to be lost! This function should
// either build it into the created BCP_warmstart or delete it.
BCPLIB_EXPORT BCP_warmstart* BCP_lp_convert_CoinWarmStart(BCP_lp_prob& p, CoinWarmStart*& ws);

#endif
