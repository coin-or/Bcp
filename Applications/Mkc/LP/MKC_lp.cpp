// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <numeric>
#include <algorithm>

#include "CoinHelperFunctions.hpp"
#include "CoinSort.hpp"

#include "OsiClpSolverInterface.hpp"
#include "OsiVolSolverInterface.hpp"

#include "CoinTime.hpp"
#include "BCP_lp.hpp"

#include "MKC_lp.hpp"
#include "MKC_solution.hpp"
#include "MKC_var.hpp"
#include "MKC_vargen.hpp"
#include "MKC_optim.hpp"

//#############################################################################

double
MKC_heur(const int rownum, const int posnum,
	 const BCP_vec<BCP_var *>& vars,
	 const BCP_vec<double>& posval, BCP_vec<int>& positives);

//#############################################################################
//#############################################################################

void
MKC_lp::unpack_module_data(BCP_buffer& buf)
{
  par.unpack(buf);
  kss.unpack(buf);
  int varnum = 0;
  buf.unpack(varnum);
  if (varnum > 0) {
    input_vars.reserve(varnum);
    int type;
    for (int i = 0; i < varnum; ++i) {
      buf.unpack(type);
      MKC_var* var = dynamic_cast<MKC_var*>(MKC_var_unpack(buf));
      input_vars.unchecked_push_back(var);
    }
  }

  objhist = new double[par.entry(MKC_lp_par::TailoffLength)];
  ks_fixings = new MKC_knapsack_fixing[kss.ks_num];
  for (int i = kss.ks_num - 1; i >= 0; --i) {
    ks_fixings[i].fixing.insert(ks_fixings[i].fixing.end(),
				kss.order_num, 1);
    ks_fixings[i].fixed_entries = new int[kss.ks_list[i].size];
    ks_fixings[i].fixed_entries_ind = new int[kss.ks_list[i].size];
  }
   
  enumerated_ks = new BCP_vec<MKC_var*>[kss.ks_num];
  MKC_enumerate_knapsacks(kss, enumerated_ks,
			  par.entry(MKC_lp_par::MaxEnumeratedSize));

  start_time = CoinCpuTime();
}

//#############################################################################
// Override the initializer so that we can choose between vol and simplex
// at runtime.

OsiSolverInterface *
MKC_lp::initialize_solver_interface()
{
  if (par.entry(MKC_lp_par::UseVolume)) {
    return new OsiVolSolverInterface;
  }

  if (par.entry(MKC_lp_par::UseClp)) {
    OsiClpSolverInterface* clp = new OsiClpSolverInterface;
    return clp;
  }

  return 0; // fake return
}

//#############################################################################
// Opportunity to reset things before optimization
void
MKC_lp::modify_lp_parameters(OsiSolverInterface* lp, const int changeType,
			     bool in_strong_branching)
{
  OsiVolSolverInterface* vollp = dynamic_cast<OsiVolSolverInterface*>(lp);
  if (vollp) {
    VOL_parms& vpar = vollp->volprob()->parm;
    vpar.lambdainit = par.entry(MKC_lp_par::Vol_lambdaInit); 
    vpar.alphainit =  par.entry(MKC_lp_par::Vol_alphaInit);
    vpar.alphamin = par.entry(MKC_lp_par::Vol_alphaMin);
    vpar.alphafactor = par.entry(MKC_lp_par::Vol_alphaFactor);
    vpar.primal_abs_precision = par.entry(MKC_lp_par::Vol_primalAbsPrecision);
    vpar.gap_abs_precision = par.entry(MKC_lp_par::Vol_gapAbsPrecision);
    vpar.gap_rel_precision = par.entry(MKC_lp_par::Vol_gapRelPrecision);
    vpar.granularity = par.entry(MKC_lp_par::Vol_granularity);
    vpar.minimum_rel_ascent = par.entry(MKC_lp_par::Vol_minimumRelAscent);
    vpar.ascent_first_check = par.entry(MKC_lp_par::Vol_ascentFirstCheck);
    vpar.ascent_check_invl = par.entry(MKC_lp_par::Vol_ascentCheckInterval);
    vpar.maxsgriters = par.entry(MKC_lp_par::Vol_maxSubGradientIterations);
    vpar.printflag =  par.entry(MKC_lp_par::Vol_printFlag);
    vpar.printinvl =  par.entry(MKC_lp_par::Vol_printInterval);
    vpar.greentestinvl = par.entry(MKC_lp_par::Vol_greenTestInterval);
    vpar.yellowtestinvl =  par.entry(MKC_lp_par::Vol_yellowTestInterval);
    vpar.redtestinvl = par.entry(MKC_lp_par::Vol_redTestInterval);
    vpar.alphaint =  par.entry(MKC_lp_par::Vol_alphaInt);
  }
  if (in_strong_branching) {
    // *THINK* : we might want to do fewer iterations???
  }
}

