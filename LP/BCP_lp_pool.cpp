// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_lp_pool.hpp"

void
BCP_lp_waiting_row::compute_violation(const BCP_lp_result& lpres){
   const double lhs = _row->dotProduct(lpres.x());
   _violation = std::max( 0.0, std::max(_row->LowerBound()-lhs,
					lhs-_row->UpperBound()));
}

int
BCP_lp_cut_pool::remove_nonviolated(const double etol)
{
   iterator waiting_row = begin();
   int cnt = 0;

   while (waiting_row != end()) {
      if ((*waiting_row)->violation() <= etol) {
	 delete *waiting_row;
	 *waiting_row = back();
	 pop_back();
	 ++cnt;
      } else {
	 ++waiting_row;
      }
   }
   return cnt;
}

//#############################################################################

void
BCP_lp_waiting_col::compute_red_cost(const BCP_lp_result& lpres)
{
   _red_cost = _col->Objective() - _col->dotProduct(lpres.pi());
}

int
BCP_lp_var_pool::remove_positives(const double etol)
{
   iterator waiting_col = begin();
   int cnt = 0;

   while (waiting_col != end()) {
      if ((*waiting_col)->red_cost() >= -etol) {
	 delete *waiting_col;
	 *waiting_col = back();
	 pop_back();
	 ++cnt;
      } else {
	 ++waiting_col;
      }
   }
   return cnt;
}


//#############################################################################

bool BCP_lp_cut_pool::_rows_are_valid = true;

//#############################################################################

bool BCP_lp_var_pool::_cols_are_valid = true;

//#############################################################################

