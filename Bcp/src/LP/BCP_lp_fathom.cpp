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

static inline int BCP_compare_var_index(const BCP_var_indexed* const var0,
					const BCP_var_indexed* const var1)
{
    return var0->index() < var1->index();
}

//#############################################################################

static void
BCP_collect_indexed_vars(BCP_vec<BCP_var*>::const_iterator first_var,
			 BCP_vec<BCP_var*>::const_iterator last_var,
			 BCP_vec<const BCP_var_indexed*>& indexed_vars)
{
    // clean this list to start with. do NOT delete the objects the pointers
    // point to; they exist in p.vars
    indexed_vars.clear();
    indexed_vars.reserve(last_var - first_var);
    // Get the list of indexed vars and order them
    for ( ; first_var != last_var; ++first_var)
	if ((*first_var)->obj_type() == BCP_IndexedObj)
	    indexed_vars.push_back(dynamic_cast<BCP_var_indexed*>(*first_var));
    if (indexed_vars.size() > 1)
	std::sort(indexed_vars.begin(), indexed_vars.end(),
		  BCP_compare_var_index);
}

//#############################################################################

void
BCP_lp_perform_fathom(BCP_lp_prob& p, const char* msg, BCP_message_tag msgtag)
{
    if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	printf("%s", msg);
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
// *THINK*: Maybe we don't have to? Rethink the indexed vars, too...

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
	if (lpres.termcode() & BCP_ProvenPrimalInf) { //############## infeasible
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
		// Here we don't have col/row_indices to compress, we say we are
		// not from fathom ('cos we do add columns, i.e., we are not going
		// to fathom the node after the call returns) and we don't want to
		// force deletion.
		BCP_lp_delete_cols_and_rows(p, 0, 0, 0, false, false);
		return false;
	    }
	} else { //############################################# over upper bound
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
		keep_ptr_vector_by_index(vars_to_add, perm.begin(), perm.end());
		keep_ptr_vector_by_index(cols_to_add, perm.begin(), perm.end());
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

    bool generated_indexed_var = false;

    // a place to store a candidate list that will be the new list of indexed
    // vars to be priced in case of all the algo vars do price out (assuming
    // of course that we do price indexed vars).
    BCP_vec<int> new_index_list;
    // assume that the final pricing status of the indexed vars is going to be
    // this (assuming of course that we do price indexed vars).
    BCP_pricing_status assumed_pr_status = BCP_PriceUntilLastIndexedToPrice;

    if (p.param(BCP_lp_par::MaintainIndexedVarPricingList)) {
	const BCP_vec<BCP_var*>& vars = p.node->vars;
	const BCP_vec<BCP_cut*>& cuts = p.node->cuts;

	if (p.node->indexed_pricing.get_status() & BCP_PriceIndexedVars) {
	    // First collect the indexed variables in order
	    BCP_vec<const BCP_var_indexed*> indexed_vars;
	    BCP_collect_indexed_vars(p.node->vars.entry(p.core->varnum()),
				     p.node->vars.end(), indexed_vars);
      
	    // Now iterate through the set of indexed vars filling the gaps between
	    // them by asking the user if there is anything there.

	    // these are for computing reduced costs and testing the reduced costs
	    double red_cost;
	    double detol = 0.0;
	    p.lp_solver->getDblParam(OsiDualTolerance, detol);
	    const double minus_etol = - detol;
	    const double gap =
		p.ub() - lpres.objval() - p.param(BCP_lp_par::Granularity);

	    // compute how many are we willing to add
	    int max_ndf_vars =
		static_cast<int>( vars.size() *
				  p.param(BCP_lp_par::MaxNonDualFeasToAdd_Frac));
	    max_ndf_vars =
		CoinMin( CoinMax(p.param(BCP_lp_par::MaxNonDualFeasToAdd_Min),
				 max_ndf_vars),
			 p.param(BCP_lp_par::MaxNonDualFeasToAdd_Max));

	    int max_indexed_vars =
		static_cast<int>( vars.size() *
				  p.param(BCP_lp_par::MaxIndexedToPriceToAdd_Frac));
	    max_indexed_vars =
		CoinMin( CoinMax(p.param(BCP_lp_par::MaxIndexedToPriceToAdd_Min),
				 max_indexed_vars),
			 p.param(BCP_lp_par::MaxIndexedToPriceToAdd_Max));

	    int max_new_vars = max_indexed_vars;

	    const size_t indexed_store =
		std::max<const size_t>(static_cast<const size_t>
				       (p.param(BCP_lp_par::IndexedToPriceStorageSize)),
				       p.node->indexed_pricing.get_indices().size());

	    new_index_list.reserve(indexed_store);

	    // the old pricing_status
	    BCP_pricing_status old_pr_status = p.node->indexed_pricing.get_status();
      
	    // this will hold the new var to add, its index and its col
	    BCP_var_indexed* new_indexed_var = 0;
	    int new_index;
	    BCP_col new_col;
	    new_col.reserve(cuts.size());

	    // The old list of non-fixables, and an iterator to scan them
	    const BCP_vec<int>& old_index_list =
		p.node->indexed_pricing.get_indices();
	    BCP_vec<int>::const_iterator old_index_list_i = old_index_list.begin();

	    // these vars are for iterating
	    BCP_vec<const BCP_var_indexed*>::const_iterator pnext_var =
		indexed_vars.begin();
	    int prev_index = -1;

	    while (true){
		// get the next variable first
		if (old_index_list_i == old_index_list.end()) {
		    // we have no more *old* indexed_to_price
		    if (old_pr_status == BCP_PriceUntilLastIndexedToPrice)
			// if no other indexed must be checked then just get out
			break;
		    // otherwise get the real next
		    new_index = p.user->next_indexed_var(prev_index);
		    if (pnext_var != indexed_vars.end() &&
			new_index == (*pnext_var)->index()){
			// if the real next is already in the problem then just skip it.
			prev_index = new_index;
			++pnext_var;
			continue;
		    }
		} else { // we have some *old* indexed_to_price.
		    // get the first that's not currently in the problem
		    new_index = *old_index_list_i;
		    while (pnext_var != indexed_vars.end() &&
			   new_index > (*pnext_var)->index()) {
			++pnext_var;
		    }
		    if (pnext_var != indexed_vars.end() &&
			new_index == (*pnext_var)->index()) {
			prev_index = new_index;
			++pnext_var;
			++old_index_list_i;
			continue;
		    }
		    ++old_index_list_i;
		}
		if (new_index == -1) // nothing is left to check
		    break;

		prev_index = new_index;
		delete new_indexed_var;
		new_col.clear();
		new_indexed_var = p.user->create_indexed_var(new_index, cuts, new_col);
		// compute the reduced cost
		red_cost = new_col.Objective() - new_col.dotProduct(lpres.pi());
		if (red_cost < minus_etol){
		    // dual infeasible col
		    if (! generated_indexed_var) {
			generated_indexed_var = true;
			// now we got to get rid of the generated objects
			purge_ptr_vector(vars_to_add);
			purge_ptr_vector(cols_to_add);
			max_new_vars = max_ndf_vars;
		    }
		    if (max_new_vars > 0){
			--max_new_vars;
			vars_to_add.push_back(new_indexed_var);
			cols_to_add.push_back(new BCP_col(new_col));
			new_indexed_var = 0;
		    } else { // we've got enough to add
			break;
		    }
		} else if (! generated_indexed_var && red_cost < gap) {
		    // a new indexed to price and we are still interested in them
		    if (new_index_list.size() < indexed_store){
			new_index_list.unchecked_push_back(new_indexed_var->index());
		    } else {
			assumed_pr_status = BCP_PriceAfterLastIndexedToPrice;
		    }
		}
	    }
	    delete new_indexed_var;
	    new_indexed_var = 0;
	}
    }

    bool generated_algo_var = false;
    if (p.node->indexed_pricing.get_status() & BCP_PriceAlgoVars) {
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
  
    if (p.param(BCP_lp_par::MaintainIndexedVarPricingList) &&
	! generated_algo_var && ! generated_indexed_var &&
	(p.node->indexed_pricing.get_status() & BCP_PriceIndexedVars)) {
	// Now we really have TDF. Update the non-fixable list if we price
	// indexed vars
	p.node->indexed_pricing.set_indices(new_index_list);
	p.node->indexed_pricing.set_status(assumed_pr_status);
    }
}

//#############################################################################

void
BCP_restore_feasibility(BCP_lp_prob& p,
			const std::vector<double*> dual_rays,
			BCP_vec<BCP_var*>& vars_to_add,
			BCP_vec<BCP_col*>& cols_to_add)
{
    const int numrays = dual_rays.size();
    int rays_uncut = numrays;
    BCP_vec<bool> rays_cut(numrays, false);

    if (p.param(BCP_lp_par::MaintainIndexedVarPricingList)) {
	const BCP_vec<BCP_cut*>& cuts = p.node->cuts;

	// we got to cut all dual rays, so we keep a bool array indicating
	// which ones have been cut

	if (p.param(BCP_lp_par::MaintainIndexedVarPricingList)) {
	    if (p.node->indexed_pricing.get_status() & BCP_PriceIndexedVars) {
		// First collect the indexed variables in order
		BCP_vec<const BCP_var_indexed*> indexed_vars;
		BCP_collect_indexed_vars(p.node->vars.entry(p.core->varnum()),
					 p.node->vars.end(), indexed_vars);

		const BCP_vec<int>& old_index_list =
		    p.node->indexed_pricing.get_indices();
		BCP_vec<int>::const_iterator old_index_list_i =
		    old_index_list.begin();

		BCP_vec<const BCP_var_indexed*>::const_iterator pnext_var =
		    indexed_vars.begin();
		int prev_index = -1;

		// the old pricing_status
		BCP_pricing_status old_pr_status =
		    p.node->indexed_pricing.get_status();
	    
		// this will hold the new var to add, its index and its col
		BCP_var_indexed* new_indexed_var = 0;
		int new_index;
		BCP_col new_col;
		new_col.reserve(cuts.size());
	    
		while (true){
		    // get the next variable first
		    if (old_index_list_i == old_index_list.end()) {
			// we have no more *old* indexed_to_price
			if (old_pr_status == BCP_PriceUntilLastIndexedToPrice)
			    // if no other index can be checked then just quit
			    break;
			// otherwise get the real next
			new_index = p.user->next_indexed_var(prev_index);
			if (pnext_var != indexed_vars.end() &&
			    new_index == (*pnext_var)->index()) {
			    // if the real next is already in the problem then
			    // just skip it.
			    prev_index = new_index;
			    ++pnext_var;
			    continue;
			}
		    } else { // we have some *old* indexed_to_price.
			// get the first that's not currently in the problem
			new_index = *old_index_list_i;
			while (pnext_var != indexed_vars.end() &&
			       new_index > (*pnext_var)->index())
			    ++pnext_var;
			if (pnext_var != indexed_vars.end() &&
			    new_index == (*pnext_var)->index()){
			    prev_index = new_index;
			    ++pnext_var;
			    ++old_index_list_i;
			    continue;
			}
			++old_index_list_i;
		    }
		    if (new_index == -1) // nothing is left to check
			break;
	       
		    prev_index = new_index;
		    delete new_indexed_var;
		    new_col.clear();
		    new_indexed_var =
			p.user->create_indexed_var(new_index, cuts, new_col);
		    // test if this col helps
		    for (int j = 0; j < numrays && rays_uncut > 0; ++j) {
			if (new_col.dotProduct(dual_rays[j]) < 0) {
			    if (!rays_cut[j]) {
				rays_cut[j] = true;
				--rays_uncut;
				if (new_indexed_var) {
				    vars_to_add.push_back(new_indexed_var);
				    cols_to_add.push_back(new BCP_col(new_col));
				    new_indexed_var = 0;
				}
			    }
			}
		    }
		    if (rays_uncut == 0)
			break;
		}
		delete new_indexed_var;
	    }
	}
    }

    if (rays_uncut == 0)
	// OK, we have restored feasibility
	return;

    if (p.node->indexed_pricing.get_status() & BCP_PriceAlgoVars) {
	// Now try to restore feasibility with algo vars
	// now we want to pass only the uncut dual rays, so collect them
	std::vector<double*> dual_rays_uncut;
	for (int i = 0; i < numrays; ++i) {
	    if (!rays_cut[i]) {
		dual_rays_uncut.push_back(dual_rays[i]);
	    }
	}
	const size_t to_add = vars_to_add.size();
	p.user->restore_feasibility(*p.lp_result, dual_rays_uncut,
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
}
