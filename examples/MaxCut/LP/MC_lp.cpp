// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>
#include <algorithm>
#include <numeric>

#include "CoinHelperFunctions.hpp"

#include <OsiVolSolverInterface.hpp>
#include <OsiClpSolverInterface.hpp>

#include "BCP_math.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_node.hpp"

#include "MC_cut.hpp"
#include "MC_lp.hpp"

//#############################################################################

static inline void
MC_update_solution(MC_solution*& sol_old, MC_solution*& sol_new)
{
  if (!sol_old) {
    sol_old = sol_new;
  } else {
    if (sol_new) {
      if (sol_old->objective_value() > sol_new->objective_value()) {
	delete sol_old;
	sol_old = sol_new;
      } else {
	delete sol_new;
      }
    }
  }
  sol_new = NULL;
}
  

//#############################################################################

static inline bool
MC_cycle_row_pair_comp(const std::pair<BCP_cut*, BCP_row*>& p0,
		       const std::pair<BCP_cut*, BCP_row*>& p1)
{
  return MC_cycle_cut_less(p0.first, p1.first);
}

//-----------------------------------------------------------------------------

void
MC_lp::unique_cycle_cuts(BCP_vec<BCP_cut*>& new_cuts,
			 BCP_vec<BCP_row*>& new_rows)
{
  size_t i;
  const size_t cutnum = new_cuts.size();
  if (cutnum != new_rows.size())
    throw BCP_fatal_error("Uneven vector sizes in MC_lp_unique_cycle_cuts.\n");

  if (cutnum == 0)
    return;

  std::pair<BCP_cut*, BCP_row*>* cut_row_pairs =
    new std::pair<BCP_cut*, BCP_row*>[cutnum];
  for (i = 0; i < cutnum; ++i) {
    cut_row_pairs[i].first = new_cuts[i];
    cut_row_pairs[i].second = new_rows[i];
  }

  std::sort(cut_row_pairs, cut_row_pairs + cutnum, MC_cycle_row_pair_comp);
  int k = 0;
  new_cuts[0] = cut_row_pairs[0].first;
  new_rows[0] = cut_row_pairs[0].second;
  for (i = 1; i < cutnum; ++i) {
    if (MC_cycle_cut_equal(new_cuts[k], cut_row_pairs[i].first)) {
      delete cut_row_pairs[i].first;
      delete cut_row_pairs[i].second;
    } else {
      new_cuts[++k] = cut_row_pairs[i].first;
      new_rows[k]   = cut_row_pairs[i].second;
    }
  }
  ++k;

  delete[] cut_row_pairs;
  new_cuts.erase(new_cuts.entry(k), new_cuts.end());
  new_rows.erase(new_rows.entry(k), new_rows.end());
}
  
//#############################################################################

void
MC_lp::unpack_module_data(BCP_buffer & buf)
{
  par.unpack(buf);
  mc.unpack(buf);

  mc.create_adj_lists();

  hist_len = par.entry(MC_lp_par::TailoffGapRelMinItcount);
  hist_len = CoinMax(hist_len, par.entry(MC_lp_par::TailoffLbAbsMinItcount));
  hist_len = CoinMax(hist_len, par.entry(MC_lp_par::TailoffLbRelMinItcount));

  objhist = hist_len <= 0 ? 0 : new double[hist_len];

  if (par.entry(MC_lp_par::LpSolver) & MC_UseVol) {
     set_param(BCP_lp_par::MaxCutsAddedPerIteration,
	       par.entry(MC_lp_par::MaxCutsAddedPerIterVol));
     set_param(BCP_lp_par::MaxPresolveIter,
	       par.entry(MC_lp_par::MaxPresolveIterVol));
  } else {
     set_param(BCP_lp_par::MaxCutsAddedPerIteration,
	       par.entry(MC_lp_par::MaxCutsAddedPerIterSim));
     set_param(BCP_lp_par::MaxPresolveIter,
	       par.entry(MC_lp_par::MaxPresolveIterSim));
  }
}

//#############################################################################
// Override the initializer so that we can choose between vol and simplex
// at runtime.

