// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>
#include <algorithm>

#include "CoinTime.hpp"

#include "OsiVolSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"
#include "ClpDualRowSteepest.hpp"
#include "ClpPrimalColumnSteepest.hpp"
#include "ClpSimplex.hpp"

#include "MC_lp.hpp"

#include "BCP_lp.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_functions.hpp"

//#############################################################################

bool
MC_lp::is_gap_tailoff_rel(const int k, const double minimp,
			  const double objval) const
{
  const int iteration_count = current_iteration();
  if (k <= 0 || iteration_count <= k)
    return false;

  const int hist_size = CoinMin(iteration_count, hist_len);
  const double* obj_k_hist = objhist + (hist_size - k);

  // There is tailing off if in the last k steps we haven't closed a certain
  // fraction of the gap
  const double ub = upper_bound();
  return ((ub - objval) > (ub - obj_k_hist[0]) * (1-minimp));
}

//#############################################################################

bool
MC_lp::is_lb_tailoff_rel(const int k, const double minimp,
			 const double objval) const
{
  const int iteration_count = current_iteration();
  if (k <= 0 || iteration_count <= k)
    return false;

  const int hist_size = CoinMin(iteration_count, hist_len);
  const double* obj_k_hist = objhist + (hist_size - k);

  // There is tailing off if in the last k steps we haven't increased the lb
  return (objval - obj_k_hist[0] < minimp * CoinAbs(obj_k_hist[0]));
}

//#############################################################################

bool
MC_lp::is_lb_tailoff_abs(const int k, const double minimp,
			 const double objval) const
{
//     printf("k: %i   iteration_count: %i  hist_len: %i\n",
//  	  k, current_iteration(), hist_len);
  const int iteration_count = current_iteration();
  if (k <= 0 || iteration_count <= k)
    return false;

  const int hist_size = CoinMin(iteration_count, hist_len);
  const double* obj_k_hist = objhist + (hist_size - k);

  // There is tailing off if in the last k steps we haven't increased the lb
  return (objval - obj_k_hist[0] < minimp);
}

//#############################################################################

void
MC_lp::tailoff_test(bool& tailoff_gap_rel, bool& tailoff_lb_abs,
		    bool& tailoff_lb_rel, const double objval) const
{
   tailoff_gap_rel =
      is_gap_tailoff_rel(par.entry(MC_lp_par::TailoffGapRelMinItcount),
			 par.entry(MC_lp_par::TailoffGapRelMinImprovement),
			 objval);
   tailoff_lb_abs =
      is_lb_tailoff_abs(par.entry(MC_lp_par::TailoffLbAbsMinItcount),
			par.entry(MC_lp_par::TailoffLbAbsMinImprovement),
			objval);
   tailoff_lb_rel =
      is_lb_tailoff_rel(par.entry(MC_lp_par::TailoffLbRelMinItcount),
			par.entry(MC_lp_par::TailoffLbRelMinImprovement),
			objval);
   const int iteration_count = current_iteration();
   const int hist_size = CoinMin(iteration_count - 1, hist_len);
   printf("MC: Tailoff check:  objval: %.3f, UB: %.0f\n",
	  objval, upper_bound());
   for (int i = hist_size - 1; i >= 0; --i) {
      printf("                   LB[%3i]: %.3f\n", i-hist_size, objhist[i]);
   }
   printf("    gap_rel: %i   lb_abs: %i   lb_rel: %i\n",
	  tailoff_gap_rel ? 1 : 0,
	  tailoff_lb_abs ? 1 : 0,
	  tailoff_lb_rel ? 1 : 0);
}

//#############################################################################

