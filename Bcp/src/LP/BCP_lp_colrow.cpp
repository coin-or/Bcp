// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "CoinHelperFunctions.hpp"
#include "CoinWarmStartBasis.hpp"

#include "BCP_problem_core.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_functions.hpp"

//#############################################################################
// want to put the most violated constraints (most positive violation) to the
// back, so return true if wrow0's violation is smaller than wrow1's, i.e.,
// wrow0 is less desirable.

inline static int BCP_compare_waiting_row_ptr(const BCP_lp_waiting_row* wrow0,
					      const BCP_lp_waiting_row* wrow1){
   return wrow0->violation() < wrow1->violation();
}

//#############################################################################
// want to put the most attractive variables (most negative reduced cost) to
// the back, so return true if wcol0's red cost is bigger (less negative) than
// wcol1's, i.e., wcol0 is less desirable.

inline static int BCP_compare_waiting_col_ptr(const BCP_lp_waiting_col* wcol0,
					      const BCP_lp_waiting_col* wcol1){
   return wcol0->red_cost() > wcol1->red_cost();
}

//#############################################################################

bool BCP_lp_fix_vars(BCP_lp_prob& p)
{
   const BCP_lp_result& lpres = *p.lp_result;

   BCP_var_set& vars = p.node->vars;
   const int varnum = vars.size();
   BCP_cut_set& cuts = p.node->cuts;
   const int cutnum = cuts.size();

   int i;
   int newly_changed = 0;

   BCP_lp_check_ub(p);

   if (lpres.dj()) {
     p.user->reduced_cost_fixing(lpres.dj(), lpres.x(),
				 p.ub() - lpres.objval() - p.granularity(),
				 vars, newly_changed);
   }
   if (newly_changed > 0 && p.param(BCP_lp_par::LpVerb_VarTightening)) {
      printf("LP: Reduced cost fixing has changed the bounds on %i vars\n",
	     newly_changed);
   }
      
   p.var_bound_changes_since_logical_fixing += newly_changed;

   BCP_vec<BCP_obj_status> var_status;
   var_status.reserve(varnum);
   for (i = 0; i < varnum; ++i)
     var_status.unchecked_push_back(vars[i]->status());

   BCP_vec<BCP_obj_status> cut_status;
   cut_status.reserve(cutnum);
   for (i = 0; i < cutnum; ++i)
     cut_status.unchecked_push_back(cuts[i]->status());

   bool lost_primal_feasibility = false;

   BCP_vec<int> changed_pos;
   BCP_vec<double> new_bd;

   p.user->logical_fixing(lpres, vars, cuts, var_status, cut_status,
			  p.var_bound_changes_since_logical_fixing,
			  changed_pos, new_bd);

   if (2 * changed_pos.size() != new_bd.size())
      throw BCP_fatal_error("logical_fixing() returned uneven vectors\n");

   const int change_num = changed_pos.size();
   if (change_num > 0){
      int bd_changes = 0;
      const double * x = lpres.x();
      const double petol = lpres.primalTolerance();
      for (i = 0; i < change_num; ++i) {
	 // check that the new bounds are not looser than the old ones.
	 // throw an exception if not.
	 const int pos = changed_pos[i];
	 const double old_lb = vars[pos]->lb();
	 const double old_ub = vars[pos]->ub();
	 const double new_lb = new_bd[2*i];
	 const double new_ub = new_bd[2*i+1];
	 if (old_lb > new_lb + petol || old_ub < new_ub - petol) {
	    throw BCP_fatal_error("logical fixing enlarged feas region!\n");
	 }
	 // check that the new bounds are actually tighter than the old
	 // ones. Print a warning if not.
	 if (new_lb < old_lb + petol  && new_ub > old_ub - petol) {
	     printf("LP: BCP_lp_fix_vars(): WARNING: no change in bounds.\n");
	     continue;
	 }
	 // otherwise register the bounds for change and check if primal
	 // feasibility is violated (dual feas can't be hurt: bound changes
	 // can hurt dual feasibility).
	 new_bd[2*bd_changes] = new_lb;
	 new_bd[2*bd_changes+1] = new_ub;
	 changed_pos[bd_changes] = pos;
	 ++bd_changes;
	 if ((x[pos] < new_lb - petol) || (x[pos] > new_ub + petol))
	    lost_primal_feasibility = true;
      }
      changed_pos.erase(changed_pos.entry(bd_changes), changed_pos.end());
      new_bd.erase(new_bd.entry(2*bd_changes), new_bd.end());

      p.var_bound_changes_since_logical_fixing = 0;

      if (bd_changes > 0) {
	 p.lp_solver->setColSetBounds(changed_pos.begin(), changed_pos.end(),
				      new_bd.begin());
	 vars.set_lb_ub(changed_pos, new_bd.begin());
	 if (bd_changes > 0 && p.param(BCP_lp_par::LpVerb_VarTightening)) {
	    printf("LP: Logical fixing has changed the bounds on %i vars\n",
		   bd_changes);
	 }
      }
   }

   return lost_primal_feasibility;
}