OsiSolverInterface *
MC_lp::initialize_solver_interface()
{
  if ((par.entry(MC_lp_par::LpSolver) & MC_UseVol) != 0) {
     return new OsiVolSolverInterface;
  }

  if ((par.entry(MC_lp_par::LpSolver) & MC_UseClp) != 0) {
     return new OsiClpSolverInterface;
  }

  throw BCP_fatal_error("MC: No solver is specified!\n");
  return 0; // fake return
}

//#############################################################################
// Opportunity to reset things before optimization
void
MC_lp::modify_lp_parameters(OsiSolverInterface* lp, const int changeType,
			    bool in_strong_branching)
{
  if (current_iteration() == 1 &&
      ( ((par.entry(MC_lp_par::LpSolver) & MC_UseClp) != 0) )
      ) {
    started_exact = false;
  }
  {
    OsiVolSolverInterface* vollp = dynamic_cast<OsiVolSolverInterface*>(lp);
    if (vollp) {
      VOL_parms& vpar = vollp->volprob()->parm;
      vpar.lambdainit = par.entry(MC_lp_par::Vol_lambdaInit); 
      vpar.alphainit =  par.entry(MC_lp_par::Vol_alphaInit);
      vpar.alphamin = par.entry(MC_lp_par::Vol_alphaMin);
      vpar.alphafactor = par.entry(MC_lp_par::Vol_alphaFactor);
      vpar.primal_abs_precision = par.entry(MC_lp_par::Vol_primalAbsPrecision);
      vpar.gap_abs_precision = par.entry(MC_lp_par::Vol_gapAbsPrecision);
      vpar.gap_rel_precision = par.entry(MC_lp_par::Vol_gapRelPrecision);
      vpar.granularity = par.entry(MC_lp_par::Vol_granularity);
      vpar.minimum_rel_ascent = par.entry(MC_lp_par::Vol_minimumRelAscent);
      vpar.ascent_first_check = par.entry(MC_lp_par::Vol_ascentFirstCheck);
      vpar.ascent_check_invl = par.entry(MC_lp_par::Vol_ascentCheckInterval);
      vpar.maxsgriters = par.entry(MC_lp_par::Vol_maxSubGradientIterations);
      vpar.printflag =  par.entry(MC_lp_par::Vol_printFlag);
      vpar.printinvl =  par.entry(MC_lp_par::Vol_printInterval);
      vpar.greentestinvl = par.entry(MC_lp_par::Vol_greenTestInterval);
      vpar.yellowtestinvl =  par.entry(MC_lp_par::Vol_yellowTestInterval);
      vpar.redtestinvl = par.entry(MC_lp_par::Vol_redTestInterval);
      vpar.alphaint =  par.entry(MC_lp_par::Vol_alphaInt);
    }
    if (in_strong_branching) {
      // *THINK* : we might want to do fewer iterations???
    }
  }
}

//#############################################################################

BCP_solution*
MC_lp::test_feasibility(const BCP_lp_result& lp_result,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts)
{
  int i, k;

  const double etol = par.entry(MC_lp_par::IntegerTolerance);
  BCP_vec<double> x(lp_result.x(), vars.size());
  // First round the integer part of the solution. 
  for (k = x.size(), i = x.size() - 1; i >= 0; --i) {
    const double xx = x[i];
    if (xx > 1 - etol) {
      --k;
      x[i] = 1;
    } else if (xx < etol) {
      --k;
      x[i] = 0;
    }
  }

  // If everything is integer check if the soln is feasible (there might be
  // violated cycles inequalities). Fortunately, the MST heuristic WILL find
  // a violated cycle if everything is integer, moreover, that cycle will be
  // violated by 1.
  if (k == 0) {
    BCP_vec<BCP_cut*> new_cuts;
    BCP_vec<BCP_row*> new_rows;
    const int improve_round = par.entry(MC_lp_par::HeurSwitchImproveRound);
    const bool edge_switch = par.entry(MC_lp_par::DoEdgeSwitchHeur);
    const int struct_switch = ( par.entry(MC_lp_par::StructureSwitchHeur) &
			        ((1 << mc.num_structure_type) - 1) );
    MC_solution* sol = MC_mst_cutgen(mc, x.begin(), NULL /* no weights*/,
				     1.0, 0.0,
				     MC_MstEdgeOrderingPreferExtreme,
				     improve_round, edge_switch, struct_switch,
				     .9, 0, new_cuts, new_rows);
    purge_ptr_vector(new_rows);
    if (new_cuts.size() == 0) {
      // jackpot! feasible solution
      return sol;
    }
    purge_ptr_vector(new_cuts);
  }

  return NULL;
}

