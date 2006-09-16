// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "CoinTime.hpp"

#include "BCP_lp_functions.hpp"
#include "BCP_enum.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_node.hpp"

int BCP_lp_generate_cuts(BCP_lp_prob& p,
			 bool varset_changed, const bool from_repricing)
{
    double time0 = CoinCpuTime();

    BCP_lp_result& lpres = *p.lp_result;
    BCP_lp_cut_pool& cp = *p.local_cut_pool;
    int prev_size = cp.size();

    if (prev_size > 0 && ! cp.rows_are_valid()){
	// we must regenerate the rows from the constraints
	// first delete the old rows then expand the cuts again

	// these will hold cuts and rows expanded from the cuts
	BCP_vec<BCP_cut*> cuts;
	BCP_vec<BCP_row*> rows;

	cuts.reserve(prev_size);
	int i;
	for (i = 0; i < prev_size; ++i) {
	    cp[i]->delete_row();
	    cuts.unchecked_push_back(cp[i]->cut());
	}
	// now expand
	rows.reserve(prev_size);
	p.user->cuts_to_rows(p.node->vars, cuts, rows,
			     lpres, BCP_Object_Leftover, false);
	for (i = 0; i < prev_size; ++i) {
	    cp[i]->set_row(rows[i]);
	}
	rows.clear();
    }
    cp.rows_are_valid(true);

    if (p.param(BCP_lp_par::LpVerb_ReportLocalCutPoolSize))
	printf("LP:   Number of leftover cuts: %i\n", prev_size);

    // Generate cuts within the LP process
    BCP_vec<BCP_cut*> new_cuts;
    BCP_vec<BCP_row*> new_rows;
    if (p.user_has_lp_result_processing) {
	new_cuts = p.new_cuts;
	new_rows = p.new_rows;
	p.new_cuts.clear();
	p.new_rows.clear();
    } else {
	p.user->generate_cuts_in_lp(lpres, p.node->vars, p.node->cuts,
				    new_cuts, new_rows);
    }
    if (new_cuts.size() > 0) {
	const int new_size = new_cuts.size();
	if (new_rows.size() != 0) {
	    if (static_cast<int>(new_rows.size()) != new_size) {
		throw BCP_fatal_error("\
LP: uneven new_cuts/new_rows sizes in generate_cuts_in_lp().\n");
	    }
	} else {
	    // expand the generated cuts
	    new_rows.reserve(new_size);
	    p.user->cuts_to_rows(p.node->vars, new_cuts, new_rows,
				 lpres, BCP_Object_FromGenerator, false);
	}

	cp.reserve(cp.size() + new_size);
	for (int i = 0; i < new_size; ++i) {
	    new_cuts[i]->set_bcpind(-BCP_lp_next_cut_index(p));
	    cp.unchecked_push_back(new BCP_lp_waiting_row(new_cuts[i],
							  new_rows[i]));
	}
	new_rows.clear();
	new_cuts.clear();
	if (p.param(BCP_lp_par::LpVerb_ReportLocalCutPoolSize))
	    printf("LP:   Number of cuts generated in the LP process: %i\n",
		   new_size);
	prev_size = cp.size();
    }

    // Compute the violation for everything in the local cut pool and throw out
    // the ones not violated
    if (prev_size > 0) {
	cp.compute_violations(lpres, cp.begin(), cp.end());
	double petol = 0.0;
	p.lp_solver->getDblParam(OsiPrimalTolerance, petol);
	const int cnt = cp.remove_nonviolated(petol);
	if (p.param(BCP_lp_par::LpVerb_ReportLocalCutPoolSize))
	    printf("LP:   Non-violated (hence removed): %i\n", cnt);
	prev_size = cp.size();
    }

    if (p.param(BCP_lp_par::MessagePassingIsSerial)) {
	// If the message passing environment is not really parallel (i.e.,
	// while the CG/CP are working the LP stops and also the LP must
	// immediately process any cuts sent back then this is the place to
	// send the lp solution to the CG/CP.
	// send the current solution to CG, and also to CP if we are either
	//  - at the beginning of a chain (but not in the root in the
	//    first phase)
	//  - or this is the cut_pool_check_freq-th iteration.
	if (p.node->cg || p.node->cp) {
	    const BCP_message_tag msgtag = BCP_lp_pack_for_cg(p);
	    if (p.node->cg) {
		++p.no_more_cuts_cnt;
		p.msg_env->send(p.node->cg, msgtag, p.msg_buf);
	    }
	    if (p.node->cp) {
		if (! (p.node->iteration_count %
		       p.param(BCP_lp_par::CutPoolCheckFrequency))
		    || varset_changed) {
		    ++p.no_more_cuts_cnt;
		    p.msg_env->send(p.node->cp, msgtag, p.msg_buf);
		}
	    }
	}
    }

    if (p.no_more_cuts_cnt > 0){
	// Receive cuts if we have sent out the lp solution somewhere.
	// set the timeout (all the times are in microseconds).
	double first_cut_time_out = varset_changed ?
	    p.param(BCP_lp_par::FirstLP_FirstCutTimeout) :
	    p.param(BCP_lp_par::LaterLP_FirstCutTimeout);
	double all_cuts_time_out = varset_changed ?
	    p.param(BCP_lp_par::FirstLP_AllCutsTimeout) :
	    p.param(BCP_lp_par::LaterLP_AllCutsTimeout);
	double tout = cp.size() == 0 ? first_cut_time_out : all_cuts_time_out;
	double tin = CoinCpuTime();

	while(true){
	    p.msg_buf.clear();
	    p.msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage,
			       p.msg_buf, tout);
	    if (p.msg_buf.msgtag() == BCP_Msg_NoMessage){
		// check that everyone is still alive
		if (! p.msg_env->alive(p.get_parent() /*tree_manager*/))
		    throw BCP_fatal_error("\
LP:   The TM has died -- LP exiting\n");
		if (p.node->cg && ! p.msg_env->alive(p.node->cg))
		    throw BCP_fatal_error("\
LP:   The CG has died -- LP exiting\n");
		if (p.node->cp && ! p.msg_env->alive(p.node->cp))
		    throw BCP_fatal_error("\
LP:   The CP has died -- LP exiting\n");
		if (p.node->vg && ! p.msg_env->alive(p.node->vg))
		    throw BCP_fatal_error("\
LP:   The VG has died -- LP exiting\n");
		if (p.node->vp && ! p.msg_env->alive(p.node->vp))
		    throw BCP_fatal_error("\
LP:   The VP has died -- LP exiting\n");
		// now the message queue is empty and received_message has
		// returned, i.e., we have waited enough
		if (p.param(BCP_lp_par::LpVerb_ReportCutGenTimeout))
		    printf("LP:   Receive cuts timed out after %f secs\n",
			   (prev_size != static_cast<int>(cp.size()) ?
			    all_cuts_time_out : first_cut_time_out));
		break;
	    }
	    p.process_message();
	    // break out if no more cuts can come
	    if (p.no_more_cuts_cnt == 0)
		break;

	    // reset the timeout
	    tout = cp.size() == 0 ? first_cut_time_out : all_cuts_time_out;
	    if (tout >= 0){
		// with this tout we'll read out the rest of the message queue
		// even if cut generation times out.
		tout = std::max<double>(0.0, tout - (CoinCpuTime() - tin));
	    }
	}
    }
    // reset no_more_cuts_cnt to 0
    p.no_more_cuts_cnt = 0;

    if (p.param(BCP_lp_par::LpVerb_ReportLocalCutPoolSize)) {
	printf("LP:   Number of cuts received from CG: %i\n",
	       static_cast<int>(cp.size() - prev_size));
	printf("LP:   Total number of cuts in local pool: %i\n",
	       static_cast<int>(cp.size()));
    }

    if (cp.size() > 0) {
	const int oldsize = cp.size();
	double petol = 0.0;
	p.lp_solver->getDblParam(OsiPrimalTolerance, petol);
	const int cnt = cp.remove_nonviolated(petol);
	if (cnt > 0) {
	    printf("\
LP: *WARNING*: There are nonviolated cuts in the local CP\n\
               at the end of cut generation.\n\
               Discarding %i cuts out of %i.\n", cnt, oldsize);
	}
    }

    p.stat.time_cut_generation += CoinCpuTime() - time0;

    return cp.size();
}