//#############################################################################

static void
BCP_lp_reset_positions(const BCP_vec<int>& deletable, BCP_vec<int>& pos,
		       const bool error_if_deletable)
{
   int i, j;
   const int delnum = deletable.size();
   const int posnum = pos.size();
   for (i = 0, j = 0; i < delnum && j < posnum; ) {
#ifdef PARANOID
      if (error_if_deletable && deletable[i] == pos[j])
	 throw BCP_fatal_error("Bad pos in BCP_lp_reset_positions()\n");
#endif
      if (deletable[i] < pos[j]) {
	 ++i;
      } else {
	 pos[j] -= i;
	 ++j;
      }
   }
   for ( ; j < posnum; ++j) {
      pos[j] -= delnum;
   }
}

//#############################################################################

void
BCP_delete_unwanted_candidates(const int num, const int added_num,
			       const BCP_vec<int>* pos,
			       BCP_vec<int>& deletable)
{
   deletable.erase(std::lower_bound(deletable.begin(), deletable.end(),
				    num-added_num), deletable.end());
   for (int i = num - added_num; i < num; ++i) {
      deletable.push_back(i);
   }
   if (pos && !pos->empty()) {
      BCP_vec<int>::iterator ifirst = 
	 std::lower_bound(deletable.begin(), deletable.end(), (*pos)[0]);
      BCP_vec<int>::iterator ilast = 
	 std::upper_bound(ifirst, deletable.end(), pos->back());
      deletable.erase(ifirst, ilast);
   }
}