//#############################################################################

BCP_solution*
MC_lp::generate_heuristic_solution(const BCP_lp_result& lpres,
				   const BCP_vec<BCP_var*>& vars,
				   const BCP_vec<BCP_cut*>& cuts)
{
  return mc_generate_heuristic_solution(lpres.x(), vars, cuts);
}

//#############################################################################

MC_solution*
MC_lp::mc_generate_heuristic_solution(const double* x,
				      const BCP_vec<BCP_var*>& vars,
				      const BCP_vec<BCP_cut*>& cuts)
{
  const int m = mc.num_edges;
  const int n = mc.num_nodes;
  
  const int heurnum = par.entry(MC_lp_par::MstHeurNum);
  const int improve_round = par.entry(MC_lp_par::HeurSwitchImproveRound);
  const bool do_edge_switch = par.entry(MC_lp_par::DoEdgeSwitchHeur);
  const int struct_switch = ( par.entry(MC_lp_par::StructureSwitchHeur) &
			      ((1 << mc.num_structure_type) - 1) );
  const double maxlambda = par.entry(MC_lp_par::MaxPerturbInMstHeur);

  double* w = new double[m];
  double * xx = new double[m];

  BCP_vec<int> nodesign(n, 1);

  MC_solution* sol;
  BCP_vec<double> obj;
  BCP_vec<char> type;

  int i, j;

  // Generate with random additive perturbation
  for (i = 0; i < heurnum; ++i) {
    for (j = 0; j < m; ++j)
      w[j] = CoinDrand48();
    sol = MC_mst_heur(mc, x, w, 1.0, (maxlambda * i) / heurnum,
		      MC_MstEdgeOrderingPreferExtreme,
		      improve_round, do_edge_switch, struct_switch);
    MC_update_solution(soln, sol);
  }
#if 0
  for (i = 0; i < heurnum; ++i) {
    if (i > 0)
      for (j = 0; j < m; ++j)
	w[j] = CoinDrand48();
    sol = MC_mst_heur(mc, x, w, 1.0, (maxlambda * i) / heurnum,
		      MC_MstEdgeOrderingPreferZero,
		      improve_round, do_edge_switch, struct_switch);
    MC_update_solution(soln, sol);
  }
  for (i = 0; i < heurnum; ++i) {
    if (i > 0)
      for (j = 0; j < m; ++j)
	w[j] = CoinDrand48();
    sol = MC_mst_heur(mc, x, w, 1.0, (maxlambda * i) / heurnum,
		      MC_MstEdgeOrderingPreferOne,
		      improve_round, do_edge_switch, struct_switch);
    MC_update_solution(soln, sol);
  }
#endif
#if 0
  // Generate with random multiplicative and additive perturbation
  for (i = 0; i < heurnum; ++i) {
    for (j = 0; j < m; ++j) {
      w[j] = CoinDrand48();
      xx[j] = x[j] * (.95 + .1 * CoinDrand48());
    }
    sol = MC_mst_heur(mc, xx, w, 1.0, (maxlambda * i) / heurnum,
		      par.entry(MC_lp_par::HeurSwitchImproveRound),
		      par.entry(MC_lp_par::DoEdgeSwitchHeur),
		      par.entry(MC_lp_par::DoIsingSquareSwitchHeur));
    MC_update_solution(soln, sol);
  }
#endif
#if 0
  // Shrink the primal solution to be feasible (well, at least the nonzero rhs
  // constraints...) and generate with additive perturbation
  const double * lhs = lpres.lhs();
  const int numcuts = cuts.size();
  double ratio = 1.0;
  for (i = 0; i < numcuts; ++i) {
    const double ub = cuts[i]->ub();
    if (lhs[i] <= ub)
      continue;
    if (ub > 0.0) {
      ratio = CoinMin(ratio, ub/lhs[i]);
    }
  }
  for (i = 0; i < m; ++i)
    xx[i] = x[i]*ratio;
  for (i = 0; i < heurnum; ++i) {
    for (j = 0; j < m; ++j) {
      w[j] = CoinDrand48();
    }
    sol = MC_mst_heur(mc, xx, w, 1.0, (maxlambda * i) / heurnum,
		      par.entry(MC_lp_par::HeurSwitchImproveRound),
		      par.entry(MC_lp_par::DoEdgeSwitchHeur),
		      par.entry(MC_lp_par::DoIsingSquareSwitchHeur));
    MC_update_solution(soln, sol);
  }
#endif
  delete[] w;
  delete[] xx;

  sol = soln;
  soln = NULL;
  return sol;
}

