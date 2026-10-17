// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "CoinHelperFunctions.hpp"
#include "OsiSolverInterface.hpp"
#include "BCP_lp_result.hpp"

void
BCP_lp_result::get_results(OsiSolverInterface& lp)
{
  lp.getDblParam(OsiPrimalTolerance, _primal_tolerance);
  lp.getDblParam(OsiDualTolerance, _dual_tolerance);
  lp.getStrParam(OsiSolverName, _solvername);
  delete[] _x;
  delete[] _pi;
  delete[] _dj;
  delete[] _lhs;
  _x = 0;
  _pi = 0;
  _dj = 0;
  _lhs = 0;

  _termcode = 0;
  _termcode |= (lp.isAbandoned() ? BCP_Abandoned : 0);
  _termcode |= (lp.isProvenOptimal() ? BCP_ProvenOptimal : 0);
  _termcode |= (lp.isProvenPrimalInfeasible() ? BCP_ProvenPrimalInf : 0);
  _termcode |= (lp.isProvenDualInfeasible() ? BCP_ProvenDualInf : 0);
  if (_solvername != "Ipopt") {
    _termcode |= (lp.isPrimalObjectiveLimitReached() ? BCP_PrimalObjLimReached : 0);
  }
  _termcode |= (lp.isDualObjectiveLimitReached() ? BCP_DualObjLimReached : 0);
  _termcode |= (lp.isIterationLimitReached() ? BCP_IterationLimit : 0);
   
  if ((_termcode & BCP_Abandoned) == 0) {
    _iternum = lp.getIterationCount();
    _objval = lp.getObjValue();
    
    const int colnum = lp.getNumCols();
    _x   = new double[colnum];
    CoinDisjointCopyN(lp.getColSolution(), colnum, _x);
    
    if (_solvername == "Ipopt") {
      _dj = NULL;
    } else {
      _dj  = new double[colnum];
      CoinDisjointCopyN(lp.getReducedCost(), colnum, _dj);
    }

    const int rownum = lp.getNumRows();
    _pi  = new double[rownum];
    CoinDisjointCopyN(lp.getRowPrice(), rownum, _pi);
    _lhs = new double[rownum];
    CoinDisjointCopyN(lp.getRowActivity(), rownum, _lhs);
  }
}
