// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
//#include <cfloat>
#include <cstdio>
#include <numeric>
#include <utility> // for pair<>

#include "CoinWarmStart.hpp"
#include "CoinTime.hpp"

#include "BCP_math.hpp"
#include "BCP_enum.hpp"
#include "BCP_matrix.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_lp.hpp"

//#############################################################################

static inline std::pair<int,int>
BCP_add_branching_objects(BCP_lp_prob& p,
			  BCP_vec<BCP_lp_branching_object*>& candidates);
static inline void
BCP_mark_result_of_strong_branching(BCP_lp_prob& p,
				    const BCP_lp_branching_object* can,
				    const int added_col_num,
				    const int added_row_num);
static inline BCP_branching_decision
BCP_lp_select_branching_object(BCP_lp_prob& p,
			       BCP_presolved_lp_brobj*& best_presolved);
static inline void
BCP_lp_make_parent_from_node(BCP_lp_prob& p);

//#############################################################################

static inline void
BCP_print_brobj_stat(BCP_lp_prob& p,
		     const int orig_varnum,
		     const int candidate_num, const int selected,
		     const BCP_presolved_lp_brobj* best_presolved)
{
    const BCP_lp_branching_object* can = best_presolved->candidate();

    if (p.param(BCP_lp_par::LpVerb_StrongBranchResult)) {
	printf("\nLP:   SB selected candidate %i out of %i.\n\n",
	       selected, candidate_num);
	printf("LP:   The selected object is:");
	if (p.param(BCP_lp_par::LpVerb_StrongBranchPositions)) {
	    can->print_branching_info(orig_varnum,
				      p.lp_result->x(),
				      p.lp_solver->getObjCoefficients());
	}
	for (int i = 0; i < can->child_num; ++i) {
	    const BCP_lp_result& res = best_presolved->lpres(i);
	    const double lb = res.objval();
	    printf((lb > BCP_DBL_MAX/10 ? " [%e,%i,%i]" : " [%.4f,%i,%i]"),
		   lb, res.termcode(), res.iternum());
	}
	printf("\n");
    }
    // Print some statistics
    if (p.param(BCP_lp_par::LpVerb_ChildrenInfo)){
	const BCP_vec<BCP_child_action>& action = best_presolved->action();
	for (int i = can->child_num - 1; i >= 0; --i)
	    switch (action[i]){
	    case BCP_KeepChild:
	    case BCP_ReturnChild:
		break;
	    case BCP_FathomChild:
		printf("LP:   Child %i is fathomed.\n", i);
		break;
	    }
    }
}

//#############################################################################

