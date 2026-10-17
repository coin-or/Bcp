// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_error.hpp"
#include "BCP_branch.hpp"
#include "BCP_buffer.hpp"
#include "OsiSolverInterface.hpp"
#include "BCP_lp_branch.hpp"

BCP_internal_brobj::BCP_internal_brobj(BCP_lp_branching_object& candidate) :
  _child_num(candidate.child_num),
  _var_positions(), _cut_positions(), _var_bounds(), _cut_bounds()
{
  if (candidate.forced_var_pos) {
    _var_positions = *candidate.forced_var_pos;
    _var_bounds = *candidate.forced_var_bd;
  }
  if (candidate.forced_cut_pos) {
    _cut_positions = *candidate.forced_cut_pos;
    _cut_bounds = *candidate.forced_cut_bd;
  }
}


void
BCP_internal_brobj::apply_child_bounds(OsiSolverInterface* lp,
				       int child_ind) const
{
  if (_var_positions.size() > 0) {
    lp->setColSetBounds(_var_positions.begin(), _var_positions.end(),
			var_bounds_child(child_ind));
  }
  if (_cut_positions.size() > 0) {
    lp->setRowSetBounds(_cut_positions.begin(), _cut_positions.end(),
			cut_bounds_child(child_ind));
  }
}

void BCP_internal_brobj::pack(BCP_buffer& buf) const {
  buf.pack(_child_num)
    .pack(_var_positions).pack(_var_bounds)
    .pack(_cut_positions).pack(_cut_bounds);
}

void BCP_internal_brobj::unpack(BCP_buffer& buf) {
  buf.unpack(_child_num)
    .unpack(_var_positions).unpack(_var_bounds)
    .unpack(_cut_positions).unpack(_cut_bounds);
}
