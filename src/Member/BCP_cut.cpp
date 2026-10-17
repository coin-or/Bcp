// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>

#include "BCP_buffer.hpp"
#include "BCP_cut.hpp"
#include "BCP_branch.hpp"

//#############################################################################

void
BCP_cut_set::set_lb_ub(const BCP_vec<int>& indices,
		       BCP_vec<double>::const_iterator bounds)
{
   BCP_vec<int>::const_iterator ii = indices.begin();
   BCP_vec<int>::const_iterator lastii = indices.end();
   for ( ; ii != lastii; ++ii){
      const double lb = *bounds;
      ++bounds;
      const double ub = *bounds;
      ++bounds;
      operator[](*ii)->change_bounds(lb, ub);
   }
}

//-----------------------------------------------------------------------------

void
BCP_cut_set::set_lb_ub_st(const BCP_vec<BCP_obj_change>& vc)
{
   BCP_vec<BCP_obj_change>::const_iterator chi = vc.begin();
   BCP_vec<BCP_obj_change>::const_iterator lastchi = vc.end();
   iterator cut = begin();
   while (chi != lastchi) {
      (*cut)->change_lb_ub_st(*chi);
      ++cut;
      ++chi;
   }
}

//-----------------------------------------------------------------------------

void
BCP_cut_set::set_lb_ub_st(BCP_vec<int>::const_iterator pos,
			  const BCP_vec<BCP_obj_change>& vc)
{
   BCP_vec<BCP_obj_change>::const_iterator chi = vc.begin();
   BCP_vec<BCP_obj_change>::const_iterator lastchi = vc.end();
   while (chi != lastchi){
      operator[](*pos)->change_lb_ub_st(*chi);
      ++pos;
      ++chi;
   }
}

//-----------------------------------------------------------------------------
#if 0
void
BCP_cut_set::nonzero_slack(int first_to_check, const double * slacks,
			   const double etol, const int ineff_limit,
			   BCP_vec<int>& coll)
{
   const int cutnum = size();
   coll.reserve(cutnum);
   for ( ; first_to_check < cutnum; ++first_to_check) {
      BCP_cut *cut = operator[](first_to_check);
      // Most cuts are removable, so it's not worth to test whether a cut is
      // removable, just test the slackness. Who cares what is the
      // effectiveness count of non-removable cuts.
      if (slacks[first_to_check] > etol) {
	if (cut->decrease_effective_count() <= -ineff_limit) {
	  coll.unchecked_push_back(first_to_check);
	}
      } else {
	cut->increase_effective_count();
      }
   }
}

//-----------------------------------------------------------------------------
void
BCP_cut_set::zero_dual(int first_to_check, const double * duals,
		       const double etol, const int ineff_limit,
		       BCP_vec<int>& coll)
{
   const int cutnum = size();
   coll.reserve(cutnum);
   for ( ; first_to_check < cutnum; ++first_to_check) {
      BCP_cut *cut = operator[](first_to_check);
      // Most cuts are removable, so it's not worth to test whether a cut is
      // removable, just test the slackness. Who cares what is the
      // effectiveness count of non-removable cuts.
      if (abs(duals[first_to_check]) <= etol) {
	if (cut->decrease_effective_count() <= -ineff_limit) {
	  coll.unchecked_push_back(first_to_check);
	}
      }else{
	cut->increase_effective_count();
      }
   }
}
//-----------------------------------------------------------------------------

void
BCP_cut_set::deletable(int bcutnum, BCP_vec<int>& collection) const
{
   const int cutnum = size();
   collection.reserve(cutnum - bcutnum);
   int i;
   for (i = bcutnum; i < cutnum; ++i) {
      BCP_cut *cut = operator[](i);
      if (cut->is_to_be_removed())
	 collection.unchecked_push_back(i);
   }
}
#endif
void
BCP_cut_set::move_deletable_to_pool(const BCP_vec<int>& del_cuts,
				    BCP_vec<BCP_cut*>& pool)
{
   BCP_vec<int>::const_iterator ii = del_cuts.begin();
   BCP_vec<int>::const_iterator lastii = del_cuts.end();
   pool.reserve(pool.size() + del_cuts.size());
   while (ii != lastii) {
      pool.unchecked_push_back(operator[](*ii));
      operator[](*ii) = 0;
      ++ii;
   }
}

//#############################################################################

BCP_cut_algo::~BCP_cut_algo() {}