//#############################################################################

void
MC_lp::pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol)
{
  const MC_solution* mcsol = dynamic_cast<const MC_solution*>(sol);
  mcsol->pack(buf);
}

//#############################################################################

void
MC_lp::cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
		    BCP_vec<BCP_cut*>& cuts,       // what to expand
		    BCP_vec<BCP_row*>& rows,       // the expanded rows
		    // things that the user can use for lifting cuts if
		    // allowed 
		    const BCP_lp_result& lpres,
		    BCP_object_origin origin, bool allow_multiple)
{
  const int cutnum = cuts.size();
  rows.clear();
  rows.reserve(cutnum);

  const double lb = -BCP_DBL_MAX;

  const int varnum = vars.size();
  double* coefs = new double[varnum];
  int*    inds  = new int[varnum];
  int* iotainds = new int[varnum];
  CoinIotaN(iotainds, varnum, 0);

  for (int i = 0; i < cutnum; ++i) {
    const MC_cycle_cut* cycle_cut =
      dynamic_cast<const MC_cycle_cut*>(cuts[i]);
    if (cycle_cut) {
      const int len = cycle_cut->cycle_len;
      const int pos = cycle_cut->pos_edges;
      CoinDisjointCopyN(cycle_cut->edges, len, inds);
      CoinFillN(coefs, pos, 1.0);
      CoinFillN(coefs + pos, len - pos, -1.0);
      CoinSort_2(inds, inds + len, coefs);
      rows.unchecked_push_back(new BCP_row(inds, inds + len,
					   coefs, coefs + len,
					   lb, pos - 1.0));
      continue;
    }
    const MC_explicit_dense_cut* dense_cut =
      dynamic_cast<const MC_explicit_dense_cut*>(cuts[i]);
    if (dense_cut) {
      rows.unchecked_push_back(new BCP_row(iotainds,
					   iotainds + varnum,
					   dense_cut->coeffs,
					   dense_cut->coeffs+varnum,
					   lb, dense_cut->rhs));
      continue;
    }
    throw BCP_fatal_error("Non-MC_cut to be exanded!\n");
  }

  if (mc.feas_sol) {
     for (int i = 0; i < cutnum; ++i) {
	const double lhs = rows[i]->dotProduct(mc.feas_sol->value);
	if (cuts[i]->lb() != rows[i]->LowerBound() ||
	    cuts[i]->ub() != rows[i]->UpperBound()) {
	   printf("*** What! *** : cut bounds are not the same: %i %f %f %f %f\n",
		  i, cuts[i]->lb(), rows[i]->LowerBound(),
		  cuts[i]->ub(), rows[i]->UpperBound());
	}
	if (lhs < cuts[i]->lb() || lhs > cuts[i]->ub()) {
	   printf("*** Violation *** : index: %i  lhs: %f    lb: %f    ub: %f\n",
		  i, lhs, cuts[i]->lb(), cuts[i]->ub());
	}
     }
  }

  delete[] iotainds;
  delete[] inds;
  delete[] coefs;
}

//#############################################################################

