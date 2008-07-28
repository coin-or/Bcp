// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <numeric>

#include "CoinHelperFunctions.hpp"
#include "CoinSort.hpp"

#include "BCP_matrix.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_functions.hpp"

//#############################################################################

void
BCP_lp_perform_fathom(BCP_lp_prob& p, const char* msg, BCP_message_tag msgtag)
{
    p.user->print(p.param(BCP_lp_par::LpVerb_FathomInfo), "%s", msg);
    // Here we don't have col/row_indices to compress, we are from fathom and
    // we do want to force deletion.
    if (p.param(BCP_lp_par::SendFathomedNodeDesc)) {
	BCP_lp_delete_cols_and_rows(p, 0, 0, 0, true, true);
    }
    BCP_lp_send_node_description(p, 0, msgtag);
    BCP_lp_clean_up_node(p);
}

//#############################################################################

// the primal is infeas when this function is called. Still, we must first try
// to achive TDF, before trying to restore feasibility.

bool BCP_lp_fathom(BCP_lp_prob& p, const bool from_repricing)
{
    BCP_lp_result& lpres = *p.lp_result;

    int i, j;
    int added_size = 0;
    int vars_to_add_size = 0;
    const int max_var = p.param(BCP_lp_par::MaxVarsAddedPerIteration);

    switch (p.node->colgen) {
    case BCP_DoNotGenerateColumns_Fathom:
	BCP_lp_perform_fathom(p, "LP:   Pruning node\n",
			      lpres.termcode() & BCP_ProvenPrimalInf ?
			      BCP_Msg_NodeDescription_Infeas_Pruned :
			      BCP_Msg_NodeDescription_OverUB_Pruned);
	return true;

    case BCP_DoNotGenerateColumns_Send:
	BCP_lp_perform_fathom(p, "LP:   Sending node for next phase\n",
			      lpres.termcode() & BCP_ProvenPrimalInf ?
			      BCP_Msg_NodeDescription_Infeas :
			      BCP_Msg_NodeDescription_OverUB);
	return true;

    case BCP_GenerateColumns:
	BCP_lp_check_ub(p);
	if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
	    printf("LP:   Generating columns before fathoming/resolving\n");
	BCP_vec<BCP_col*> cols_to_add;
	BCP_vec<BCP_var*> vars_to_add;
	if (lpres.termcode() & BCP_ProvenPrimalInf) { //############ infeasible
	    // *FIXME* : change the hardcoded 10 into a parameter
	    std::vector<double*> dual_rays = p.lp_solver->getDualRays(10);
	    if (dual_rays.size() > 0) {
		BCP_restore_feasibility(p, dual_rays, vars_to_add, cols_to_add);
		for (i = dual_rays.size() - 1; i >= 0; --i) {
		    delete[] dual_rays[i];
		}
	    } else {
		throw BCP_fatal_error("\
BCP_lp_fathom(): infeasible but can't get a dual ray!\n");
	    }
	    vars_to_add_size = vars_to_add.size();
	    if (vars_to_add_size == 0) {
		// Nothing helps...
		BCP_lp_perform_fathom(p, "\
LP:   Fathoming node (discovered not restorable inf.)\n",
				      BCP_Msg_NodeDescription_Infeas_Pruned);
		return true;
	    } else {
		// Great, we can fix infeasibility:
		for (i = 0; i < vars_to_add_size; ++i) {
		    vars_to_add[i]->set_bcpind(-BCP_lp_next_var_index(p));
		}
		BCP_lp_add_cols_to_lp(cols_to_add, p.lp_solver);
		purge_ptr_vector(cols_to_add);
		p.node->vars.append(vars_to_add);
		p.local_cut_pool->rows_are_valid(false);
		if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
		    printf("LP:   %i variables added while restoring feasibility\n",
			   static_cast<int>(vars_to_add.size()));
		// need not delete the entries in vars_to_add one-by-one; those
		// pointers are appended to p.node->variables
		// Here we don't have col/row_indices to compress, we say we
		// are not from fathom ('cos we do add columns, i.e., we are
		// not going to fathom the node after the call returns) and we
		// don't want to force deletion.
		BCP_lp_delete_cols_and_rows(p, 0, 0, 0, false, false);
		return false;
	    }
	} else { //########################################### over upper bound
	    BCP_price_vars(p, true /*from fathom*/, vars_to_add, cols_to_add);
	    if (vars_to_add.size() == 0) {
		// we can fathom!
		BCP_lp_perform_fathom(p, "\
LP:   Fathoming node (discovered tdf & high cost)\n",
				      BCP_Msg_NodeDescription_OverUB_Pruned);
		return true;
	    }
	    // keep only the best so many to add
	    if (max_var < vars_to_add_size) {
		// reorder the generated variables (and the corresponding
		// columns) based on the reduced costs of the columns
		const double * duals = p.lp_result->pi();
		BCP_vec<double> rc(vars_to_add_size, 0.0);
		for (i = 0; i < vars_to_add_size; ++i) {
		    rc[i] = (cols_to_add[i]->Objective() -
			     cols_to_add[i]->dotProduct(duals));
		}
		BCP_vec<int> perm;
		perm.reserve(vars_to_add_size);
		for (i = 0; i < vars_to_add_size; ++i)
		    perm.unchecked_push_back(i);
		CoinSort_2(rc.begin(), rc.end(), perm.begin());
		const double rc_cutoff = rc[max_var];
		CoinSort_2(perm.begin(), perm.end(), rc.begin());
		for (i = 0, j = 0; i < vars_to_add_size; ++i) {
		    if (rc[i] <= rc_cutoff) {
			perm[j++] = i;
		    }
		}
		perm.erase(perm.entry(j), perm.end());
		// those in perm are to be kept
		keep_ptr_vector_by_index(vars_to_add, perm.begin(),perm.end());
		keep_ptr_vector_by_index(cols_to_add, perm.begin(),perm.end());
		// cols_to_add.keep_by_index(perm); // this was wrong
	    }

	    // Just add the given colums and go back to resolve
	    added_size = vars_to_add.size();
	    for (i = 0; i < added_size; ++i){
		vars_to_add[i]->set_bcpind(-BCP_lp_next_var_index(p));
	    }
	    BCP_lp_add_cols_to_lp(cols_to_add, p.lp_solver);
	    purge_ptr_vector(cols_to_add);
	    p.node->vars.append(vars_to_add);
	    p.local_cut_pool->rows_are_valid(false);
	    if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
		printf("LP:   %i variables added in price-out (not TDF :-( )\n",
		       static_cast<int>(vars_to_add.size()));
	    // need not delete the entries in vars_to_add one-by-one; those
	    // pointers are appended to p.node->variables.
	    // Here we don't have col/row_indices to compress, we say we are
	    // not from fathom ('cos we do add columns, i.e., we are not going
	    // to fathom the node after the call returns) and we don't want
	    // to force deletion.
	    BCP_lp_delete_cols_and_rows(p, 0, 0, 0, false, false);
	    return false;
	}
	break;
    }

    return true; // fake return
}

