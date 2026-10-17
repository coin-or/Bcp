// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "CoinWarmStartPrimalDual.hpp"
#include "BCP_warmstart_primaldual.hpp"

//#############################################################################

CoinWarmStart*
BCP_warmstart_primaldual::convert_to_CoinWarmStart() const
{
  if (storage() != BCP_Storage_Explicit)
    return NULL;

  const BCP_vec<double>& primal = _primal.explicit_vector();
  const BCP_vec<double>& dual = _dual.explicit_vector();
  return new CoinWarmStartPrimalDual(primal.size(), dual.size(),
				     &primal[0], &dual[0]);
}

//#############################################################################

void
BCP_warmstart_primaldual::update(const BCP_warmstart* const change)
{
  const BCP_warmstart_primaldual* const pdch =
    dynamic_cast<const BCP_warmstart_primaldual* const>(change);
  if (!pdch)
    throw BCP_fatal_error("BCP_warmstart_primaldual::update\n  \
Trying to update a BCP_warmstart_primaldual with a different warmstart!\n");

  _primal.update(pdch->_primal);
  _dual.update(pdch->_dual);
}


//#############################################################################

BCP_warmstart* 
BCP_warmstart_primaldual::as_change(const BCP_warmstart* const old_ws,
				    const BCP_vec<int>& del_vars,
				    const BCP_vec<int>& del_cuts,
				    const double petol,
				    const double detol) const
{
  const BCP_warmstart_primaldual* const ows =
    dynamic_cast<const BCP_warmstart_primaldual* const>(old_ws);
  if (!ows)
      throw BCP_fatal_error("\
Trying to describe a BCP_warmstart_primaldual as a change to a different ws!\n");

  if (storage() != BCP_Storage_Explicit)
    throw BCP_fatal_error("\
BCP_warmstart_primaldual::as_change : current data is not explicit!\n");

  if (ows->storage() == BCP_Storage_WrtParent)
    throw BCP_fatal_error("\
BCP_warmstart_primaldual::as_change : old_ws is WrtParent!\n");

  if (ows->storage() == BCP_Storage_NoData)
    return new BCP_warmstart_primaldual(*this);

  // Both are explicit
  BCP_warmstart_primaldual* ws = new BCP_warmstart_primaldual();
  new (&ws->_primal) BCP_vec_change<double>(ows->_primal, _primal,
					    del_vars, petol);
  if (ws->_primal.storage_size() >= _primal.storage_size())
    ws->_primal = _primal;

  new (&ws->_dual) BCP_vec_change<double>(ows->_dual, _dual, del_cuts, detol);
  if (ws->_dual.storage_size() >= _dual.storage_size())
    ws->_dual = _dual;

  return ws;
}