BCP_object_compare_result
MC_lp::compare_cuts(const BCP_cut* c0, const BCP_cut* c1)
{
  return MC_cycle_cut_equal(c0, c1) ? BCP_ObjsAreSame : BCP_DifferentObjs;
}

//#############################################################################
//#############################################################################

void
MC_lp::generate_mst_cuts(const double* x, const double* lhs,
			 const double objval,
			 const BCP_vec<BCP_var*>& vars,
			 const BCP_vec<BCP_cut*>& cuts,
			 BCP_vec<BCP_cut*>& new_cuts,
			 BCP_vec<BCP_row*>& new_rows)
{
   const double max_lp_viol = getMaxLpViol();
   const int heurnum = par.entry(MC_lp_par::CycleCutHeurNum);
   const double minviol =
      CoinMax(max_lp_viol, par.entry(MC_lp_par::MinMstCycleCutViolation));
   const double maxlambda = par.entry(MC_lp_par::MaxPerturbInMstCycleCutGen);

   const int heurswitchround = par.entry(MC_lp_par::HeurSwitchImproveRound);
   const bool edge_switch = par.entry(MC_lp_par::DoEdgeSwitchHeur);
   const int struct_switch = ( par.entry(MC_lp_par::StructureSwitchHeur) &
			       ((1 << mc.num_structure_type) - 1) );

   const int numvars = vars.size();
   double* w = new double[numvars];
   double* xx = new double[numvars];
   const int max_cycle_num = par.entry(MC_lp_par::MaxCycleCutNum);

   // Generate with random additive perturbation
   int i, j;
   MC_solution* sol = NULL;
   for (i = 0; i < heurnum; ++i) {
      if (i > 0)
	 for (j = 0; j < numvars; ++j)
	    w[j] = CoinDrand48();
      sol = MC_mst_cutgen(mc, x, w, 1.0, (maxlambda*i)/heurnum,
			  MC_MstEdgeOrderingPreferExtreme,
			  heurswitchround, edge_switch, struct_switch,
			  minviol, new_cuts.size() + max_cycle_num,
			  new_cuts, new_rows);
      MC_update_solution(soln, sol);
   }
#if 0
   for (i = 0; i < heurnum; ++i) {
      if (i > 0)
	 for (j = 0; j < numvars; ++j)
	    w[j] = CoinDrand48();
      sol = MC_mst_cutgen(mc, x, w, 1.0, (maxlambda*i)/heurnum,
			  MC_MstEdgeOrderingPreferOne, minviol,
			  heurswitchround, edge_switch, struct_switch,
			  minviol, new_cuts.size() + max_cycle_num,
			  new_cuts, new_rows);
      delete sol;
   }
   for (i = 0; i < heurnum; ++i) {
      if (i > 0)
	 for (j = 0; j < numvars; ++j)
	    w[j] = CoinDrand48();
      sol = MC_mst_cutgen(mc, x, w, 1.0, (maxlambda*i)/heurnum,
			  MC_MstEdgeOrderingPreferExtreme, minviol,
			  heurswitchround, edge_switch, struct_switch,
			  minviol, new_cuts.size() + max_cycle_num,
			  new_cuts, new_rows);
      delete sol;
   }
   const int cutnum_add = new_cuts.size() - cutnum;
   printf("MC:  cycle cuts (add): %i\n", cutnum_add);
#endif
#if 0
   // Generate with random multiplicative and additive perturbation
   for (i = 0; i < numvars; ++i)
      xx[i] = x[i]*(.95 + .1 * CoinDrand48());
   for (i = 0; i < heurnum; ++i) {
      if (i > 0) {
	 for (j = 0; j < numvars; ++j)
	    w[j] = CoinDrand48();
      }
      sol = MC_mst_cutgen(mc, xx, w, 1.0, (maxlambda*i)/heurnum, minviol,
			  heurswitchround, new_cuts.size() + max_cycle_num,
			  new_cuts, new_rows);
      printf("MC: sol value:     %.0f\n", sol->objective_value());
      
      MC_update_solution(soln, sol);
   }
   const int cutnum_mult_add = new_cuts.size() - cutnum - cutnum_add;
   printf("MC:  cycle cuts (mult & add): %i\n", cutnum_mult_add);
#endif
   
   // Shrink the primal solution to be feasible (well, at least the nonzero
   // rhs constraints...) and generate with additive perturbation
   {
#if 0
      const double * lhs = lpres.lhs();
      const int numcuts = cuts.size();
      double ratio = 1.0;
      int cnt = 0;
      double maxviol = 0.0;
      for (i = 0; i < numcuts; ++i) {
	 const double ub = cuts[i]->ub();
	 if (lhs[i] <= ub)
	    continue;
	 if (ub == 0.0) {
	    ++cnt;
	    maxviol = CoinMax(maxviol, lhs[i] - ub);
	 } else {
	    ratio = CoinMin(ratio, ub/lhs[i]);
	 }
      }
      printf("MC: primal shrinking: %.6f , %i cuts left violated (%.6f)\n",
	     ratio, cnt, maxviol);
      for (i = 0; i < numvars; ++i)
	 xx[i] = x[i]*ratio;
      for (i = 0; i < heurnum; ++i) {
	 if (i > 0) {
	    for (j = 0; j < numvars; ++j)
	       w[j] = CoinDrand48();
	 }
	 sol = MC_mst_cutgen(mc, xx, w, 1.0, (maxlambda*i)/heurnum, minviol,
			     heurswitchround, new_cuts.size() + max_cycle_num,
			     new_cuts, new_rows);
	 printf("MC: sol value:     %.0f\n", sol->objective_value());
	 MC_update_solution(soln, sol);
      }
      const int cutnum_shrink_add =
	 new_cuts.size() - cutnum - cutnum_add - cutnum_mult_add;
      printf("MC:  cycle cuts (shrink & add): %i\n", cutnum_shrink_add);
#endif
   }
   delete[] w;
   delete[] xx;
}