//#############################################################################

void
MKC_lp::initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
					const BCP_vec<BCP_cut*>& cuts,
					const BCP_vec<BCP_obj_status>& vstat,
					const BCP_vec<BCP_obj_status>& cstat,
					BCP_vec<int>& var_changed_pos,
					BCP_vec<double>& var_new_bd,
					BCP_vec<int>& cut_changed_pos,
					BCP_vec<double>& cut_new_bd)
{
   // Make things compatible with the branching variables
   int i, j;
   for (i = kss.ks_num - 1; i >= 0; --i) {
      MKC_knapsack_fixing& ksf = ks_fixings[i];
      ksf.fixed_entry_num = 0;
      ksf.fixing.clear();
      ksf.fixing.insert(ksf.fixing.end(), kss.order_num, 1);
      ksf.fixed_cost = 0.0;
      ksf.fixed_weight = 0.0;
   }

   // get the row formulation of the lp
   OsiSolverInterface* solver = getLpProblemPointer()->lp_solver;
   const CoinPackedMatrix* rowMatrix = solver->getMatrixByRow();

   const int * rowstart = rowMatrix->getVectorStarts(); //rowrc
   const int * rowlen = rowMatrix->getVectorLengths();
   const int * colind = rowMatrix->getIndices();         //colrc
   const int * rowcur = 0;
   const int * rowend = 0;

   for (i = vars.size() - 1; i >= 0; --i) {
      const MKC_branching_var* bvar =
	 dynamic_cast<const MKC_branching_var*>(vars[i]);
      if (bvar == 0)
	 continue;
      int this_order = bvar->order_index;
      rowcur = colind + rowstart[kss.ks_num + this_order];
      rowend = rowcur + rowlen[kss.ks_num + this_order];

      const int * this_slab =
	 bvar->slab_list + static_cast<int>(vars[i]->lb());
      int * first_slab = bvar->slab_list;
      int * last_slab = bvar->slab_list + (bvar->slab_num - 1);
      // First fix vars to zero that are manufactured from one of the other
      // slabs and does contain this order
      // (Note that the last entry in slab_list is bogus, that indicates the
      // branch when the order must not be manufactured from any of the
      // slabs.)
      while (rowcur != rowend) {
	 const int icol = *rowcur++;
	 const MKC_var* var = dynamic_cast<const MKC_var*>(vars[icol]);
	 if (std::find(first_slab, last_slab, var->entries[0]) != this_slab) {
	    // it IS from one of the other slabs ==> fix to zero
	    var_changed_pos.push_back(icol);
	 }
      }
      const int ks_ind = *this_slab;
      if (ks_ind != -1) {
	 // this_slab is a real one, no bogus
	 // Fix vars to zero that are manufactured from the same slab, but
	 // does not contain this order
	 rowcur = colind + rowstart[ks_ind];
	 rowend = rowcur + rowlen[ks_ind];
	 this_order += kss.ks_num;
	 while (rowcur != rowend) {
	    const int icol = *rowcur++;
	    const MKC_var* var = dynamic_cast<const MKC_var*>(vars[icol]);
	    if (var->entries + var->entry_num ==
		std::find(var->entries, var->entries+var->entry_num,
			  this_order)) {
	       // is IS from this slab, but the order is not ==> fix to zero
	       var_changed_pos.push_back(icol);
	    }
	 }
	 this_order -= kss.ks_num;
      }

      // now update ks_fixings
      for (int * slab = first_slab; slab != last_slab; ++slab) {
	 ks_fixings[*slab].fixing[this_order] = 0;
      }
      if (ks_ind != -1) {
	 MKC_knapsack_fixing& ksf = ks_fixings[ks_ind];
	 MKC_knapsack& ks = kss.ks_list[ks_ind];
	 int j;
	 for (j = 0; j < ks.size; ++j) {
	    if (ks.entries[j].index == this_order)
	       break;
	 }
	 if (j == ks.size)
	    abort();
	 ksf.fixed_entries[ksf.fixed_entry_num] = j;
	 ksf.fixed_entries_ind[ksf.fixed_entry_num++] = this_order;
	 ksf.fixed_weight += ks.entries[j].weight;
	 ksf.fixed_cost += ks.entries[j].orig_cost;
      }
   }

   // Now go through the KS's and if one has orders assigned to it then fix
   // every var to zero that would be produced from the same slab and has such
   // orders in the var that alltogether there would be more than 2 colors in
   // the slab.
   for (i = kss.ks_num - 1; i >= 0; --i){
      MKC_knapsack_fixing& ksf = ks_fixings[i];
      const int fixed_num = ksf.fixed_entry_num;
      if (fixed_num == 0)
	 continue;

      MKC_knapsack& ks = kss.ks_list[i];
      MKC_knapsack_entry* entries = ks.entries;
      int clr0 = -1;
      int clr1 = -1;
      for (j = 0; j < fixed_num; ++j) {
	 const int ind = ksf.fixed_entries[j];
	 const int k = entries[ind].color;
	 if (clr0 == -1) {
	    clr0 = k;
	    continue;
	 }
	 if (clr0 == k)
	    continue;
	 if (clr1 == -1) {
	    clr1 = k;
	    continue;
	 }
	 if (clr1 == k)
	    continue;
	 abort();
      }
      if (clr1 != -1) {
	 // two colors
	 rowcur = colind + rowstart[i];
	 rowend = rowcur + rowlen[i];
	 while (rowcur != rowend) {
	    const int icol = *rowcur++;
	    const MKC_var* var = dynamic_cast<const MKC_var*>(vars[icol]);
	    if ( (var->color[0] == clr0 && var->color[1] == clr1) ||
		 (var->color[0] == clr1 && var->color[1] == clr0) )
	       continue;
	    var_changed_pos.push_back(icol);
	 }
      } else if (clr0 != -1) {
	 // one color
	 rowcur = colind + rowstart[i];
	 rowend = rowcur + rowlen[i];
	 while (rowcur != rowend) {
	    const int icol = *rowcur++;
	    const MKC_var* var = dynamic_cast<const MKC_var*>(vars[icol]);
	    if (var->color[0] == clr0 || var->color[1] == clr0)
	       continue;
	    var_changed_pos.push_back(icol);
	 }
      }
   }

   // Now go through the KS's and if one has two different colored orders
   // assigned to it then fix every variable to zero that would be produced
   // from the same KS and has a yet another colored order in it.   
   for (i = kss.ks_num - 1; i >= 0; --i){
      MKC_knapsack_fixing& ksf = ks_fixings[i];
      MKC_knapsack& ks = kss.ks_list[i];
      MKC_knapsack_entry* entries = ks.entries;
      const int fixed_num = ksf.fixed_entry_num;
      if (fixed_num == 0)
	 continue;
      int clr0 = -1;
      int clr1 = -1;
      for (j = 0; j < fixed_num; ++j) {
	 const int ind = ksf.fixed_entries[j];
	 const int k = entries[ind].color;
	 if (clr0 == -1) {
	    clr0 = k;
	    continue;
	 }
	 if (clr0 == k)
	    continue;
	 if (clr1 == -1) {
	    clr1 = k;
	    continue;
	 }
	 if (clr1 == k)
	    continue;
	 abort();
      }
      if (clr1 != -1) {
	 // two colors
	 BCP_vec<char> flag(kss.order_num, false);
	 // Go through the orders in the KS and mark those that can be used
	 // (i.e., one of the two colors)
	 for (j = ks.size - 1; j >= 0; --j) {
	    if (entries[j].color == clr0 || entries[j].color == clr1)
	       flag[entries[j].index] = true;
	 }
	 // no go through the cols made from that KS and check that each
	 // order in the col has the right flag up.
	 rowcur = colind + rowstart[i];
	 rowend = rowcur + rowlen[i];
	 while (rowcur != rowend) {
	    const int icol = *rowcur++;
	    const MKC_var* var = dynamic_cast<const MKC_var*>(vars[icol]);
	    const int * var_entries = var->entries;
	    for (j = var->entry_num - 1; j > 0; --j) {
	       if (! flag[var_entries[j] - kss.ks_num]) {
		  var_changed_pos.push_back(icol);
		  break;
	       }
	    }
	 }
      }
   }

   // Get rid of duplicates in var_changed_pos
   std::sort(var_changed_pos.begin(), var_changed_pos.end());
   var_changed_pos.erase(std::unique(var_changed_pos.begin(),
				     var_changed_pos.end()),
			 var_changed_pos.end() );
   var_new_bd.clear();
   var_new_bd.insert(var_new_bd.end(), 2 * var_changed_pos.size(), 0.0);
   printf("MKC: Logical fixing fixed %i vars to 0.\n",
	  static_cast<int>(var_changed_pos.size()));
}

