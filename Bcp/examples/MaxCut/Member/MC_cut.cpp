// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_buffer.hpp"

#include "MC_cut.hpp"

static const int cycle_cut = 0;
static const int explicit_dense_cut = 1;

//#############################################################################

void
MC_cycle_cut::pack(BCP_buffer& buf) const
{
  const MC_cut_t type = MC_cut_t__cycle;
  buf.pack(type).pack(pos_edges).pack(edges, cycle_len);
}

MC_cycle_cut::MC_cycle_cut(BCP_buffer& buf) :
  BCP_cut_algo(-BCP_DBL_MAX, 0.0), cycle_len(0), pos_edges(0), edges(0)
{
  buf.unpack(pos_edges).unpack(edges, cycle_len);
  set_ub(pos_edges-1.0);
}

//#############################################################################

void
MC_explicit_dense_cut::pack(BCP_buffer& buf) const
{
  const MC_cut_t type = MC_cut_t__explicit_dense;
  buf.pack(type).pack(rhs).pack(coeffs, varnum);
}

MC_explicit_dense_cut::MC_explicit_dense_cut(BCP_buffer& buf) :
  BCP_cut_algo(-BCP_DBL_MAX, 0.0), rhs(0.0), coeffs(0), varnum(0)
{
  buf.unpack(rhs).unpack(coeffs, varnum);
}

//#############################################################################

BCP_MemPool MC_cycle_cut::memPool(sizeof(MC_cycle_cut));
BCP_MemPool MC_explicit_dense_cut::memPool(sizeof(MC_explicit_dense_cut));