void
MC_lp::generate_sp_cuts(const double* x, const double* lhs,
			const double objval,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts,
			BCP_vec<BCP_cut*>& new_cuts,
			BCP_vec<BCP_row*>& new_rows)
{
   const double max_lp_viol = getMaxLpViol();
   const bool all_sp_cuts = par.entry(MC_lp_par::ReportAllSPCycleCuts);
   const double minviol =
      CoinMax(max_lp_viol, par.entry(MC_lp_par::MinSPCycleCutViolation));
   MC_generate_shortest_path_cycles(mc, x, all_sp_cuts, minviol,
				    new_cuts, new_rows);
}

void
MC_lp::generate_cuts_in_lp(const BCP_lp_result& lpres,
			   const BCP_vec<BCP_var*>& vars,
			   const BCP_vec<BCP_cut*>& cuts,
			   BCP_vec<BCP_cut*>& new_cuts,
			   BCP_vec<BCP_row*>& new_rows)
{
#if 0
   const double* x = lpres.x();
   double* xx = new double[vars.size()];
   for (int i = vars.size() - 1; i >= 0; --i) {
      if (x[i] < 0.0)
	 xx[i] = 0.0;
      else if (x[i] > 1.0)
	 xx[i] = 1.0;
      else
	 xx[i] = x[i];
   }
   generate_cuts_in_lp(xx, lpres.lhs(), lpres.objval(),
		       vars, cuts, new_cuts, new_rows);
   delete[] xx;
#else
   generate_cuts_in_lp(lpres.x(), lpres.lhs(), lpres.objval(),
		       vars, cuts, new_cuts, new_rows);
#endif
}

double
MC_lp::getMaxLpViol()
{
   double max_lp_viol = 0.0;
   // If the volume algorithm is used than we want to go with at least as
   // high minimum violation than the max violation of the last
   // optimization.
   OsiVolSolverInterface* vollp =
      dynamic_cast<OsiVolSolverInterface*>(getLpProblemPointer()->lp_solver);
   if (vollp) {
      const double * lhs = getLpProblemPointer()->lp_result->lhs();
      const double * rhs = vollp->getRightHandSide();
      for (int k = vollp->getNumRows() - 1; k >= 0; --k)
	 max_lp_viol = CoinMax(max_lp_viol, lhs[k] - rhs[k]);
   }
   max_lp_viol += 0.001;
   return max_lp_viol;
}