//#############################################################################

double
MKC_lp::compute_lower_bound(const double old_lower_bound,
			    const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts)
{
  purge_ptr_vector(generated_vars);

  generate_vars(lpres, vars, lpres.dualTolerance(), generated_vars);
  if (generated_vars.empty()) {
      return lpres.objval();
  }
  if (! par.entry(MKC_lp_par::ComputeLpLowerBound))
      return old_lower_bound;
  
  double exact_red_cost_array[] = {5.0, 1.0, 0.5, 0.25, 0.1, 0.05, 0.025};
  const int array_size =
      sizeof(exact_red_cost_array) / sizeof(exact_red_cost_array[0]);
  const double exact_red_cost =
      current_index() == 0 ?
      exact_red_cost_array[std::min(array_size-1, current_iteration() / 3)] :
      exact_red_cost_array[array_size-2 + std::min(1, current_iteration()/3)];

  const bool exact_fallback = par.entry(MKC_lp_par::ExactFallbackAtLowerBd);

  double lpobj =
      lpres.objval() +
      MKC_compute_ks_upper_bound(kss, ks_fixings, enumerated_ks,
				 par.entry(MKC_lp_par::MaxEnumeratedSize),
				 lpres.pi(), lpres.dualTolerance(),
				 exact_red_cost,
				 par.entry(MKC_lp_par::PrintBestDj),
				 exact_fallback);

  if (current_index() == 0 && lpobj > best_lb_in_root)
      best_lb_in_root = lpobj;

  printf("\
MKC: Result of lower bounding heuristic for the LP optimum:\n\
MKC:  (the heuristic is based on Dantzig-Wolfe decomposition and\n\
MKC:  checking the enumerated knapsacks, exactly solving smaller\n\
MKC:  other knapsack and solving the LP relaxation for bigger\n\
MKC:  other knapsacks).\n");
  printf("MKC: So the lower bound on the LP optimum: %f.\n", lpobj);
  printf("MKC: Best such lower bound in the root: %f.\n", best_lb_in_root);

  return lpobj;
}