/* Return true if solved to optimality (otherwise the node can be fathomed) */
bool
MC_solveClp(ClpSimplex& clp_simplex, OsiVolSolverInterface* vollp)
{
   double t = CoinCpuTime();

   // Set message handler to have same levels etc
   // clp_simplex.passInMessageHandler(clp->messageHandler());
   // set reasonable defaults
   clp_simplex.scaling(0); // this is 1 in OsiClpSolverInterface
   ClpDualRowSteepest steep;
   clp_simplex.setDualRowPivotAlgorithm(steep);
   ClpPrimalColumnSteepest steepP;
   clp_simplex.setPrimalColumnPivotAlgorithm(steepP);

   ClpSolve clp_options;
   // useDual, usePrimal, usePrimalorSprint, automatic
   clp_options.setSolveType(ClpSolve::useDual);
   // presolveOn, presolveOff, presolveNumber
   clp_options.setPresolveType(ClpSolve::presolveOn);
   /** which translation is:
	 which:
	 0 - startup in Dual  (nothing if basis exists).:
	              0 - no basis
		      1 - crash
		      2 - use initiative about idiot! but no crash
         1 - startup in Primal (nothing if basis exists):
	              0 - use initiative
		      1 - use crash
		      2 - use idiot and look at further info
		      3 - use sprint and look at further info
		      4 - use all slack
		      5 - use initiative but no idiot
		      6 - use initiative but no sprint
		      7 - use initiative but no crash
		      8 - do allslack or idiot
		      9 - do allslack or sprint
 	 2 - interrupt handling - 0 yes, 1 no (for threadsafe)
	 3 - whether to make +- 1matrix - 0 yes, 1 no
   */
   clp_options.setSpecialOption(0, 1);
   clp_options.setSpecialOption(2, 0);
   clp_options.setSpecialOption(3, 0);
   clp_simplex.initialSolve(clp_options);

   t = CoinCpuTime() - t;
   printf("LP: exact optimization took %.3f seconds\n", t);

   if (clp_simplex.isProvenPrimalInfeasible()) {
      printf("MC: Solving LP to optimality: infeas.\n");
      printf("MC: Forcing BCP to prune the node.\n");
      return false;
   }
   return true;
}

//#############################################################################

