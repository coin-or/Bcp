// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <numeric>

#include "CoinHelperFunctions.hpp"

#include "BCP_simul_sort.hpp"
#include "BCP_temporary.hpp"
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

void
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

static BCP_dual_status
BCP_price_vars(BCP_lp_prob& p,
	       BCP_vec<BCP_var*>& vars_to_add,
	       BCP_vec<BCP_col*>& cols_to_add);

static void
BCP_restore_feasibility(BCP_lp_prob& p,
			const std::vector<double*> dual_rays,
			BCP_vec<BCP_var*>& vars_to_add,
			BCP_vec<BCP_col*>& cols_to_add);

//#############################################################################

// the primal must be infeas or OverUB.

bool BCP_lp_fathom(BCP_lp_prob& p, const bool from_repricing)
{
   if (p.node->colgen == BCP_DoNotGenerateColumns_Fathom) {
      if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	 printf("LP:   Pruning node\n");
      // Here we don't have col/row_indices to compress and we do want to
      // force deletion.
      if (p.param(BCP_lp_par::FixVarsBeforeFathom))
	 BCP_lp_fix_vars(p,
			 true /* from fathom */,
			 false /* ??? *THINK*: this is safe, but maybe
				  true would be OK, too ??? */
			      );
      BCP_lp_delete_cols_and_rows(p, 0, true);
      BCP_lp_send_node_description(p, 0, BCP_Msg_NodeDescription_Discarded);
      return true;
   }

   BCP_lp_result& lpres = *p.lp_result;

   if (p.param(BCP_lp_par::FixVarsBeforeFathom))
      BCP_lp_fix_vars(p,
		      true /* from fathom */,
		      false /* ??? *THINK*: this is safe, but maybe true
			     * would be OK, too ??? */
		      );

   int i, j;
   int added_size = 0;
   int vars_to_add_size = 0;
   const int max_var = p.param(BCP_lp_par::MaxVarsAddedPerIteration);

   switch (p.node->colgen){
    case BCP_DoNotGenerateColumns_Fathom:
      if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	 printf("LP:   Pruning node\n");
      // Here we don't have col/row_indices to compress and we do want to
      // force deletion.
      BCP_lp_delete_cols_and_rows(p, 0, true);
      BCP_lp_send_node_description(p, 0, BCP_Msg_NodeDescription_Discarded);
      return true;

    case BCP_DoNotGenerateColumns_Send:
      if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	 printf("LP:   Sending node for next phase\n");
      // Here we don't have col/row_indices to compress and we do want to
      // force deletion.
      BCP_lp_delete_cols_and_rows(p, 0, true);
      BCP_lp_send_node_description(p, 0,
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
      // allocate space for 64K entries, so that push back will be cheap
      // later on.
      vars_to_add.reserve((1<<16) - 256); 
      BCP_dual_status dst = BCP_price_vars(p, vars_to_add, cols_to_add);
      vars_to_add_size = vars_to_add.size();

      switch (dst){
       case BCP_NotTotalDualFeasible:
	 // keep only the best so many to add
	 if (max_var < vars_to_add_size) {
	    // reorder the generated variables (and the corresponding columns)
	    // based on the reduced costs of the columns
	    const double * duals = p.lp_result->pi();
	    BCP_temp_vec<double> tmp_rc(vars_to_add_size, 0.0);
	    BCP_vec<double>& rc = tmp_rc.vec();
	    for (i = 0; i < vars_to_add_size; ++i) {
	       rc[i] = (cols_to_add[i]->Objective() -
			cols_to_add[i]->dotProduct(duals));
	    }
	    BCP_temp_vec<int> tmp_perm(vars_to_add_size); //reserve, not fill!
	    BCP_vec<int>& perm = tmp_perm.vec();
	    for (i = 0; i < vars_to_add_size; ++i)
	       perm.unchecked_push_back(i);
	    BCP_simul_sort(rc.begin(), rc.end(), perm.begin());
	    const double rc_cutoff = rc[max_var];
	    BCP_simul_sort(perm.begin(), perm.end(), rc.begin());
	    for (i = 0, j = 0; i < vars_to_add_size; ++i) {
	       if (rc[i] <= rc_cutoff) {
		  perm[j++] = i;
	       }
	    }
	    perm.erase(perm.entry(j), perm.end());
	    // those in perm are to be kept
	    keep_ptr_vector_by_index(vars_to_add, perm.begin(), perm.end());
	    cols_to_add.keep_by_index(perm);
	 }

	 // Just add the given colums and go back to resolve
	 added_size = vars_to_add.size();
	 for (i = 0; i < added_size; ++i){
	    vars_to_add[i]->set_bcpind(-BCP_lp_next_var_index(p));
	 }
	 BCP_lp_add_cols_to_lp(cols_to_add, p.lp_solver);
	 p.node->vars.append(vars_to_add);
	 p.local_cut_pool->rows_are_valid(false);
	 if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
	    printf("LP:   %i variables added in price-out (not TDF :-( )\n",
		   static_cast<int>(vars_to_add.size()));
	 // need not delete the entries in vars_to_add one-by-one; those
	 // pointers are appended to p.node->variables.
	 // Here we don't have col/row_indices to compress and we don't want
	 // to force deletion.
	 BCP_lp_delete_cols_and_rows(p, 0, false);
	 return false;

       case BCP_TotalDualFeasible_HasAllIndexed: // same as TotalDualFeasible
       case BCP_TotalDualFeasible_NotAllIndexed:
	 if (p.over_ub(p.lp_result->objval())) {
	    // doesn't matter how we came (infeas or over the bound) to this
	    // function, if we have TDF we can fathom the node
	    if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	       printf("LP:   Fathoming node (discovered tdf & high cost)\n");
	    // Here we don't have col/row_indices to compress and we do want
	    // to force deletion.
	    BCP_lp_delete_cols_and_rows(p, 0, true);
	    BCP_lp_send_node_description
	       (p, 0, BCP_Msg_NodeDescription_OverUB_Pruned);
	    // before returnning, we must clean vars_to_add
	    purge_ptr_vector(vars_to_add);
	    return true;
	 }

	 // At this point we must have TDF and an infeas LP.
	 
	 if (dst == BCP_TotalDualFeasible_HasAllIndexed &&
	     vars_to_add.size() != 0){
	    // We got something in vars_to_add and we have HasAll. Just add
	    // these to the problem, and go back resolving. They might fix
	    // infeasibility. If they don't, we'll be back here in no time,
	    // and then vars_to_add will be empty, making things simpler.

	    // Since we we'll add every index that might ever need pricing, we
	    // can clear the indexed_to_price list
	    const int st =
	      p.node->indexed_pricing.get_status() & BCP_PriceAlgoVars;
	    p.node->indexed_pricing.
	      set_status(static_cast<BCP_pricing_status>(st));
	    added_size = vars_to_add.size();
	    for (int i = 0; i < added_size; ++i) {
	       vars_to_add[i]->set_bcpind(-BCP_lp_next_var_index(p));
	    }
	    BCP_lp_add_cols_to_lp(cols_to_add, p.lp_solver);
	    p.node->vars.append(vars_to_add);
	    p.local_cut_pool->rows_are_valid(false);
	    if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
	       printf("\
LP:   tdf_has_all_indexed achieved by adding %i variables. resolving.\n",
		      static_cast<int>(vars_to_add.size()));
	    // need not delete the entries in vars_to_add one-by-one; those
	    // pointers are appended to p.node->variables.
	    // Here we don't have col/row_indices to compress and we don't
	    // want to force deletion.
	    BCP_lp_delete_cols_and_rows(p, 0, false);
	    return false;
	 }

	 // OK. Now we either have:
	 //   - NotAllIndexed   or
	 //   - HasAllIndexed AND an empty vars_to_add, in which case
	 //     p.node->non_fixables must be empty, too.
	 // In either case, we can just go ahead and call restore_feasibility.
	 // If the default is used, then restore_feasibility will
	 // be needed in the first case and will return in no time in the
	 // second case. So we are OK.
	 // *FIXME* : change the hardcoded 10 into a parameter
	 std::vector<double*> dual_rays = p.lp_solver->getDualRays(10);
	 if (dual_rays.size() > 0) {
	   BCP_restore_feasibility(p, dual_rays, vars_to_add, cols_to_add);
	   for (i = dual_rays.size() - 1; i >= 0; --i) {
	     delete[] dual_rays[i];
	   }
	 }

	 vars_to_add_size = vars_to_add.size();
	 if (vars_to_add_size == 0){
	    // Nothing helps...
	    if (p.param(BCP_lp_par::LpVerb_FathomInfo))
	       printf("LP:   Fathoming node (tdf & not restorable inf.)\n");
	    // Here we don't have col/row_indices to compress and we do
	    // want to force deletion.
	    BCP_lp_delete_cols_and_rows(p, 0, true);
	    BCP_lp_send_node_description
	       (p, 0, BCP_Msg_NodeDescription_Infeas_Pruned);
	    // need not delete the entries in vars_to_add one-by-one; it's
	    // empty
	    return true;
	 }else{
	    // Great, we can fix infeasibility:
	    for (i = 0; i < vars_to_add_size; ++i) {
	       vars_to_add[i]->set_bcpind(-BCP_lp_next_var_index(p));
	    }
	    BCP_lp_add_cols_to_lp(cols_to_add, p.lp_solver);
	    p.node->vars.append(vars_to_add);
	    p.local_cut_pool->rows_are_valid(false);
	    if (p.param(BCP_lp_par::LpVerb_ColumnGenerationInfo))
	       printf("LP:   %i variables added while restoring feasibility\n",
		      static_cast<int>(vars_to_add.size()));
	    // need not delete the entries in vars_to_add one-by-one; those
	    // pointers are appended to p.node->variables
	    // Here we don't have col/row_indices to compress and we don't
	    // want to force deletion.
	    BCP_lp_delete_cols_and_rows(p, 0, false);
	    return false;
	 }
      }
   }
   return true; // fake return
}

//#############################################################################

BCP_dual_status
BCP_price_vars(BCP_lp_prob& p,
	       BCP_vec<BCP_var*>& vars_to_add,
	       BCP_vec<BCP_col*>& cols_to_add)
{
  BCP_dual_status dst_indexed = BCP_TotalDualFeasible_HasAllIndexed;
  // a place to store a candidate list that will be the new list of indexed
  // vars to be priced in case of all the algo vars do price out (assuming
  // of course that we do price indexed vars).
  BCP_vec<int> new_index_list;
  // assume that the final pricing status of the indexed vars is going to be
  // this (assuming of course that we do price indexed vars).
  BCP_pricing_status assumed_pr_status = BCP_PriceUntilLastIndexedToPrice;

  if (p.param(BCP_lp_par::MaintainIndexedVarPricingList)) {
    const BCP_lp_result& lpres = *p.lp_result;
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
	p.upper_bound - lpres.objval() - p.param(BCP_lp_par::Granularity);

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
	std::max(static_cast<const size_t>
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

      const double* rowprice = p.lp_solver->getRowPrice();

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
	}
	if (new_index == -1) // nothing is left to check
	  break;

	prev_index = new_index;
	delete new_indexed_var;
	new_col.clear();
	new_indexed_var = p.user->create_indexed_var(new_index, cuts, new_col);
	// compute the reduced cost
	red_cost = new_col.Objective() - new_col.dotProduct(rowprice);
	if (red_cost < minus_etol){
	  // dual infeasible col
	  if (dst_indexed != BCP_NotTotalDualFeasible){
	    dst_indexed = BCP_NotTotalDualFeasible;
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
	} else if (dst_indexed != BCP_NotTotalDualFeasible && red_cost < gap) {
	  // a new indexed to price and we are still interested in them
	  if (new_index_list.size() < indexed_store){
	    new_index_list.unchecked_push_back(new_indexed_var->index());
	  } else {
	    assumed_pr_status = BCP_PriceAfterLastIndexedToPrice;
	  }
	  if (max_new_vars > 0){
	    --max_new_vars;
	    vars_to_add.push_back(new_indexed_var);
	    cols_to_add.push_back(new BCP_col(new_col));
	    new_indexed_var = 0;
	  } else { // run out of space; we can't add all indexed to price
	    dst_indexed = BCP_TotalDualFeasible_NotAllIndexed;
	  }
	}
      }
      delete new_indexed_var;
      new_indexed_var = 0;
    }
  }

  BCP_dual_status dst_algo = BCP_TotalDualFeasible;
  if (p.node->indexed_pricing.get_status() & BCP_PriceAlgoVars) {
    const size_t to_add = vars_to_add.size();
    p.user->generate_vars_in_lp(*p.lp_result, p.node->vars, p.node->cuts,
				true /* to be fathomed */,
				vars_to_add, cols_to_add);
    if (vars_to_add.size() > to_add)
      dst_algo = BCP_NotTotalDualFeasible;
  }
  
  if (dst_algo == BCP_TotalDualFeasible) {
    if (dst_indexed != BCP_NotTotalDualFeasible) {
      // Now we really have TDF. Update the non-fixable list if we price
      // indexed vars
      if (p.node->indexed_pricing.get_status() & BCP_PriceIndexedVars) {
	p.node->indexed_pricing.set_indices(new_index_list);
	p.node->indexed_pricing.set_status(assumed_pr_status);
      }
    }
  }

  switch (dst_indexed){
  case BCP_TotalDualFeasible_HasAllIndexed:
    return dst_algo == BCP_TotalDualFeasible ?
      BCP_TotalDualFeasible_HasAllIndexed : BCP_NotTotalDualFeasible;;
  case BCP_TotalDualFeasible_NotAllIndexed:
    return dst_algo == BCP_TotalDualFeasible ?
      BCP_TotalDualFeasible_NotAllIndexed : BCP_NotTotalDualFeasible;
  case BCP_NotTotalDualFeasible:
    return BCP_NotTotalDualFeasible;
  }

  return BCP_TotalDualFeasible; // fake return
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

    // we got to cut all dual rays, so we keep a bool array indicating which
    // ones have been cut

    if (p.node->indexed_pricing.get_status() & BCP_PriceIndexedVars) {
      // This routine is invoked only if everything has positive red cost.
      // In that case, the indexed vars that can't be fixed (at least some of
      // them) would be listed in vars_to_add. So if vars_to_add is empty, no
      // indexed var can fix the infeasibility without driving the
      // objective too high.
      if (vars_to_add.size() > 0) {
	// First check if those in vars_to_add can restore feasibility
	BCP_temp_vec<int> tmp_res;
	BCP_vec<int>& res = tmp_res.vec();
	for (int i = cols_to_add.size() - 1; i >= 0 && rays_uncut > 0; --i) {
	  for (int j = 0; j < numrays && rays_uncut > 0; ++j) {
	    if (cols_to_add[i]->dotProduct(dual_rays[j]) < 0) {
	      if (!rays_cut[j]) {
		res.push_back(i);
		rays_cut[j] = true;
		--rays_uncut;
	      }
	    }
	  }
	}
	if (res.size() != 0) {
	  // the cols in res help (a bit) , purge those NOT in res
	  keep_ptr_vector_by_index(vars_to_add, res.begin(), res.end());
	  keep_ptr_vector_by_index(cols_to_add, res.begin(), res.end());
	} else {
	  purge_ptr_vector(vars_to_add);
	  purge_ptr_vector(cols_to_add);
	}

	if (rays_uncut > 0) {
	  // Oh well... check the indexed vars
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
		// if no other index can be checked then just get out
		break;
	      // otherwise get the real next
	      new_index = p.user->next_indexed_var(prev_index);
	      if (pnext_var != indexed_vars.end() &&
		  new_index == (*pnext_var)->index()) {
		// if the real next is already in the problem then just skip
		// it.
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
		  vars_to_add.push_back(new_indexed_var);
		  cols_to_add.push_back(new BCP_col(new_col));
		  rays_cut[j] = true;
		  --rays_uncut;
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
  }

  if (p.node->indexed_pricing.get_status() & BCP_PriceAlgoVars) {
    // now we want to pass only the uncut dual rays, so collect them
    std::vector<double*> dual_rays_uncut;
    for (int i = 0; i < numrays; ++i) {
      if (!rays_cut[i]) {
	dual_rays_uncut.push_back(dual_rays[i]);
      }
    }
    p.user->restore_feasibility(*p.lp_result, dual_rays_uncut,
				p.node->vars, p.node->cuts,
				vars_to_add, cols_to_add);
  }
}