//#############################################################################

BCP_solution*
MKC_lp::test_feasibility(const BCP_lp_result& lp_result,
			 const BCP_vec<BCP_var *>& vars,
			 const BCP_vec<BCP_cut*>& cuts)
{
  MKC_solution* sol = 0;

  // first select the positive vars and order them based on their value
  BCP_vec<int> positives;
  const double* x = lp_result.x();
  const int varnum = vars.size();
  select_positives(x, x + varnum, 0.00001, positives);

  // From this list throw out those that are branching variables
  int posnum = positives.size();
  int i, j;
  for (i = 0, j = 0; i < posnum; ++i) {
    if (dynamic_cast<const MKC_var*>(vars[positives[i]]) != 0)
      positives[j++] = positives[i];
  }
  posnum = j;
  positives.erase(positives.entry(posnum), positives.end());

  if (par.entry(MKC_lp_par::WriteToVarFile)) {
    FILE *outfile = fopen(par.entry(MKC_lp_par::OutputVarFile).c_str(), "a");
    if (! outfile)
      throw BCP_fatal_error("MKC: Can't open OutputVarFile.\n");
    for (int i = 0; i < posnum; ++i) {
      MKC_var* var = dynamic_cast<MKC_var*>(vars[positives[i]]);
      fprintf(outfile, "%4i %4i %3i",
	      var->color[0], var->color[1], var->entry_num);
      for (int j = 0; j < var->entry_num; ++j)
	fprintf(outfile, " %5i", var->entries[j]);
      fprintf(outfile, "\n");
    }
    fprintf(outfile, "-1\n");
    fclose(outfile);
  }

  BCP_vec<double> posval;
  posval.reserve(posnum);
  for (i = 0; i < posnum; ++i)
    posval.unchecked_push_back(x[positives[i]]);
  CoinSort_2(posval.begin(), posval.end(), positives.begin());
  
  // Check if it is feasible
  bool feas = false;
  double obj = DBL_MAX;
  if (posval[0] > .99999) {
    // Great! It is feasible
    obj = lp_result.objval();
    feas = (obj < upper_bound());
  } else {
    // do a heuristic
    obj = MKC_heur(cuts.size(), posnum, vars, posval, positives);
    for (i = 0, j = 0; i < posnum; ++i) {
      if (positives[i] >= 0)
	positives[j++] = positives[i];
    }
    posnum = j;
    positives.erase(positives.entry(posnum), positives.end());
    feas = (obj < upper_bound());
  }

  if (feas) {
    BCP_vec<MKC_var*> mkc_vars;
    mkc_vars.reserve(posnum);
    for (i = 0; i < posnum; ++i) {
      // this cast must succeed, we have thrown out the branching vars
      const MKC_var * var = dynamic_cast<const MKC_var*>(vars[positives[i]]);
      mkc_vars.unchecked_push_back(new MKC_var(*var));
    }
    sol = new MKC_solution(mkc_vars, obj);
  }

  return sol;
}

