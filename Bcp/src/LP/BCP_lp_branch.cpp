// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include <numeric>
#include <algorithm>

#include "CoinSort.hpp"
#include "BCP_vector.hpp"
#include "BCP_lp_branch.hpp"

#include "OsiSolverInterface.hpp"
#include "OsiBranchingObject.hpp"

#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_lp_result.hpp"

//#############################################################################

static void
BCP_reset_pos(BCP_vec<int>& pos, const int start)
{
   int i = pos.size();
   while (--i >= 0) {
      if (pos[i] < 0) {
	 pos[i] = (start - 1) - pos[i];
      }
   }
}

//#############################################################################

static void BCP_reorder_pos(const int child_num,
			    BCP_vec<int>& positions, BCP_vec<double>& bounds)
{
   const int size = positions.size();
   if (size <= 1)
      return;

   BCP_vec<int> perm;
   perm.reserve(size);
   for (int i = 0; i < size; ++i)
      perm.unchecked_push_back(i);
   // order the pair list based on the first entry (pos)
   CoinSort_2(positions.begin(), positions.end(), perm.begin());

   // apply the permutation to each block in bounds
   BCP_vec<double> new_bd;
   new_bd.reserve(bounds.size());
   const BCP_vec<int>::const_iterator lastpos = perm.end();
   for (int i = 0; i < child_num; ++i){
      BCP_vec<double>::const_iterator old_bd = bounds.entry(2 * size * i);
      BCP_vec<int>::const_iterator pos = perm.begin();
      for ( ; pos != lastpos; ++pos) {
	 const int bd_pos = *pos << 1;
	 new_bd.unchecked_push_back(*(old_bd + bd_pos));
	 new_bd.unchecked_push_back(*(old_bd + (bd_pos+1)));
      }
   }
   bounds = new_bd;
}

//#############################################################################

BCP_lp_branching_object::
BCP_lp_branching_object(const BCP_lp_integer_branching_object& o,
			const int* order) :
    child_num(2),
    vars_to_add(0), cuts_to_add(0),
    forced_var_pos(new BCP_vec<int>(1,-1)), forced_cut_pos(0),
    forced_var_bd(new BCP_vec<double>(4,0.0)), forced_cut_bd(0),
    implied_var_pos(0), implied_cut_pos(0),
    implied_var_bd(0), implied_cut_bd(0)
{
    BCP_vec<int>& fvp = *forced_var_pos;
    BCP_vec<double>& fvb = *forced_var_bd;
    fvp[0] = o.originalObject()->columnNumber();
    memcpy(&fvb[0], o.childBounds(order[0]), 2*sizeof(double));
    memcpy(&fvb[2], o.childBounds(order[1]), 2*sizeof(double));
}

//#############################################################################

BCP_lp_branching_object::
BCP_lp_branching_object(const OsiSolverInterface* osi,
			const BCP_lp_sos_branching_object& o,
			const int* order) :
    child_num(2),
    vars_to_add(0), cuts_to_add(0),
    forced_var_pos(0), forced_cut_pos(0),
    forced_var_bd(0), forced_cut_bd(0),
    implied_var_pos(0), implied_cut_pos(0),
    implied_var_bd(0), implied_cut_bd(0)
{
    const OsiSOS* sos = dynamic_cast<const OsiSOS*>(o.originalObject());
    const int * which = sos->members();
    const double * weights = sos->weights();
    const double value = o.value();
    int i;

    const double* clb = osi->getColLower();
    const double* cub = osi->getColUpper();

    const int len = sos->numberMembers();
    forced_var_pos = new BCP_vec<int>(sos->members(), sos->members()+len);
    forced_var_bd  = new BCP_vec<double>(4*len, 0.0);
    BCP_vec<double>& fvb = *forced_var_bd;
    double* downchildBounds = NULL;
    double* upchildBounds = NULL;
    if ( order[0] == 0) {
	downchildBounds = &fvb[0];
	upchildBounds = &fvb[2*len];
    } else {
	downchildBounds = &fvb[2*len];
	upchildBounds = &fvb[0];
    }
    for (i = 0; i < len; ++i) {
	const int pos = which[i];
	downchildBounds[2*i]   = upchildBounds[2*i]   = clb[pos];
	downchildBounds[2*i+1] = upchildBounds[2*i+1] = cub[pos];
    }
    // upper bounds in child 0
    for (i = 0; i < len; ++i) {
	if (weights[i] > value)
	    break;
    }
    assert (i < len);
    for ( ; i < len; ++i) {
	downchildBounds[2*i+1] = 0.0;
    }
    // upper bounds in child 1
    for (i = 0 ; i < len; ++i) {
	if (weights[i] >= value)
	    break;
	else
	    upchildBounds[2*i+1] = 0.0;
    }
    assert ( i < len);
}

//#############################################################################