//#############################################################################

void
BCP_price_vars(BCP_lp_prob& p, const bool from_fathom,
	       BCP_vec<BCP_var*>& vars_to_add, BCP_vec<BCP_col*>& cols_to_add)
{
    const BCP_lp_result& lpres = *p.lp_result;

    bool generated_algo_var = false;
    const size_t to_add = vars_to_add.size();
    if (p.user_has_lp_result_processing) {
	vars_to_add.append(p.new_vars);
	cols_to_add.append(p.new_cols);
	p.new_vars.clear();
	p.new_cols.clear();
    } else {
	p.user->generate_vars_in_lp(lpres, p.node->vars, p.node->cuts,
				    from_fathom, vars_to_add, cols_to_add);
    }
    if (vars_to_add.size() > to_add) {
	generated_algo_var = true;
	if (cols_to_add.size() > to_add) {
	    if (cols_to_add.size() !=  vars_to_add.size()) {
		throw BCP_fatal_error("\
LP: uneven new_vars/new_cols sizes in BCP_price_vars().\n");
	    }
	} else {
	    // expand the generated vars
	    BCP_vec<BCP_var*> new_vars(vars_to_add.begin() + to_add,
				       vars_to_add.end());
	    BCP_vec<BCP_col*> new_cols;
	    p.user->vars_to_cols(p.node->cuts, new_vars, new_cols,
				 lpres, BCP_Object_FromGenerator, false);
	    cols_to_add.insert(cols_to_add.end(),
			       new_cols.begin(), new_cols.end());
	}
    }
}

//#############################################################################

void
BCP_restore_feasibility(BCP_lp_prob& p,
			const std::vector<double*> dual_rays,
			BCP_vec<BCP_var*>& vars_to_add,
			BCP_vec<BCP_col*>& cols_to_add)
{
    // Now try to restore feasibility with algo vars
    // now we want to pass only the uncut dual rays, so collect them
    const size_t to_add = vars_to_add.size();
    p.user->restore_feasibility(*p.lp_result, dual_rays,
				p.node->vars, p.node->cuts,
				vars_to_add, cols_to_add);
    if (vars_to_add.size() > to_add) {
	if (cols_to_add.size() > to_add) {
	    if (cols_to_add.size() !=  vars_to_add.size()) {
		throw BCP_fatal_error("\
LP: uneven new_vars/new_cols sizes in BCP_restore_feasibility().\n");
	    }
	} else {
	    // expand the generated vars
	    BCP_vec<BCP_var*> new_vars(vars_to_add.begin() + to_add,
				       vars_to_add.end());
	    BCP_vec<BCP_col*> new_cols;
	    p.user->vars_to_cols(p.node->cuts, new_vars, new_cols,
				 *p.lp_result, BCP_Object_FromGenerator,
				 false);
	    cols_to_add.insert(cols_to_add.end(),
			       new_cols.begin(), new_cols.end());
	}
    }
}