static inline std::pair<int,int>
BCP_add_branching_objects(BCP_lp_prob& p,
			  BCP_vec<BCP_lp_branching_object*>& candidates){
    // to make things short
    if (candidates.size() == 0)
	return std::pair<int,int>(0, 0);

    BCP_lp_branching_object* can;
    BCP_vec<BCP_lp_branching_object*>::iterator cani;
    BCP_vec<BCP_lp_branching_object*>::iterator lastcani = candidates.end();
    BCP_var_set& vars = p.node->vars;
    BCP_cut_set& cuts = p.node->cuts;

    // first count the number of cols/rows to add
    int newvar_num = 0;
    int newcut_num = 0;
    for (cani = candidates.begin(); cani != lastcani; ++cani){
	can = *cani;
	can->init_pos_for_added(vars.size() + newvar_num,
				cuts.size() + newcut_num);
	if (can->vars_to_add)
	    newvar_num += can->vars_to_add->size();
	if (can->cuts_to_add)
	    newcut_num += can->cuts_to_add->size();
    }

    const int orig_col_num = vars.size();
    const int orig_row_num = cuts.size();

    OsiSolverInterface* lp = p.lp_solver;

    // deal with the vars
    if (newvar_num > 0){
	BCP_vec<BCP_var*> new_vars;
	new_vars.reserve(newvar_num);
	for (cani = candidates.begin(); cani != lastcani; ++cani){
	    can = *cani;
	    if (can->vars_to_add)
		new_vars.append(*can->vars_to_add);
	}

	BCP_vec<BCP_col*> cols;
	cols.reserve(newvar_num);
	p.user->vars_to_cols(cuts, new_vars, cols,
			     *p.lp_result, BCP_Object_Branching, false);
	BCP_lp_add_cols_to_lp(cols, lp);
	purge_ptr_vector(cols);

	for (int i = 0; i < newvar_num; ++i) {
	    new_vars[i]->set_bcpind(-BCP_lp_next_var_index(p));
	}
	vars.append(new_vars);
    }

    // now add the rows
    if (newcut_num > 0){
	BCP_vec<BCP_cut*> new_cuts;
	new_cuts.reserve(newcut_num);
	for (cani = candidates.begin(); cani != lastcani; ++cani){
	    can = *cani;
	    if (can->cuts_to_add)
		new_cuts.append(*can->cuts_to_add);
	}

	BCP_vec<BCP_row*> rows;
	rows.reserve(newcut_num);
	BCP_fatal_error::abort_on_error = false;
	try {
	    p.user->cuts_to_rows(vars, new_cuts, rows,
				 *p.lp_result, BCP_Object_Branching, false);
	}
	catch (...) {
	}
	BCP_fatal_error::abort_on_error = true;
	BCP_lp_add_rows_to_lp(rows, lp);
	purge_ptr_vector(rows);

	for (int i = 0; i < newcut_num; ++i) {
	    new_cuts[i]->set_bcpind(-BCP_lp_next_cut_index(p));
	}
	cuts.append(new_cuts);
	p.node->lb_at_cutgen.insert(p.node->lb_at_cutgen.end(), newcut_num,
				    p.lp_result->objval());
    }

    // mark the newly added vars as fixed to 0, and the newly added cuts as
    // free. (var_indices and cut_indices simply contain the indices of the
    // added vars/cuts.)

    if (newvar_num > 0) {
	for (int i = orig_col_num; i < orig_col_num + newvar_num; ++i)
	    lp->setColBounds(i, 0.0, 0.0);
    }
    if (newcut_num > 0) {
	const double inf = lp->getInfinity();
	for (int i = orig_row_num; i < orig_row_num + newcut_num; ++i)
	    lp->setRowBounds(i, -inf, inf);
    }

    return std::pair<int,int>(newvar_num, newcut_num);
}

//#############################################################################

static inline void
BCP_mark_result_of_strong_branching(BCP_lp_prob& p,
				    const BCP_lp_branching_object* can,
				    const int added_col_num,
				    const int added_row_num)
{
    BCP_var_set& vars = p.node->vars;
    if (can->forced_var_pos) {
	BCP_vec<int>::const_iterator ii = can->forced_var_pos->begin();
	BCP_vec<int>::const_iterator lastii = can->forced_var_pos->end();
	for ( ; ii != lastii; ++ii)
	    vars[*ii]->make_non_removable();
    }
    if (can->implied_var_pos) {
	// just to make sure that these vars are not deleted before the implied
	// bound change takes place...
	BCP_vec<int>::const_iterator ii = can->implied_var_pos->begin();
	BCP_vec<int>::const_iterator lastii = can->implied_var_pos->end();
	for ( ; ii != lastii; ++ii)
	    vars[*ii]->make_active();
    }
      
    if (added_col_num) {
	BCP_var_set::iterator vari = vars.entry(vars.size() - added_col_num);
	BCP_var_set::const_iterator lastvari = vars.end();
	for ( ; vari != lastvari; ++vari) {
	    if (! (*vari)->is_non_removable())
		(*vari)->make_to_be_removed();
	}
    }

    BCP_cut_set& cuts = p.node->cuts;
    if (can->forced_cut_pos) {
	BCP_vec<int>::const_iterator ii = can->forced_cut_pos->begin();
	BCP_vec<int>::const_iterator lastii = can->forced_cut_pos->end();
	for ( ; ii != lastii; ++ii)
	    cuts[*ii]->make_non_removable();
    }
    if (can->implied_cut_pos) {
	// just to make sure that these cuts are not deleted before the implied
	// bound change takes place...
	BCP_vec<int>::const_iterator ii = can->implied_cut_pos->begin();
	BCP_vec<int>::const_iterator lastii = can->implied_cut_pos->end();
	for ( ; ii != lastii; ++ii)
	    cuts[*ii]->make_active();
    }
    if (added_row_num > 0){
	BCP_cut_set::iterator cuti = cuts.entry(cuts.size() - added_row_num);
	BCP_cut_set::const_iterator lastcuti = cuts.end();
	for ( ; cuti != lastcuti; ++cuti) {
	    if (! (*cuti)->is_non_removable())
		(*cuti)->make_to_be_removed();
	}
    }
}