//#############################################################################

void
MKC_lp::pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol)

{
  const MKC_solution* mkc_sol = dynamic_cast<const MKC_solution*>(sol);
  if (! mkc_sol)
    abort();
  mkc_sol->pack(buf);
}

//#############################################################################
// we've only base constraints, so variable expansion is *really* easy

void
MKC_lp::vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // on what to expand
		     BCP_vec<BCP_var*>& vars,       // what to expand
		     BCP_vec<BCP_col*>& cols,       // the expanded cols
		     // few things that the user can use for lifting vars if
		     // allowed 
		     const BCP_lp_result& lpres,
		     BCP_object_origin origin, bool allow_multiple)
{
  vars_to_cols(cuts, vars, cols);
}

void
MKC_lp::vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // on what to expand
		     BCP_vec<BCP_var*>& vars,       // what to expand
		     BCP_vec<BCP_col*>& cols) // the expanded cols
{
  const int cutnum = cuts.size();
  const int varnum = vars.size();
  if (varnum == 0)
    return;
  cols.reserve(varnum);

  const int* empty_int = 0;
  double* ones = new double[cutnum];
  CoinFillN(ones, cutnum, 1.0);
  
  BCP_col * col = 0;
  for (int i = 0; i < varnum; ++i) {
    const MKC_var* mvar = dynamic_cast<const MKC_var*>(vars[i]);
    if (mvar != 0) {
      col = new BCP_col(mvar->entries, mvar->entries + mvar->entry_num,
			ones, ones + mvar->entry_num, mvar->cost, 0.0, 1.0);
    } else {
      col = new BCP_col(empty_int, empty_int, ones, ones, 0.0, 0.0, 1e40);
    }
    cols.unchecked_push_back(col);
  }
  delete[] ones;
}

//#############################################################################