void
BCP_lp_branching_object::init_pos_for_added(const int added_vars_start,
					    const int added_cuts_start)
{
   if (vars_to_add){
      if (forced_var_pos)
	 BCP_reset_pos(*forced_var_pos, added_vars_start);
      if (implied_var_pos)
	 BCP_reset_pos(*implied_var_pos, added_vars_start);
   }
   if (cuts_to_add){
      if (forced_cut_pos)
	 BCP_reset_pos(*forced_cut_pos, added_cuts_start);
      if (implied_cut_pos)
	 BCP_reset_pos(*implied_cut_pos, added_cuts_start);
   }

   // reorder the positions so that it's increasing everywhere
   if (forced_var_pos)
      BCP_reorder_pos(child_num, *forced_var_pos, *forced_var_bd);
   if (implied_var_pos)
      BCP_reorder_pos(child_num, *implied_var_pos, *implied_var_bd);
   if (forced_cut_pos)
      BCP_reorder_pos(child_num, *forced_cut_pos, *forced_cut_bd);
   if (implied_cut_pos)
      BCP_reorder_pos(child_num, *implied_cut_pos, *implied_cut_bd);
}

//#############################################################################

void
BCP_lp_branching_object::apply_child_bd(OsiSolverInterface* lp,
					const int child_ind) const
{
   if (forced_var_pos) {
      const int len = forced_var_pos->size();
      lp->setColSetBounds(forced_var_pos->begin(), forced_var_pos->end(),
			  forced_var_bd->entry(2 * len * child_ind));
   }
   if (implied_var_pos) {
      const int len = implied_var_pos->size();
      lp->setColSetBounds(implied_var_pos->begin(), implied_var_pos->end(),
			  implied_var_bd->entry(2 * len * child_ind));
   }
   if (forced_cut_pos) {
      const int len = forced_cut_pos->size();
      lp->setRowSetBounds(forced_cut_pos->begin(), forced_cut_pos->end(),
			  forced_cut_bd->entry(2 * len * child_ind));
   }
   if (implied_cut_pos) {
      const int len = implied_cut_pos->size();
      lp->setRowSetBounds(implied_cut_pos->begin(), implied_cut_pos->end(),
			  implied_cut_bd->entry(2 * len * child_ind));
   }
}

//#############################################################################

void
BCP_lp_branching_object::print_branching_info(const int orig_varnum,
					      const double * x,
					      const double * obj) const
{
   printf(" (");
   if (forced_var_pos) {
      const int ind = (*forced_var_pos)[0];
      if (ind < orig_varnum) {
	 printf("%i,%.4f,%.4f", ind, x[ind], obj[ind]);
      } else {
	 // must be a var added in branching
	 printf(";%i,-,-", ind);
      }
      const int size = forced_var_pos->size();
      for (int i = 1; i < size; ++i) {
	 const int ind = (*forced_var_pos)[i];
	 if (ind < orig_varnum) {
	    printf(";%i,%.4f,%.4f", ind, x[ind], obj[ind]);
	 } else {
	    // must be a var added in branching
	    printf(";%i,-,-", ind);
	 }
      }
   }
   printf(" / ");
   if (forced_cut_pos) {
      printf("%i", (*forced_cut_pos)[0]);
      const int size = forced_cut_pos->size();
      for (int i = 1; i < size; ++i)
	 printf(";%i", (*forced_cut_pos)[i]);
   }
   printf(" )");
}

//#############################################################################

void BCP_presolved_lp_brobj::fake_objective_values(const double itlim_objval)
{
   for (int i = _candidate->child_num - 1; i >= 0; --i){
      const int tc = _lpres[i]->termcode();
      if (tc & (BCP_ProvenPrimalInf | BCP_DualObjLimReached)) {
	 _lpres[i]->fake_objective_value(DBL_MAX / 2);
	 continue;
      }
      // *THINK* : what to do in these cases?
      if (tc & (BCP_ProvenDualInf | BCP_PrimalObjLimReached |
		BCP_IterationLimit | BCP_Abandoned | BCP_TimeLimit) ) {
	 _lpres[i]->fake_objective_value(itlim_objval);
	 continue;
      }
   }
}

const bool BCP_presolved_lp_brobj::fathomable(const double objval_limit) const
{
   // If ALL descendants in cand terminated with primal infeasibility
   // or high cost, that proves that the current node can be fathomed.
   for (int i = _candidate->child_num - 1; i >= 0; --i) {
      const int tc = _lpres[i]->termcode();
      if (! ((tc & BCP_ProvenPrimalInf) ||
	     (tc & BCP_DualObjLimReached) ||
	     ((tc & BCP_ProvenOptimal) && _lpres[i]->objval() > objval_limit)))
	 return false;
   }
   return true;
}

const bool BCP_presolved_lp_brobj::had_numerical_problems() const
{
   for (int i = _candidate->child_num - 1; i >= 0; --i)
      if (_lpres[i]->termcode() == BCP_Abandoned)
	 return true;
   return false;
}