OsiSolverInterface* 
MC_lp::solveToOpt(OsiVolSolverInterface* vollp,
		  const BCP_lp_result& lpres,
		  const BCP_vec<BCP_var*>& vars,
		  const BCP_vec<BCP_cut*>& cuts,
		  double& exact_obj)
{
  if ((par.entry(MC_lp_par::LpSolver) & MC_UseClp) == 0) {
     throw BCP_fatal_error("\
MC_lp::solveToOpt: got here but no simplex based solver is enabled in the\n\
                   MC_LpSolver parameter.\n");
  }

  OsiClpSolverInterface* solver = new OsiClpSolverInterface;
  solver->loadProblem(*vollp->getMatrixByCol(), vollp->getColLower(),
			 vollp->getColUpper(), vollp->getObjCoefficients(),
			 vollp->getRowLower(), vollp->getRowUpper());
  ClpSimplex& clp = *solver->getModelPtr();

  if (par.entry(MC_lp_par::ExplicitSlacksInOpt)) {
     ClpSimplex clp_slack(clp);
     ClpSimplex& model = clp;
     ClpSimplex& model2 = clp_slack;

     int numcols = model.numberColumns();
     int numrows = model.numberRows();

     int * addStarts = new int [numrows+1];
     int * addRow = new int[numrows];
     double * addElement = new double[numrows];
     double * newUpper = new double[numrows];
     double * newLower = new double[numrows];

     double * lower = model2.rowLower();
     double * upper = model2.rowUpper();
     int iRow, iColumn;
     // Simplest is to change all rhs to zero
     // One should skip E rows but this is simpler coding
     for (iRow=0;iRow<numrows;iRow++) {
	newUpper[iRow]=upper[iRow];
	upper[iRow]=0.0;
	newLower[iRow]=lower[iRow];
	lower[iRow]=0.0;
	addRow[iRow]=iRow;
	addElement[iRow]=-1.0;
	addStarts[iRow]=iRow;
     }
     addStarts[numrows]=numrows;
     model2.addColumns(numrows,newLower,newUpper,NULL,
		       addStarts,addRow,addElement);
     delete [] addStarts;
     delete [] addRow;
     delete [] addElement;
     delete [] newLower;
     delete [] newUpper;

     // Modify costs
     model2.transposeTimes(1.0,vollp->getRowPrice(),model2.objective());
     // solve
     if (!MC_solveClp(model2, vollp)) {
	delete solver;
	return NULL;
     }
     int lookup[]={0,1,2,3,0,3};
     CoinWarmStartBasis basis;
     basis.setSize(numcols,numrows);
     for (iColumn = 0; iColumn < numcols; iColumn++) {
	int iStatus = model2.getColumnStatus(iColumn);
	iStatus = lookup[iStatus];
	basis.setStructStatus(iColumn,(CoinWarmStartBasis::Status) iStatus);
     }
     for (iRow=0;iRow<numrows;iRow++) {
	int iStatus = model2.getRowStatus(iRow) == ClpSimplex::basic ?
	   ClpSimplex::basic : model2.getColumnStatus(iRow+numcols);
	iStatus = lookup[iStatus];
	basis.setArtifStatus(iRow,(CoinWarmStartBasis::Status) iStatus);
     }
     solver->setWarmStart(&basis);
     solver->resolve();
  } else {
     if (!MC_solveClp(clp, vollp)) {
	delete solver;
	return NULL;
     }
  }

  exact_obj = solver->getObjValue();
  
  soln = mc_generate_heuristic_solution(solver->getColSolution(), vars, cuts);
  if (soln && soln->objective_value() < upper_bound()) {
    send_feasible_solution(soln);
  }

  // Run the heuristics.
  printf("MC: Solving LP to optimality before branching: %.3f",
	 exact_obj);
  const double gap =
    upper_bound() - exact_obj - get_param(BCP_lp_par::Granularity);
  printf("  Gap: %.3f\n",gap);
  if (gap < 0) {
    printf("MC: exact LP solving proved fathomability.\n");
    delete solver;
    return NULL;
  }

  // In any case, do a reduced cost fixing here.
  int fix = 0;
  reduced_cost_fixing(solver->getReducedCost(), solver->getColSolution(), gap,
		      // need to do this since rc_fixing expects non-const
		      getLpProblemPointer()->node->vars, fix);
  printf("MC: rc fixing with exact opt and dj of exact fixed %i vars.\n", fix);
  return solver;
}

//#############################################################################

void
MC_lp::choose_branching_vars(const BCP_vec<BCP_var*>& vars, const double * x,
			     const int cand_num,
			     BCP_vec<BCP_lp_branching_object*>& cands)
{
  // Try to branch on the variables when they are ordered by abs((distance
  // from integrality) * cost)
  const int m = mc.num_edges;
  const MC_graph_edge* edges = mc.edges;

  BCP_vec<int> perm;
  perm.reserve(m);

  BCP_vec<double> dist;
  dist.reserve(m);

  const double etol = par.entry(MC_lp_par::IntegerTolerance);

  int i;

  // If it's an Ising problem AND there's an external field then check only
  // the edges coming out of the external node
  if (mc.ising_triangles) {
     const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
     i = 2 * grid * grid; // number of grid edges
  } else {
     i = 0;
  }
  for ( ; i < m; ++i) {
     const double xi = x[i];
     if (xi > etol && xi < 1-etol) {
	perm.unchecked_push_back(i);
	const double w = (0.5 - CoinAbs(xi-0.5)) * edges[i].cost;
	dist.unchecked_push_back(-CoinAbs(w)); 
     }
  }

  const int distsize = dist.size();
  if (distsize > cand_num) {
    CoinSort_2(dist.begin(), dist.end(), perm.begin());
    perm.erase(perm.entry(cand_num), perm.end());
  }

  if (perm.size() > 0) {
     append_branching_vars(x, vars, perm, cands);
  }
}