//#############################################################################
void BCP_lp_delete_cols_and_rows(BCP_lp_prob& p,
				 BCP_lp_branching_object* can,
				 const int added_colnum,
				 const int added_rownum,
				 const bool from_fathom,
				 const bool force_delete)
{
   // If can is set then this function is invoked after branching is decided
   // upon (in this case force_delete will be true, too). In this case we have
   // to update the various positions in can.

   int del_num;

   BCP_lp_result& lpres = *p.lp_result;
   BCP_var_set& vars = p.node->vars;
   const size_t varnum = vars.size();
   BCP_cut_set& cuts = p.node->cuts;
   const size_t cutnum = cuts.size();

   OsiSolverInterface* lp = p.lp_solver;
   CoinWarmStart* ws = lp->getWarmStart();
   CoinWarmStartBasis* bas = dynamic_cast<CoinWarmStartBasis*>(ws);

   BCP_vec<int> deletable;
   deletable.reserve(CoinMax(varnum, cutnum));

   // find out which columns could be deleted
   p.user->select_vars_to_delete(lpres, vars, cuts, from_fathom, deletable);

   // clean up deletable
   del_num = deletable.size();
   if (del_num > 0) {
      // Make sure deletable is sorted and has unique entries
      std::sort(deletable.begin(), deletable.end());
      deletable.erase(std::unique(deletable.begin(), deletable.end()),
		      deletable.end());
      del_num = deletable.size();
      if (can && added_colnum) {
	 // make sure that all but can's added cols are to be deleted
	 BCP_delete_unwanted_candidates(varnum, added_colnum,
					can->forced_var_pos, deletable);
	 del_num = deletable.size();
      }
      if (bas) {
	 // If there's a basis throw out those from deletable which are in the
	 // basis UNLESS they are unwanted candidates
	 BCP_vec<int> do_not_delete;
	 for (int i = 0; i < del_num; ++i) {
	    const int ind = deletable[i];
	    if (can && ind >= (int)(varnum - added_colnum))
	       continue;
	    const CoinWarmStartBasis::Status stat = bas->getStructStatus(ind);
	    if (stat == CoinWarmStartBasis::basic ||
		stat == CoinWarmStartBasis::isFree) {
	       // No, this can't be deleted
	       do_not_delete.push_back(i);
	    }
	 }
	 if (! do_not_delete.empty()) {
	    deletable.unchecked_erase_by_index(do_not_delete);
	    del_num = deletable.size();
	 }
      }
   }

   int min_by_frac =
     static_cast<int>(varnum * p.param(BCP_lp_par::DeletedColToCompress_Frac));
   int min_to_del = force_delete ?
     0 : CoinMax(p.param(BCP_lp_par::DeletedColToCompress_Min), min_by_frac);

   if (del_num > min_to_del) {
      if (p.param(BCP_lp_par::LpVerb_MatrixCompression))
	 printf("LP:   Deleting %i columns from the matrix.\n", del_num);
      if (can) {
	 if (can->forced_var_pos)
	    BCP_lp_reset_positions(deletable, *can->forced_var_pos, true);
	 if (can->implied_var_pos)
	    BCP_lp_reset_positions(deletable, *can->implied_var_pos, false);
      }
      if (bas) {
	 bas->deleteColumns(del_num, &deletable[0]);
      }
      p.lp_solver->deleteCols(del_num, &deletable[0]);
      purge_ptr_vector_by_index(dynamic_cast< BCP_vec<BCP_var*>& >(vars),
				deletable.begin(), deletable.end());
      p.local_cut_pool->rows_are_valid(false);
   }

   // Now do the same for rows
   deletable.clear();
   p.user->select_cuts_to_delete(lpres, vars, cuts, from_fathom, deletable);

   // clean up deletable
   del_num = deletable.size();
   if (del_num > 0) {
      // Make sure deletable is sorted and has unique entries
      std::sort(deletable.begin(), deletable.end());
      deletable.erase(std::unique(deletable.begin(), deletable.end()),
		      deletable.end());
      del_num = deletable.size();
      if (can && added_rownum) {
	 // make sure that all but can's added rows are to be deleted
	 BCP_delete_unwanted_candidates(cutnum, added_rownum,
					can->forced_cut_pos, deletable);
	 del_num = deletable.size();
      }
      if (bas) {
	 // If there's a basis throw out those from deletable which are not in
	 // the basis UNLESS they are unwanted candidates
	 BCP_vec<int> do_not_delete;
	 for (int i = 0; i < del_num; ++i) {
	    const int ind = deletable[i];
	    if (can && ind >= (int)(cutnum - added_rownum))
	       continue;
	    const CoinWarmStartBasis::Status stat = bas->getArtifStatus(ind);
	    if (stat != CoinWarmStartBasis::basic) {
	       // No, this can't be deleted
	       do_not_delete.push_back(i);
	    }
	 }
	 if (! do_not_delete.empty()) {
	    deletable.unchecked_erase_by_index(do_not_delete);
	    del_num = deletable.size();
	 }
      }
   }

   min_by_frac =
     static_cast<int>(cutnum * p.param(BCP_lp_par::DeletedRowToCompress_Frac));
   min_to_del = force_delete ?
     0 : CoinMax(p.param(BCP_lp_par::DeletedRowToCompress_Min), min_by_frac);

   if (del_num > min_to_del) {
      if (p.param(BCP_lp_par::LpVerb_MatrixCompression))
	 printf("LP:   Deleting %i rows from the matrix.\n", del_num);
      if (can) {
	 if (can->forced_cut_pos)
	    BCP_lp_reset_positions(deletable, *can->forced_cut_pos, true);
	 if (can->implied_cut_pos)
	    BCP_lp_reset_positions(deletable, *can->implied_cut_pos, false);
      } else {
	 if (p.param(BCP_lp_par::BranchOnCuts))
	    p.node->cuts.move_deletable_to_pool(deletable, p.slack_pool);
      }
      if (bas) {
	 bas->deleteRows(del_num, &deletable[0]);
      }
      p.lp_solver->deleteRows(deletable.size(), &deletable[0]);
      purge_ptr_vector_by_index(dynamic_cast< BCP_vec<BCP_cut*>& >(cuts),
				deletable.begin(), deletable.end());
      p.node->lb_at_cutgen.erase_by_index(deletable);
      p.local_var_pool->cols_are_valid(false);
   }

   if (bas) {
      if (varnum != vars.size() || cutnum != cuts.size()) {
	 // if anything got deleted set the modified basis
	 lp->setWarmStart(bas);
      }
      delete bas;
   }
}

//#############################################################################

