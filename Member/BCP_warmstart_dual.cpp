// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "OsiWarmStartDual.hpp"
#include "BCP_warmstart_dual.hpp"

//#############################################################################

OsiWarmStart*
BCP_warmstart_dual::convert_to_OsiWarmStart() const
{
  if (storage() != BCP_Storage_Explicit)
    return NULL;

  const BCP_vec<double>& dual = _dual.explicit_vector();
  return new OsiWarmStartDual(dual.size(), dual.begin());
}

//#############################################################################

void
BCP_warmstart_dual::update(const BCP_warmstart* const change)
{
  const BCP_warmstart_dual* const dch =
    dynamic_cast<const BCP_warmstart_dual* const>(change);
  if (!dch)
    throw BCP_fatal_error("BCP_warmstart_dual::update\n  \
Trying to update a BCP_warmstart_dual with a different warmstart!\n");

  _dual.update(dch->_dual);
}


//#############################################################################

BCP_warmstart* 
BCP_warmstart_dual::as_change(const BCP_warmstart* const old_ws,
			      const BCP_vec<int>& del_vars,
			      const BCP_vec<int>& del_cuts,
			      const double petol,
			      const double detol) const
{
  const BCP_warmstart_dual* const ows =
    dynamic_cast<const BCP_warmstart_dual* const>(old_ws);
  if (!ows)
      throw BCP_fatal_error("\
Trying to describe a BCP_warmstart_dual as a change to a different ws!\n");

  if (storage() != BCP_Storage_Explicit)
    throw BCP_fatal_error("\
BCP_warmstart_dual::as_change : current data is not explicit!\n");

  if (ows->storage() == BCP_Storage_WrtParent)
    throw BCP_fatal_error("\
BCP_warmstart_dual::as_change : old_ws is WrtParent!\n");

  if (ows->storage() == BCP_Storage_NoData)
    return new BCP_warmstart_dual(*this);

  // Both are explicit
  BCP_warmstart_dual* ws = new BCP_warmstart_dual();
  new (&ws->_dual) BCP_vec_change<double>(ows->_dual, _dual, del_cuts, 1e-4);
  if (ws->_dual.storage_size() >= _dual.storage_size())
    ws->_dual = _dual;

  return ws;
}