//#############################################################################

BCP_branching_decision
MC_lp::select_branching_candidates(const BCP_lp_result& lpres,
				   const BCP_vec<BCP_var*>& vars,
				   const BCP_vec<BCP_cut*>& cuts,
				   const BCP_lp_var_pool& local_var_pool,
				   const BCP_lp_cut_pool& local_cut_pool,
				   BCP_vec<BCP_lp_branching_object*>& cands)
{
  OsiSolverInterface* lp = getLpProblemPointer()->lp_solver;
  OsiVolSolverInterface* vollp = dynamic_cast<OsiVolSolverInterface*>(lp);
  OsiSolverInterface* exact_solver = NULL;
  double objval = lpres.objval();
  bool tailoff_gap_rel = false;
  bool tailoff_lb_abs = false;
  bool tailoff_lb_rel = false;

  const int solver_t = par.entry(MC_lp_par::LpSolver);
  const bool has_vol = (solver_t & MC_UseVol);
  const bool has_simplex = (solver_t & MC_UseClp);

  int pool_size = local_cut_pool.size();

  if (current_index() != 0 && has_vol && has_simplex) {
     started_exact = true;
  }

  const int iteration_count = current_iteration();
  bool solve_to_opt = false;

  if (started_exact) {
     // must be has_vol && has_simplex
     solve_to_opt = true;
  } else {
     tailoff_test(tailoff_gap_rel, tailoff_lb_abs, tailoff_lb_rel, objval);
     if (has_vol && has_simplex &&
	 (tailoff_gap_rel && (tailoff_lb_abs || tailoff_lb_rel))) {
	solve_to_opt = true;
     }
  }

  if (solve_to_opt) {
     exact_solver = solveToOpt(vollp, lpres, vars, cuts, objval);
     if (par.entry(MC_lp_par::OnceOptAlwaysOpt)) {
	started_exact = true;
     }
     if (exact_solver == NULL)
	return BCP_DoNotBranch_Fathomed;
     tailoff_test(tailoff_gap_rel, tailoff_lb_abs, tailoff_lb_rel, objval);

     // Generate cuts from the exact soln (if there's any)
     BCP_lp_prob* p = getLpProblemPointer();
     BCP_lp_cut_pool& cp = *p->local_cut_pool;
     BCP_vec<BCP_cut*> new_cuts;
     BCP_vec<BCP_row*> new_rows;
     generate_cuts_in_lp(exact_solver->getColSolution(),
			 exact_solver->getRowActivity(),
			 exact_solver->getObjValue(),
			 vars, cuts, new_cuts, new_rows);
     const int new_size = new_cuts.size();
     if (new_rows.size() != 0) {
	cp.reserve(cp.size() + new_size);
	if (new_rows.size() == 0) {
	   new_rows.reserve(new_size);
	   cuts_to_rows(vars, new_cuts, new_rows,
			lpres, BCP_Object_FromGenerator, false);
	}
	for (int i = 0; i < new_size; ++i) {
	   new_cuts[i]->set_bcpind(-BCP_lp_next_cut_index(*p));
	   cp.unchecked_push_back(new BCP_lp_waiting_row(new_cuts[i],
							 new_rows[i]));
	}
     }
     printf("MC:   Generated %i cuts from exact solution.\n", new_size);
     pool_size = cp.size();

     if (par.entry(MC_lp_par::SwitchToSimplex)) {
	par.set_entry(MC_lp_par::LpSolver, MC_UseClp);
	set_param(BCP_lp_par::MaxCutsAddedPerIteration,
		  par.entry(MC_lp_par::MaxCutsAddedPerIterSim));
	set_param(BCP_lp_par::MaxPresolveIter,
		  par.entry(MC_lp_par::MaxPresolveIterSim));
	// swap solvers
	getLpProblemPointer()->lp_solver = exact_solver;
	getLpProblemPointer()->master_lp = new OsiClpSolverInterface;
	getLpProblemPointer()->lp_result->get_results(*exact_solver);
	exact_solver = 0;
	delete vollp;
	vollp = 0;
	// Now it is possible (though highly unlikely) that:
	//   No cut was found for volume, there's tailoff even with the exact
	//   opt, and the exact solution IS the opt solution (so no cut for
	//   that one either). In this case (after switching to the simplex
	//   solver) we can't branch, but we can't add a cut either. Best is
	//   just to go back and force another round (feasibility will be
	//   discovered there).
	return BCP_DoNotBranch;
     }
  }

  // Update the history
  if (iteration_count > hist_len) {
     if (objval < objhist[hist_len-1]) {
	objval = objhist[hist_len-1];
     }
     std::rotate(objhist, objhist+1, objhist+hist_len);
     objhist[hist_len-1] = objval;
  } else {
     if (iteration_count > 1) {
	if (objval < objhist[iteration_count-2]) {
	   objval = objhist[iteration_count-2];
	}
     }
     objhist[iteration_count-1] = objval;
  }

  if (pool_size != 0 &&
      !(tailoff_gap_rel && (tailoff_lb_abs || tailoff_lb_rel))) {
     delete exact_solver;
     return BCP_DoNotBranch;
  }

  // OK, we need to branch (or fathom if there's a limit on the depth)
  if (current_level() > par.entry(MC_lp_par::MaxDepth)) {
    printf("MC:  Maximum depth reached, pruning node.\n");
    return BCP_DoNotBranch_Fathomed;
  }

  // As a last ditch effort, try to solve the problem to optimality to see if
  // we can fathom the node
  if (vollp && !exact_solver) {
     exact_solver = solveToOpt(vollp, lpres, vars, cuts, objval);
     if (exact_solver == NULL) {
	return BCP_DoNotBranch_Fathomed;
     }
  }

  // OK, branch

  choose_branching_vars(vars, lpres.x(),
			par.entry(MC_lp_par::SB_CandidateNum), cands);

  if (cands.size() == 0) {
    // Now THIS is impossible. Well, no... However unlikely, it is possible
    // that we had tailing off, had an integer solution and had violated
    // cycle inequalities. Absurd, isn't it? But it did happen to me...
    // In this case just continue...
    if (local_cut_pool.size() == 0) {
      throw BCP_fatal_error("What the heck?! No cuts, integer & tailoff!\n");
    }
    delete exact_solver;
    return BCP_DoNotBranch;
  }

  // If we have an exact solver (i.e., we used volume) then perform the strong
  // branching ourselves. It's way faster to do it with the simplex. And it
  // may be better, too.
  // BTW, this routine just throws out all but one candidate from cands.
  if (exact_solver) {
     perform_strong_branching(lpres, exact_solver, cands);
     delete exact_solver;
  }

  return BCP_DoBranch;
}

