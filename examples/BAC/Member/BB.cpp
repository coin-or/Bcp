// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BB.hpp"
#include "CoinPackedMatrix.hpp"

/****************************************************************************/
BB_prob::BB_prob() :
  EPSILON(1e-8),
  rownum(0), colnum(0),
  integer(0), clb(0), cub(0), obj(0),
  rlb_core(0), rub_core(0), rlb_indexed(0), rub_indexed(0),
  core(0), indexed(0) {}

/****************************************************************************/
BB_prob::~BB_prob()
{
  delete[] integer;
  delete[] clb;
  delete[] cub;
  delete[] obj;
  delete[] rlb_core;
  delete[] rub_core;
  delete[] rlb_indexed;
  delete[] rub_indexed;
  delete core;
  delete indexed;
}