//#############################################################################

static inline int
BCP_lp_perform_strong_branching(BCP_lp_prob& p,
				BCP_vec<BCP_lp_branching_object*>& candidates,
				BCP_presolved_lp_brobj*& best_presolved)
{
    OsiSolverInterface* lp = p.lp_solver;
    BCP_var_set& vars = p.node->vars;
    BCP_cut_set& cuts = p.node->cuts;

    const int orig_colnum = vars.size();

    const std::pair<int,int> added_object_num =
	BCP_add_branching_objects(p, candidates);
   
    const int added_colnum = added_object_num.first;
    const int added_rownum = added_object_num.second;

    const int colnum = vars.size();
    const int rownum = cuts.size();

    int i, j; // loop variable

    const CoinWarmStart * ws = p.lp_solver->getWarmStart();

    // prepare for strong branching
    lp->markHotStart();

    // save the lower/upper bounds of every var/cut
    BCP_vec<double> rowbounds(2 * rownum, 0.0);
    BCP_vec<double> colbounds(2 * colnum, 0.0);

    const int maxind = std::max<int>(rownum, colnum);
    BCP_vec<int> all_indices(maxind, 0);
    for (i = 0; i < maxind; ++i)
	all_indices[i] = i;

    const double * rlb_orig = lp->getRowLower();
    const double * rub_orig = lp->getRowUpper();
    for (j = -1, i = 0; i < rownum; ++i) {
	rowbounds[++j] = rlb_orig[i];
	rowbounds[++j] = rub_orig[i];
    }

    const double * clb_orig = lp->getColLower();
    const double * cub_orig = lp->getColUpper();
    for (j = -1, i = 0; i < colnum; ++i) {
	colbounds[++j] = clb_orig[i];
	colbounds[++j] = cub_orig[i];
    }

    best_presolved = 0;

    // Look at the candidates one-by-one and presolve them.
    BCP_vec<BCP_lp_branching_object*>::iterator cani;

    if (p.param(BCP_lp_par::MaxPresolveIter) > 0) {
	lp->setIntParam(OsiMaxNumIterationHotStart,
			p.param(BCP_lp_par::MaxPresolveIter));
    }

    if (p.param(BCP_lp_par::LpVerb_StrongBranchResult)) {
	printf("\nLP: Starting strong branching:\n\n");
    }

    const OsiBabSolver* babSolver = p.user->getOsiBabSolver();

    for (cani = candidates.begin(); cani != candidates.end(); ++cani){
	// Create a temporary branching object to hold the current results
	BCP_presolved_lp_brobj* tmp_presolved =
	    new BCP_presolved_lp_brobj(*cani);
	const BCP_lp_branching_object* can = *cani;
	for (i = 0; i < can->child_num; ++i){
	    can->apply_child_bd(lp, i);
	    p.user->modify_lp_parameters(p.lp_solver, true);
#if 0
	    char fname[1000];
	    sprintf(fname, "matrix-%i.%i.%i-child-%i",
		    p.node->level, p.node->index, p.node->iteration_count, i);
	    lp->writeMps(fname, "mps");
#endif
	    lp->solveFromHotStart();
	    tmp_presolved->get_results(*lp, i);
	    BCP_lp_test_feasibility(p, tmp_presolved->lpres(i));
	    if (babSolver) {
		p.user->generate_cuts_in_lp(tmp_presolved->lpres(i),
					    p.node->vars, p.node->cuts,
					    tmp_presolved->get_new_cuts()[i],
					    tmp_presolved->get_new_rows()[i]);
	    }
	}
	// reset the bounds of the affected vars/cuts
	if (can->cuts_affected() > 0)
	    lp->setRowSetBounds(all_indices.begin(), all_indices.entry(rownum),
				rowbounds.begin());
	if (can->vars_affected() > 0)
	    lp->setColSetBounds(all_indices.begin(), all_indices.entry(colnum),
				colbounds.begin());

	if (p.param(BCP_lp_par::LpVerb_PresolveResult)) {
	    printf("LP:   Presolving:");
	    if (p.param(BCP_lp_par::LpVerb_PresolvePositions)) {
		can->print_branching_info(orig_colnum,
					  p.lp_result->x(),
					  p.lp_solver->getObjCoefficients());
	    }
	    for (i = 0; i < can->child_num; ++i) {
		const BCP_lp_result& res = tmp_presolved->lpres(i);
		const double lb = res.objval();
		printf((lb > BCP_DBL_MAX/10 ? " [%e,%i,%i]" : " [%.4f,%i,%i]"),
		       lb, res.termcode(), res.iternum());
	    }
	    printf("\n");
	}
	// Compare the current one with the best so far
	switch (p.user->compare_branching_candidates(tmp_presolved,
						     best_presolved)) {
	case BCP_OldPresolvedIsBetter:
	    break;
	case BCP_NewPresolvedIsBetter:
	    std::swap(tmp_presolved, best_presolved);
	    break;
	case BCP_NewPresolvedIsBetter_BranchOnIt:
	    // Free the remaining candidates if there are any. This also resets
	    // candidates.end(), thus 
	    purge_ptr_vector(candidates, cani + 1, candidates.end());
	    std::swap(tmp_presolved, best_presolved);
	    break;
	}
	delete tmp_presolved;
    }

    // indicate to the lp solver that strong branching is done
    lp->unmarkHotStart();
    p.lp_solver->setWarmStart(ws);

    // delete all the candidates but the selected one (candidates will just
    // silently go out of scope and we'll be left with a pointer to the final
    // candidate in best_presolved).
    BCP_lp_branching_object* can = best_presolved->candidate();
    int selected = 0;
    for (i=0, cani=candidates.begin(); cani != candidates.end(); ++cani, ++i) {
	if (*cani != can) {
	    delete *cani;
	} else {
	    selected = i;
	}
    }

    // Mark the cols/rows of the OTHER candidates as removable
    BCP_mark_result_of_strong_branching(p, can, added_colnum, added_rownum);
    // Delete whatever cols/rows we want to delete. This function also updates
    // var/cut_positions !!!
    BCP_lp_delete_cols_and_rows(p, can, added_colnum, added_rownum,
				false /* not from fathom */,
				true /* to force deletion */);
   
    delete ws;

    return selected;
}

