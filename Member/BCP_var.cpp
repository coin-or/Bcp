// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_buffer.hpp"
#include "BCP_temporary.hpp"
#include "BCP_var.hpp"
#include "BCP_branch.hpp"

//#############################################################################

void
BCP_var_set::set_lb_ub(const BCP_vec<int>& indices,
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
BCP_var_set::set_lb_ub_st(const BCP_vec<BCP_obj_change>& vc)
{
   BCP_vec<BCP_obj_change>::const_iterator chi = vc.begin();
   BCP_vec<BCP_obj_change>::const_iterator lastchi = vc.end();
   iterator var = begin();
   while (chi != lastchi){
      (*var)->change_lb_ub_st(*chi);
      ++var;
      ++chi;
   }
}

//-----------------------------------------------------------------------------

void
BCP_var_set::set_lb_ub_st(BCP_vec<int>::const_iterator pos,
			  const BCP_vec<BCP_obj_change>& vc){
   BCP_vec<BCP_obj_change>::const_iterator chi = vc.begin();
   BCP_vec<BCP_obj_change>::const_iterator lastchi = vc.end();
   while (chi != lastchi){
      operator[](*pos)->change_lb_ub_st(*chi);
      ++pos;
      ++chi;
   }
}

//-----------------------------------------------------------------------------

void
BCP_var_set::deletable(const int bvarnum, BCP_vec<int>& collection){
   collection.clear();
   collection.reserve(size() - bvarnum);
   iterator var = entry(bvarnum);
   for (int i = bvarnum; var != end(); ++var, ++i) {
      if ((*var)->is_to_be_removed())
	 collection.unchecked_push_back(i);
   }
}

//#############################################################################

BCP_var_algo::~BCP_var_algo() {}

void
BCP_var::display(const double val) const
{
  switch (obj_type()) {
  case BCP_CoreObj:
    printf("  Core  var (internal index: %6i                    ) at %.4f\n",
	   _bcpind, val);
    break;
  case BCP_IndexedObj:
    printf("  Indexed var (internal index: %6i, user index: %6i) at %.4f\n",
	   _bcpind,(dynamic_cast<const BCP_var_indexed*>(this))->index(), val);
    break;
  case BCP_AlgoObj:
    printf("  Algo  var (internal index: %6i                    ) at %.4f\n",
	   _bcpind, val);
    break;
  default:
    throw BCP_fatal_error("Untyped object in BCP_solution_gen::display()\n");
  }
}