void
MC_lp::generate_cuts_in_lp(const double* x, const double* lhs,
			   const double objval,
			   const BCP_vec<BCP_var*>& vars,
			   const BCP_vec<BCP_cut*>& cuts,
			   BCP_vec<BCP_cut*>& new_cuts,
			   BCP_vec<BCP_row*>& new_rows)
{
  bool tailoff_gap_rel, tailoff_lb_abs, tailoff_lb_rel;
  tailoff_test(tailoff_gap_rel, tailoff_lb_abs, tailoff_lb_rel, objval);

  const double max_lp_viol = getMaxLpViol();

  int i, cutnum = new_cuts.size();

  if (mc.ising_four_cycles || mc.ising_triangles) {
    const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
    const int grid_nodes = grid*grid;
    const double minviol =
      CoinMax(max_lp_viol, par.entry(MC_lp_par::MinIsingCutViolation));

    if (mc.ising_four_cycles) {
      MC_test_ising_four_cycles(grid_nodes, mc.ising_four_cycles,
				x, minviol, new_cuts, new_rows);
      const int ising_four_cycle_num = new_cuts.size() - cutnum;
      printf("MC:  ising four cycles: %i \n", ising_four_cycle_num);
      cutnum = new_cuts.size();
    }

    if (mc.ising_triangles) {
      MC_test_ising_triangles(2*grid_nodes, mc.ising_triangles,
			      x, 0.9, new_cuts, new_rows);
      const int ising_triangle_num = new_cuts.size() - cutnum;
      printf("MC:  ising triangles: %i \n", ising_triangle_num);
      cutnum = new_cuts.size();
    }
  }

  const int gen_mst_cycle = par.entry(MC_lp_par::MstCycleCutGeneration);
#if 0
  const bool mst = gen_mst_cycle == MC_AlwaysGenerateMstCycleCuts;
#else
  const bool mst =
     gen_mst_cycle == MC_AlwaysGenerateMstCycleCuts ||
     (gen_mst_cycle == MC_GenerateMstCycleCutsAsLastResort &&
      (cutnum < 50 || tailoff_gap_rel || tailoff_lb_abs || tailoff_lb_rel));
#endif

  if (mst) {
     generate_mst_cuts(x, lhs, objval, vars, cuts, new_cuts, new_rows);
     const int cycle_num = new_cuts.size() - cutnum;
     unique_cycle_cuts(new_cuts, new_rows);
     printf("MC:  cycle cuts: %i  (%i before removing duplicates)\n",
	    static_cast<int>(new_cuts.size()) - cutnum, cycle_num);
     cutnum = new_cuts.size();
  }

  const MC_SPCycleCutGen gen_sp_cycle =
    static_cast<MC_SPCycleCutGen>(par.entry(MC_lp_par::SPCycleCutGeneration));
#if 0
  const bool sp_cuts = gen_sp_cycle == MC_AlwaysGenerateSPCycleCuts;
#else
  const bool sp_cuts =
    gen_sp_cycle == MC_AlwaysGenerateSPCycleCuts ||
    (gen_sp_cycle == MC_GenerateSPCycleCutsAsLastResort &&
     (cutnum < 50 || tailoff_gap_rel || tailoff_lb_abs || tailoff_lb_rel));
#endif

  if (sp_cuts) {
     generate_sp_cuts(x, lhs, objval, vars, cuts, new_cuts, new_rows);
     const int sp_cycle_num = new_cuts.size() - cutnum;
     unique_cycle_cuts(new_cuts, new_rows);
     printf("MC:  SP based cycle cuts: %i  (%i before removing duplicates)\n",
	    static_cast<int>(new_cuts.size()) - cutnum, sp_cycle_num);
     cutnum = new_cuts.size();
  }

  if (mc.feas_sol) {
     for (i = 0; i < cutnum; ++i) {
	const double lhs = new_rows[i]->dotProduct(mc.feas_sol->value);
	if (new_cuts[i]->lb() != new_rows[i]->LowerBound() ||
	    new_cuts[i]->ub() != new_rows[i]->UpperBound()) {
	   printf("*** What! *** : cut bounds are not the same: %i %f %f %f %f\n",
		  i, new_cuts[i]->lb(), new_rows[i]->LowerBound(),
		  new_cuts[i]->ub(), new_rows[i]->UpperBound());
	}
	if (lhs < new_cuts[i]->lb() || lhs > new_cuts[i]->ub()) {
	   printf("*** Violation *** : index: %i  lhs: %f    lb: %f    ub: %f\n",
		  i, lhs, new_cuts[i]->lb(), new_cuts[i]->ub());
	}
     }
  }
}