void
MKC_lp::generate_vars_in_lp(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    const bool before_fathom,
			    BCP_vec<BCP_var*>& new_vars,
			    BCP_vec<BCP_col*>& new_cols)
{
    if (! generated_vars.empty() || ! before_fathom) {
	new_vars.append(generated_vars);
	generated_vars.clear();
	return;
    }

    // must be before fathoming. we need vars with red cost below the
    // negative of (lpobj-ub)/ks_num otherwise we can really fathom.
    const double rc_bound = 
	(lpres.dualTolerance() + (lpres.objval() - upper_bound()))/kss.ks_num;
    generate_vars(lpres, vars, rc_bound, new_vars);
}

//#############################################################################

void
MKC_lp::generate_vars(const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const double rc_bound,
		      BCP_vec<BCP_var*>& new_vars)
{
  const bool for_all_knapsack = par.entry(MKC_lp_par::ParallelVargenForAllKS);
  const bool all_encountered_var = par.entry(MKC_lp_par::AddAllGeneratedVars);
  const bool print_best_dj = par.entry(MKC_lp_par::PrintBestDj);
  const bool exact_fallback = par.entry(MKC_lp_par::ExactFallbackAtVargen);
  const int max_enumerated_size = par.entry(MKC_lp_par::MaxEnumeratedSize);
  const double gap = upper_bound() - best_lb_in_root;
  
  MKC_generate_variables(kss, ks_fixings, enumerated_ks, max_enumerated_size,
			 lpres.pi(), gap, new_vars, rc_bound,
			 for_all_knapsack, all_encountered_var,
			 print_best_dj, exact_fallback,
			 current_index(), current_iteration());
  // also check the input_vars
  const int input_var_num = input_vars.size();
  const size_t oldsize = new_vars.size();
  for (int i = 0; i < input_var_num; ++i) {
    if (input_vars[i]->dj(lpres.pi()) < - rc_bound)
      new_vars.push_back(new MKC_var(*input_vars[i]));
  }
  if (oldsize < new_vars.size()) {
    printf("MKC: Also found %i vars among the input vars.\n",
	   static_cast<int>(new_vars.size() - oldsize));
  }
}

//#############################################################################
// logical fixing means that we go through the variables and if there's an
// MKC_branching_var then we fix vars to 0 that do not satisfy the branching
// rule and also we update ks_fixings.

void
MKC_lp::logical_fixing(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var *>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       const BCP_vec<BCP_obj_status>& var_status,
		       const BCP_vec<BCP_obj_status>& cut_status,
		       const int var_bound_changes_since_logical_fixing,
		       BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd)
{
   if (par.entry(MKC_lp_par::HeurFixByReducedCost)) {
      // sort the variables based on their reduced costs and delete those with
      // high rc.
      int i;
      const int vsize = vars.size();
      BCP_vec<int> ind;
      ind.reserve(vsize);
      for (i = 0; i < vsize; ++i)
	 ind.unchecked_push_back(i);
      BCP_vec<double> dj;
      dj.append(lpres.dj(), lpres.dj() + vsize);
      CoinSort_2(dj.begin(), dj.end(), ind.begin());
      for (i = dj.size() - 1; i >= 0; --i) {
	 if (vars[ind[i]]->is_removable() && vars[ind[i]]->ub() > 0)
	    break;
      }
      if (i >= 0 && dj[i] > 0.02) {
	 const int cutoff_ind =
	    std::distance(dj.begin(),
			  std::lower_bound(dj.begin(), dj.end(), dj[i]/2));
	 const int oldsize = changed_pos.size();
	 changed_pos.reserve(oldsize + dj.size() - cutoff_ind);
	 for (i = dj.size() - 1; i >= cutoff_ind; --i) {
	    if (vars[ind[i]]->is_removable() && vars[ind[i]]->ub() > 0)
	       changed_pos.unchecked_push_back(ind[i]);
	 }
	 std::sort(changed_pos.begin(), changed_pos.end());
	 changed_pos.erase(std::unique(changed_pos.begin(), changed_pos.end()),
			   changed_pos.end() );
	 printf("MKC: Heur rc fixing fixed %i vars to 0.\n",
		static_cast<int>(changed_pos.size() - oldsize));
      }
   }
   
   // create new_bd (fix all to 0)
   new_bd.clear();
   new_bd.insert(new_bd.end(), 2 * changed_pos.size(), 0.0);
}