//#############################################################################

static inline BCP_branching_decision
BCP_lp_select_branching_object(BCP_lp_prob& p,
			       BCP_presolved_lp_brobj*& best_presolved)
{
    BCP_var_set& vars = p.node->vars;
    BCP_cut_set& cuts = p.node->cuts;
    BCP_vec<BCP_lp_branching_object*> candidates;

    BCP_branching_decision do_branch = 
	p.user->select_branching_candidates(*p.lp_result,
					    vars, cuts,
					    *p.local_var_pool,
					    *p.local_cut_pool,
					    candidates);
    switch (do_branch){
    case BCP_DoNotBranch_Fathomed:
	return BCP_DoNotBranch_Fathomed;
    case BCP_DoNotBranch:
	if (p.local_var_pool->size() == 0 && p.local_cut_pool->size() == 0) {
	    printf("\
LP: ***WARNING*** : BCP_DoNotBranch, but nothing can be added! ***WARNING***\n");
	    //throw BCP_fatal_error("BCP_DoNotBranch, but nothing can be added!\n");
	}
	return BCP_DoNotBranch;
    case BCP_DoBranch:
	break;
    }

    // give error message if there are no branching candidates
    if (candidates.size() < 1) {
	throw BCP_fatal_error("\
BCP_lp_select_branching_object: branching forced but no candidates selected\n");
    }
   
    // ** OK, now we have to branch. **
    double time0 = CoinCpuTime();
    const int orig_colnum = p.node->vars.size();

    // if branching candidates are not presolved then choose the first
    // branching candidate as the best candidate.
    int selected = 0;
    if (p.param(BCP_lp_par::MaxPresolveIter) < 0) {
	if (candidates.size() > 1) {
	    printf("\
LP: Strong branching is disabled but more than one candidate is selected.\n\
    Deleting all candidates but the first.\n");
	    // delete all other candidates
	    BCP_vec<BCP_lp_branching_object*>::iterator can =
		candidates.begin();
	    for (++can; can != candidates.end(); ++can) {
		delete *can;
	    }
	    candidates.erase(candidates.begin()+1, candidates.end());
	}
	BCP_add_branching_objects(p, candidates);
	best_presolved = new BCP_presolved_lp_brobj(candidates[0]);
	best_presolved->initialize_lower_bound(p.node->true_lower_bound);
    } else {
	selected = BCP_lp_perform_strong_branching(p, candidates,
						   best_presolved);
    }

    const BCP_lp_branching_object* can = best_presolved->candidate();

    // decide what to do with each child
    p.user->set_actions_for_children(best_presolved);
    BCP_vec<BCP_child_action>& action = best_presolved->action();
    // override the set values if we won't dive
    if (p.node->dive == BCP_DoNotDive){
	bool needed_overriding = false;
	for (int i = can->child_num - 1; i >= 0; --i) {
	    if (action[i] == BCP_KeepChild) {
		action[i] = BCP_ReturnChild;
		needed_overriding = true;
	    }
	}
	if (needed_overriding &&
	    p.param(BCP_lp_par::LpVerb_StrongBranchResult)){
	    printf("LP:   Every child is returned because of not diving.\n");
	}
    }

    // We don't know what is fathomable! strong branching may give an approx
    // sol for the children's subproblems, but it may not be a lower
    // bound. Let the tree manager decide what to do with them.

    // now throw out the fathomable ones. This can be done only if nothing
    // needs to be priced, there already is an upper bound and strong branching
    // was enabled (otherwise we don't have the LPs solved)
    if (p.param(BCP_lp_par::MaxPresolveIter) >= 0) {
	BCP_print_brobj_stat(p, orig_colnum, candidates.size(), selected,
			     best_presolved);
    }

    // finally get the user data for the children
    p.user->set_user_data_for_children(best_presolved, selected);
   
    // Now just resolve the LP to get what'll be sent to the TM.
    p.user->modify_lp_parameters(p.lp_solver, false);
    p.lp_solver->initialSolve();
    p.lp_result->get_results(*p.lp_solver);
    p.node->quality = p.lp_result->objval();

    p.stat.time_branching += CoinCpuTime() - time0;

    return BCP_DoBranch;
}