//#############################################################################

BCP_branching_object_relation
MC_lp::compare_branching_candidates(BCP_presolved_lp_brobj* newobj,
				    BCP_presolved_lp_brobj* oldobj)
{
   // *FIXME*: For now just use the default, later we might want to choose the
   // one that makes things more integral.
   return BCP_lp_user::compare_branching_candidates(newobj, oldobj);
}

//#############################################################################

void
MC_lp::set_actions_for_children(BCP_presolved_lp_brobj* best)
{
   // We have stored our own presolved brobj if we performed strong branching.
   // In that case just initialize the action vector then swap best with
   // best_presolved, and finally let BCP adjust the actions for the children
   if (best_presolved) {
      best->swap(*best_presolved);
      delete best_presolved;
      best_presolved = 0;
   }
   BCP_lp_user::set_actions_for_children(best);
}

//#############################################################################


void
MC_lp::perform_strong_branching(const BCP_lp_result& lpres,
				OsiSolverInterface* exact_solver,
				BCP_vec<BCP_lp_branching_object*>& cands)
{
   int i;
   
   // prepare for strong branching
   exact_solver->markHotStart();

   // Look at the candidates one-by-one and presolve them.
   BCP_vec<BCP_lp_branching_object*>::iterator cani;

   printf("\n\
MC: Starting strong branching (the objective is transformed!):\
    The objective shift is %.4f\n\n", obj_shift);

   assert(best_presolved == 0);
   int best_fathom_child = 0;
   double best_objsum = DBL_MAX;
   BCP_vec<double> tmpobj;

   for (cani = cands.begin(); cani != cands.end(); ++cani){
      // Create a temporary branching object to hold the current results
      BCP_presolved_lp_brobj* tmp_presolved= new BCP_presolved_lp_brobj(*cani);
      const BCP_lp_branching_object* can = *cani;
      for (i = 0; i < can->child_num; ++i){
	 can->apply_child_bd(exact_solver, i);
	 exact_solver->solveFromHotStart();
	 tmp_presolved->get_results(*exact_solver, i);
      }
      // add the shift to the objective values
      tmp_presolved->get_lower_bounds(tmpobj);
      for (i = 0; i < can->child_num; ++i) {
	 tmpobj[i] += obj_shift;
      }
      tmp_presolved->set_lower_bounds(tmpobj);
      // reset the bounds of the affected var
      const int bvar_ind = (*can->forced_var_pos)[0];
      exact_solver->setColBounds(bvar_ind, 0.0, 1.0);
      if (get_param(BCP_lp_par::LpVerb_PresolveResult)) {
	 printf("MC strong branch:   Presolving:");
	 if (get_param(BCP_lp_par::LpVerb_PresolvePositions)) {
	    can->print_branching_info(exact_solver->getNumCols(), lpres.x(),
				      exact_solver->getObjCoefficients());
	 }
	 for (i = 0; i < can->child_num; ++i) {
	    const BCP_lp_result& res = tmp_presolved->lpres(i);
	    const double lb = res.objval();
	    printf((lb > DBL_MAX / 4 ? " [%e,%i,%i]" : " [%.4f,%i,%i]"),
		   lb, res.termcode(), res.iternum());
	 }
	 printf("\n");
      }
      
      // Compare the current one with the best so far
      // First see how many children would be fathomed
      int fathom_child = 0;
      double objsum = 0.0;
      for (i = 0; i < can->child_num; ++i) {
	 const BCP_lp_result& res = tmp_presolved->lpres(i);
	 if (res.termcode() == BCP_ProvenPrimalInf) {
	    ++fathom_child;
	    continue;
	 }
	 if (res.termcode() == BCP_ProvenOptimal &&
	     res.objval() + obj_shift + get_param(BCP_lp_par::Granularity) >
	     upper_bound()) {
	    ++fathom_child;
	    continue;
	 }
	 objsum += res.objval();
      }
      if (best_fathom_child < fathom_child) {
	 std::swap(tmp_presolved, best_presolved);
	 best_fathom_child = fathom_child;
	 delete tmp_presolved;
	 if (best_fathom_child == can->child_num) {
	    purge_ptr_vector(cands, cani + 1, cands.end());
	 }
	 best_objsum = objsum;
	 continue;
      }

      if (objsum < best_objsum) {
	 best_objsum = objsum;
	 std::swap(tmp_presolved, best_presolved);
      }
      delete tmp_presolved;
   }

   // indicate to the lp solver that strong branching is done
   exact_solver->unmarkHotStart();
   
   // delete all the candidates but the selected one
   BCP_lp_branching_object* can = best_presolved->candidate();
   for (cani = cands.begin(); cani != cands.end(); ++cani) {
      if (*cani != can)
	 delete *cani;
   }

   cands.clear();
   cands.push_back(can);
}