//#############################################################################
//#############################################################################

static inline bool
MKC_orthog(const int *vec0, const int len0, const int * vec1, const int len1)
{
   int i, j;
   for (i = 0, j = 0; i < len0 && j < len1; ) {
      if (vec0[i] == vec1[j])
	 return false;
      if (vec0[i] < vec1[j])
	 ++i;
      else
	 ++j;
   }
   return true;
}

//#############################################################################

double
MKC_heur(const int rownum, const int posnum,
	 const BCP_vec<BCP_var *>& vars,
	 const BCP_vec<double>& posval, BCP_vec<int>& positives)
{
   int i, j, k;

   BCP_vec<char> covered(rownum, false);
   double obj = 0.0;
   for (i = posnum - 1; i >= 0; --i) {
      // this cast must succeed, we have thrown out the branching vars
      const MKC_var * var = dynamic_cast<const MKC_var*>(vars[positives[i]]);
      const int * var_entries = var->entries;
      if (posval[i] < .51)
	 break;
      obj += var->cost;
      for (j = var->entry_num - 1; j >= 0; --j)
	 covered[var_entries[j]] = true;
   }
   int small_num = i + 1;
   if (small_num == 0)
      return obj;

   BCP_vec<int> smalls;
   smalls.reserve(small_num);
   for (i = 0, k = 0; i < small_num; ++i) {
      const MKC_var * var = dynamic_cast<const MKC_var*>(vars[positives[i]]);
      const int * var_entries = var->entries;
      for (j = var->entry_num - 1; j >= 0; --j)
	 if (covered[var_entries[j]])
	    break;
      if (j < 0)
	 smalls.unchecked_push_back(i);
      else
	 positives[i] = -1;
   }
   small_num = smalls.size();
   if (small_num == 0)
      return obj;

   if (small_num == 1) {
      const MKC_var * var =
	 dynamic_cast<const MKC_var*>(vars[positives[smalls[0]]]);
      obj += var->cost;
      return obj;
   }

   // Enumerate on the remaining ones
   char * orthog = new char[small_num * small_num];
   double * small_objs = new double[small_num];
   for (i = small_num - 1; i >= 0; --i) {
      const MKC_var * var_i =
	 dynamic_cast<const MKC_var*>(vars[positives[smalls[i]]]);
      small_objs[i] = var_i->cost;
      for (j = i - 1; j >= 0; --j) {
	 const MKC_var * var_j =
	    dynamic_cast<const MKC_var*>(vars[positives[smalls[j]]]);
	 orthog[i * small_num + j] = orthog[j * small_num + i] =
	    MKC_orthog(var_i->entries, var_i->entry_num,
		       var_j->entries, var_j->entry_num) ? 1 : 0;
      }
   }

   char * flag = new char[small_num];
   char * best_flag = new char[small_num];
   double small_obj = 0;
   double best_obj = 0;

   for (i = 0; i < small_num; ++i) {
      memset(flag, 1, small_num);
      flag[i] = 2;
      for (k = i+1; k < small_num; ++k)
	 flag[k] &= orthog[i*small_num + k];
      small_obj = small_objs[i];
      for (j = i + 1; j < small_num; ++j) {
	 if (flag[j] == 1) {
	    small_obj += small_objs[j];
	    flag[j] = 2;
	    for (k = j+1; k < small_num; ++k)
	       flag[k] &= orthog[j*small_num + k];
	 }
      }
      if (small_obj < best_obj) {
	 best_obj = small_obj;
	 memcpy(best_flag, flag, small_num);
      }
   }

   obj += best_obj;
   for (i = 0; i < small_num; ++i) {
      if (best_flag[i] != 2)
	 positives[smalls[i]] = -1;
   }

   delete[] flag;
   delete[] best_flag;
   delete[] small_objs;
   delete[] orthog;

   return obj;
}