//#############################################################################

static inline void
BCP_lp_make_parent_from_node(BCP_lp_prob& p)
{
    BCP_lp_parent& parent = *p.parent;
    BCP_lp_node& node = *p.node;

    const int bvarnum = p.core->varnum();
    const int bcutnum = p.core->cutnum();

    // deal with parent's fields one-by-one

    // core_as_change has already been updated correctly in BCP_lp_pack_core()
    node.tm_storage.core_change = (bvarnum + bcutnum > 0 ?
				   BCP_Storage_WrtParent : BCP_Storage_NoData);
    int i;

    const BCP_var_set& vars = node.vars;
    const int varnum = vars.size();
    BCP_obj_set_change var_set = parent.var_set;
    BCP_vec<int>& var_ind = var_set._new_objs;
    BCP_vec<BCP_obj_change>& var_ch = var_set._change;
    var_ind.clear();
    var_ch.clear();
    var_ind.reserve(varnum - bvarnum);
    var_ch.reserve(varnum - bvarnum);
    for (i = bvarnum; i < varnum; ++i) {
	const BCP_var* var = vars[i];
	var_ind.unchecked_push_back(var->bcpind());
	var_ch.unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
						  var->status()));
    }
    node.tm_storage.var_change = BCP_Storage_WrtParent;

    const BCP_cut_set& cuts = node.cuts;
    const int cutnum = cuts.size();
    BCP_obj_set_change cut_set = parent.cut_set;
    BCP_vec<int>& cut_ind = cut_set._new_objs;
    BCP_vec<BCP_obj_change>& cut_ch = cut_set._change;
    cut_ind.clear();
    cut_ch.clear();
    cut_ind.reserve(cutnum - bcutnum);
    cut_ch.reserve(cutnum - bcutnum);
    for (i = bcutnum; i < cutnum; ++i) {
	const BCP_cut* cut = cuts[i];
	cut_ind.unchecked_push_back(cut->bcpind());
	cut_ch.unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
						  cut->status()));
    }
    node.tm_storage.cut_change = BCP_Storage_WrtParent;

    delete parent.warmstart;
    parent.warmstart = node.warmstart;
    node.warmstart = 0;
    node.tm_storage.warmstart = BCP_Storage_WrtParent;

    parent.index = node.index;

    delete node.user_data;
    node.user_data = 0;
}

