// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "OsiWarmStartDual.hpp"
#include "OsiWarmStartBasis.hpp"

#include "BCP_warmstart_dual.hpp"
#include "BCP_warmstart_basis.hpp"

#include "BCP_lp_functions.hpp"

BCP_warmstart*
BCP_lp_convert_OsiWarmStart(BCP_lp_prob& p, OsiWarmStart*& warmstart)
{
  if (warmstart == NULL)
    return NULL;

  BCP_warmstart* bcp_ws = NULL;

  {
    const OsiWarmStartDual* ws =
      dynamic_cast<const OsiWarmStartDual*>(warmstart);
    if (ws != NULL) {
      const int size = ws->size();
      const double* dual = ws->dual();
      bcp_ws = new BCP_warmstart_dual(dual, dual+size);
    }
  }

  {
    const OsiWarmStartBasis* ws =
      dynamic_cast<const OsiWarmStartBasis*>(warmstart);
    if (ws != NULL) {
      const int vnum = ws->getNumStructural();
      const int cnum = ws->getNumArtificial();
      bcp_ws = new BCP_warmstart_basis(ws->getStructuralStatus(),
				       ws->getStructuralStatus() + vnum,
				       ws->getArtificialStatus(),
				       ws->getArtificialStatus() + cnum);
    }
  }

  delete warmstart;
  warmstart = NULL;

  return bcp_ws;
}