//#############################################################################

void
MC_lp::logical_fixing(const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const BCP_vec<BCP_cut*>& cuts,
		      const BCP_vec<BCP_obj_status>& var_status,
		      const BCP_vec<BCP_obj_status>& cut_status,
		      const int var_bound_changes_since_logical_fixing,
		      BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd)
{
  /* We are going to find the connected components of the subgraph spanned by
     the fixed edges and at the same time in each component we'll mark on
     which side each node is */
  if (var_bound_changes_since_logical_fixing == 0)
    return;
#if 1
  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  const MC_graph_edge* edges = mc.edges;

  int * first_on_chain = new int[n];
  int * last_on_chain = new int[n];
  int * next_on_chain = new int[n];
  int * size_of_chain = new int[n];
  int * sig = new int[n];

  CoinIotaN(first_on_chain, n, 0);
  CoinIotaN(last_on_chain, n, 0);
  CoinFillN(next_on_chain, n, -1);
  CoinFillN(size_of_chain, n, 1);
  CoinFillN(sig, n, 1);

  int tree_size = 0;

  int label_s = -1; // shorter chain head
  int label_l = -1; // longer chain head
  for (int k = 0; k < m; ++k) {
    if (vars[k]->lb() != vars[k]->ub())
      continue;
    const int i = edges[k].tail;
    const int j = edges[k].head;
    const int label_i = first_on_chain[i];
    const int label_j = first_on_chain[j];
    if (label_i == label_j)
      continue;
    if (size_of_chain[label_i] > size_of_chain[label_j]) {
      label_s = label_j;
      label_l = label_i;
    } else {
      label_s = label_i;
      label_l = label_j;
    }
    ++tree_size;
    for (int l = label_s; l != -1; l = next_on_chain[l]) {
      first_on_chain[l] = label_l;
    }
    if ((vars[k]->lb() == 0.0 && sig[i] != sig[j]) ||
	(vars[k]->lb() == 1.0 && sig[i] == sig[j])) {
      // must reverse the sigs on the shorter chain, too
      for (int l = label_s; l != -1; l = next_on_chain[l]) {
	sig[l] *= -1;
      }
    }
    next_on_chain[last_on_chain[label_l]] = label_s;
    last_on_chain[label_l] = last_on_chain[label_s];
    size_of_chain[label_l] += size_of_chain[label_s];
  }
  
  delete[] last_on_chain;
  delete[] next_on_chain;
  delete[] size_of_chain;

  // if there are only a few components we caould do brute force
  // *FIXME*
  // if (n - tree_size < 6) { ... }

  const int * component = first_on_chain;
  for (int k = 0; k < m; ++k) {
    if (vars[k]->lb() == vars[k]->ub())
      continue;
    const int i = edges[k].tail;
    const int j = edges[k].head;
    if (component[i] == component[j]) {
      // this can be fixed!
      changed_pos.push_back(k);
      if (sig[i] == sig[j]) {
	new_bd.push_back(0.0);
	new_bd.push_back(0.0);
      } else {
	new_bd.push_back(1.0);
	new_bd.push_back(1.0);
      }
    }
  }
  printf("MC: logical fixing: # of components: %i, fixed %i variables.\n",
	 n - tree_size, static_cast<int>(changed_pos.size()));
  delete[] first_on_chain;
  delete[] sig;
#endif
}