//#############################################################################

BCP_branching_result
BCP_lp_branch(BCP_lp_prob& p)
{
    BCP_presolved_lp_brobj* best_presolved = 0;

    // this is needed to stop gcc complaining about uninitialized use of
    // do_branch (this complaint might arise from inlining)
    BCP_branching_decision do_branch = BCP_DoBranch;
    do_branch = BCP_lp_select_branching_object(p, best_presolved);

    switch (do_branch){
    case BCP_DoNotBranch_Fathomed:
	BCP_lp_send_cuts_to_cp(p, -1);
	BCP_lp_perform_fathom(p,"LP:   Forcibly fathoming node in branch().\n",
			      BCP_Msg_NodeDescription_Discarded);
	return BCP_BranchingFathomedThisNode;
    case BCP_DoNotBranch:
	BCP_lp_send_cuts_to_cp(p,
			       p.param(BCP_lp_par::CutEffectiveCountBeforePool));
	// the branching objects are already added and we will add from the
	// local pools as well.
	return BCP_BranchingContinueThisNode;
	break;
    case BCP_DoBranch:
	break;
    }

    // Now p.node has the final set of vars/cuts for this node, and this
    // routine will extract the final warmstart information. Send this all off
    // to the the TM. This function also sends off the branching info, and gets
    // diving info from TM. In case of diving it receives/updates the index of
    // the node and the internal indices of the not yet indexed vars/cuts.
    int keep = BCP_lp_send_node_description(p, best_presolved,
					    BCP_Msg_NoMessage);

    // send out the cuts to be sent to the CP
    BCP_lp_send_cuts_to_cp(p, -1);

    if (keep < 0){ // if no diving then return quickly
	if (p.param(BCP_lp_par::LpVerb_FathomInfo)) {
	    if (best_presolved->is_pruned())
		printf("LP:   Forcibly Pruning node\n");
	    else
		printf("LP:   Returned children to TM. Waiting for new node.\n");
	}
	delete best_presolved->candidate();
	delete best_presolved;
	BCP_lp_clean_up_node(p);
	return BCP_BranchingFathomedThisNode;
    }

    // Otherwise we dive. start updating things.
    // first move the current node to be the parent
    BCP_lp_make_parent_from_node(p);

    // now apply the bounds of the kept child
    BCP_lp_branching_object* can = best_presolved->candidate();
    can->apply_child_bd(p.lp_solver, keep);
    if (can->vars_affected()) {
	BCP_var_set& vars = p.node->vars;
	if (can->forced_var_pos) {
	    vars.set_lb_ub(*can->forced_var_pos, can->forced_var_bd_child(keep));
	    const BCP_vec<int>& pos = *can->forced_var_pos;
	    for (int p = pos.size() - 1; p >= 0; --p) {
		vars[pos[p]]->make_non_removable();
	    }
	}
	if (can->implied_var_pos)
	    vars.set_lb_ub(*can->implied_var_pos,can->implied_var_bd_child(keep));
    }
    if (can->cuts_affected()) {
	BCP_cut_set& cuts = p.node->cuts;
	if (can->forced_cut_pos) {
	    cuts.set_lb_ub(*can->forced_cut_pos, can->forced_cut_bd_child(keep));
	    const BCP_vec<int>& pos = *can->forced_cut_pos;
	    for (int p = pos.size() - 1; p >= 0; --p) {
		cuts[pos[p]]->make_non_removable();
	    }
	}
	if (can->implied_cut_pos)
	    cuts.set_lb_ub(*can->implied_cut_pos,can->implied_cut_bd_child(keep));
    }
    // Delete the old user data before getting new
    delete p.node->user_data;
    p.node->user_data = best_presolved->user_data()[keep];
    best_presolved->user_data()[keep] = 0;

    // Get rid of best_presolved
    delete best_presolved->candidate();
    delete best_presolved;

    return BCP_BranchingDivedIntoNewNode;
}
