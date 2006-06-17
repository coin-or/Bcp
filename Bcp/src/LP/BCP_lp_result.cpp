// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "CoinHelperFunctions.hpp"
#include "OsiSolverInterface.hpp"
#include "BCP_lp_result.hpp"

static inline int BCP_getTermcode(OsiSolverInterface& lp)
{
   int tc = 0;
   tc |= (lp.isAbandoned() ? BCP_Abandoned : 0);
   tc |= (lp.isProvenOptimal() ? BCP_ProvenOptimal : 0);
   tc |= (lp.isProvenPrimalInfeasible() ? BCP_ProvenPrimalInf : 0);
   tc |= (lp.isProvenDualInfeasible() ? BCP_ProvenDualInf : 0);
   tc |= (lp.isPrimalObjectiveLimitReached() ? BCP_PrimalObjLimReached : 0);
   tc |= (lp.isDualObjectiveLimitReached() ? BCP_DualObjLimReached : 0);
   tc |= (lp.isIterationLimitReached() ? BCP_IterationLimit : 0);
  
  return tc;
}

void
BCP_lp_result::get_results(OsiSolverInterface& lp_solver)
{
  lp_solver.getDblParam(OsiPrimalTolerance, _primal_tolerance);
  lp_solver.getDblParam(OsiDualTolerance, _dual_tolerance);
  delete[] _x;
  delete[] _pi;
  delete[] _dj;
  delete[] _lhs;
  _x = 0;
  _pi = 0;
  _dj = 0;
  _lhs = 0;

  _termcode = BCP_getTermcode(lp_solver);
  if ((_termcode & BCP_Abandoned) == 0) {
    _iternum = lp_solver.getIterationCount();
    _objval = lp_solver.getObjValue();
    
    const int colnum = lp_solver.getNumCols();
    _x   = new double[colnum];
    CoinDisjointCopyN(lp_solver.getColSolution(), colnum, _x);
    _dj  = new double[colnum];
    CoinDisjointCopyN(lp_solver.getReducedCost(), colnum, _dj);

    const int rownum = lp_solver.getNumRows();
    _pi  = new double[rownum];
    CoinDisjointCopyN(lp_solver.getRowPrice(), rownum, _pi);
    _lhs = new double[rownum];
    CoinDisjointCopyN(lp_solver.getRowActivity(), rownum, _lhs);
  }
}