void BCP_lp_adjust_row_effectiveness(BCP_lp_prob& p)
{
  if (p.param(BCP_lp_par::IneffectiveConstraints) != BCP_IneffConstr_None){
    const BCP_lp_result& lpres = *p.lp_result;

    BCP_cut_set& cuts = p.node->cuts;
    int i, ineff = 0;
    const int cutnum = cuts.size();

    switch (p.param(BCP_lp_par::IneffectiveConstraints)){
    case BCP_IneffConstr_None:
      break;
    case BCP_IneffConstr_NonzeroSlack:
      {
	const double * lhs = p.lp_result->lhs();
	const double * rlb = p.lp_solver->getRowLower();
	const double * rub = p.lp_solver->getRowUpper();
	const double petol = lpres.primalTolerance();
	for (i = p.core->cutnum(); i < cutnum; ++i) {
	  BCP_cut *cut = cuts[i];
	  if (rlb[i] + petol < lhs[i] && lhs[i] < rub[i] - petol) {
	    cut->decrease_effective_count();
	    if (! cut->is_non_removable())
	      ++ineff;
	  } else {
	    cut->increase_effective_count();
	  }
	}
      }
      break;
    case BCP_IneffConstr_ZeroDualValue:
      {
	const double * pi = p.lp_result->pi();
	const double detol = lpres.dualTolerance();
	for (i = p.core->cutnum(); i < cutnum; ++i) {
	  BCP_cut *cut = cuts[i];
	  if (CoinAbs(pi[i]) < detol) {
	    cut->decrease_effective_count();
	    if (! cut->is_non_removable())
	      ++ineff;
	  } else {
	    cut->increase_effective_count();
	  }
	}
      }
      break;
    }
    if (p.param(BCP_lp_par::LpVerb_RowEffectivenessCount))
      printf("LP:   Row effectiveness: rownum: %i ineffective: %i\n",
	     static_cast<int>(p.node->cuts.size()), ineff);
  }
}

//#############################################################################

int BCP_lp_add_from_local_cut_pool(BCP_lp_prob& p)
{
   // First find out how many do we want to add
   BCP_lp_cut_pool& cp = *p.local_cut_pool;
   size_t added_rows =
      std::min<size_t>(size_t(p.param(BCP_lp_par::MaxCutsAddedPerIteration)),
		       cp.size());

   if (added_rows == 0)
     return 0;

   if (added_rows < cp.size()) {
      /* The violations in the cut pool are: max(0, max(lb-lhs, lhs-ub)).
	 If the parameter says so then normalize it, so that we get the
	 distance of the fractional point from the cutting planes */
      switch (p.param(BCP_lp_par::CutViolationNorm)) {
       case BCP_CutViolationNorm_Plain:
	 break;
       case BCP_CutViolationNorm_Distance:
	 for (int i = cp.size() - 1; i >= 0; --i) {
	    BCP_lp_waiting_row& cut = *cp[i];
	    if (cut.violation() > 0) {
	       cut.set_violation(cut.violation()/sqrt(cut.row()->normSquare()));
	    }
	 }
	 break;
       case BCP_CutViolationNorm_Directional: {
	  const double* c = p.lp_solver->getObjCoefficients();
	  const int n = p.lp_solver->getNumCols();
	  const double cnorm = sqrt(std::inner_product(c, c+n, c, 0.0));
	  for (int i = cp.size() - 1; i >= 0; --i) {
	     BCP_lp_waiting_row& cut = *cp[i];
	     if (cut.violation() > 0) {
		cut.set_violation(cut.violation() * cnorm /
				  fabs(cut.row()->dotProduct(c)));
	     }
	  }
       }
	 break;
       default:
	 throw BCP_fatal_error("LP: No violation norm specified!\n");
      }
      // Sort the waiting rows if we have more than we want to add. The most
      // violated ones will be at the end with this sort.
      std::sort(cp.begin(), cp.end(), BCP_compare_waiting_row_ptr);
   }

#ifdef PARANOID
   if (! cp.rows_are_valid())
      throw BCP_fatal_error
	 ("BCP_lp_add_from_local_cut_pool() : rows are not valid.\n");
#endif

   // Now collect the rows corresponding to the cuts to be added
   // (those in [first, last) )
   BCP_lp_cut_pool::const_iterator first = cp.entry(cp.size() - added_rows);
   BCP_lp_cut_pool::const_iterator last = cp.end();
   BCP_lp_cut_pool::const_iterator cpi = first - 1;

   // create the row set
   const CoinPackedVectorBase** rows =
     new const CoinPackedVectorBase*[added_rows];

   BCP_vec<double> rlb;
   rlb.reserve(added_rows);

   BCP_vec<double> rub;
   rub.reserve(added_rows);

   p.node->cuts.reserve(p.node->cuts.size() + added_rows);

   cpi = first - 1;
   while (++cpi != last) {
     BCP_row * row = (*cpi)->row();
     BCP_cut * cut = (*cpi)->cut();
     *rows++ = row;
     rlb.unchecked_push_back(row->LowerBound());
     rub.unchecked_push_back(row->UpperBound());
     cut->set_effective_count(0);
     p.node->cuts.unchecked_push_back(cut);
   }
   rows -= added_rows;
   p.lp_solver->addRows(added_rows, rows, rlb.begin(), rub.begin());
   cpi = first - 1;
   while (++cpi != last) {
     (*cpi)->clear_cut();
   }

   delete[] rows;

   int leftover = cp.size() - added_rows;
   const double frac = p.param(BCP_lp_par::MaxLeftoverCutFrac);
   const int maxnum = p.param(BCP_lp_par::MaxLeftoverCutNum);
   if (frac < 1.0) {
     leftover = (frac <= 0.0) ? 0 : static_cast<int>(frac * leftover);
   }
   if (leftover > maxnum)
     leftover = maxnum > 0 ? maxnum : 0;
   
   purge_ptr_vector(dynamic_cast<BCP_vec<BCP_lp_waiting_row*>&>(cp), 
		    cp.entry(leftover), cp.end());

   p.node->lb_at_cutgen.insert(p.node->lb_at_cutgen.end(), added_rows,
			       p.lp_result->objval());
   return added_rows;
}

//#############################################################################

int BCP_lp_add_from_local_var_pool(BCP_lp_prob& p)
{
   // First find out how many do we want to add
   BCP_lp_var_pool& vp = *p.local_var_pool;
   size_t added_cols =
      std::min<size_t>(size_t(p.param(BCP_lp_par::MaxVarsAddedPerIteration)),
		       vp.size());

   if (added_cols == 0)
     return 0;

   if (added_cols < vp.size()) {
      // Sort the waiting cols if we have more than we want to add. The most
      // violated ones will be at the end with this sort.
      std::sort(vp.begin(), vp.end(), BCP_compare_waiting_col_ptr);
   }

#ifdef PARANOID
   if (! vp.cols_are_valid())
      throw BCP_fatal_error
	 ("BCP_lp_add_from_local_var_pool() : cols are not valid.\n");
#endif

   // Now collect the colums corresponding to the variables to be added
   // (those in [first, last) )
   BCP_lp_var_pool::const_iterator first = vp.entry(vp.size() - added_cols);
   BCP_lp_var_pool::const_iterator last = vp.end();
   BCP_lp_var_pool::const_iterator vpi = first - 1;

   // create the col set
   const CoinPackedVectorBase** cols =
     new const CoinPackedVectorBase*[added_cols];

   BCP_vec<double> clb;
   clb.reserve(added_cols);

   BCP_vec<double> cub;
   cub.reserve(added_cols);

   BCP_vec<double> obj;
   obj.reserve(added_cols);

   p.node->vars.reserve(p.node->vars.size() + added_cols);

   vpi = first - 1;
   while (++vpi != last) {
     BCP_col* col = (*vpi)->col();
     BCP_var* var = (*vpi)->var();
     p.node->vars.unchecked_push_back(var);
     // make certain that the bounds of integer vars is integer...
     if (var->var_type() != BCP_ContinuousVar) {
       var->set_lb(floor(var->lb()+1e-8));
       var->set_ub(ceil(var->ub()-1e-8));
     }
     *cols++ = col;
     clb.unchecked_push_back(col->LowerBound());
     cub.unchecked_push_back(col->UpperBound());
     obj.unchecked_push_back(col->Objective());
   }
   cols -= added_cols;
   p.lp_solver->addCols(added_cols, cols,
			clb.begin(), cub.begin(), obj.begin());
   vpi = first - 1;
   while (++vpi != last) {
     (*vpi)->clear_var();
   }

   delete[] cols;
   
   purge_ptr_vector(dynamic_cast<BCP_vec<BCP_lp_waiting_col*>&>(vp), 
		    vp.entry(vp.size() - added_cols), vp.end());

   return added_cols;
}

//#############################################################################
