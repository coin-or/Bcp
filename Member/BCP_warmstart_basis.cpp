// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "OsiWarmStartBasis.hpp"
#include "BCP_warmstart_basis.hpp"

//#############################################################################

OsiWarmStart*
BCP_warmstart_basis::convert_to_OsiWarmStart() const
{
  if (storage() != BCP_Storage_Explicit)
    return NULL;

  const BCP_vec<char>& vst = _var_stat.explicit_vector();
  const BCP_vec<char>& cst = _cut_stat.explicit_vector();
  return
    new OsiWarmStartBasis(vst.size(), cst.size(), vst.begin(), cst.begin());
}

//#############################################################################

BCP_storage_t
BCP_warmstart_basis::storage() const {
  // The storage is WrtParent if at least one status set is WrtParent
  const BCP_storage_t vst = _var_stat.storage();
  const BCP_storage_t cst = _cut_stat.storage();
  if (vst == BCP_Storage_WrtParent || cst == BCP_Storage_WrtParent)
    return BCP_Storage_WrtParent;
  // Otherwise Explicit if at least one status set is Explicit
  if (vst == BCP_Storage_Explicit || cst == BCP_Storage_Explicit)
    return BCP_Storage_Explicit;
  // Otherwise it must be NoData
  return BCP_Storage_NoData;
}

//#############################################################################

void
BCP_warmstart_basis::update(const BCP_warmstart* const change)
{
  const BCP_warmstart_basis* ch =
    dynamic_cast<const BCP_warmstart_basis*>(change);
  if (! ch)
    throw BCP_fatal_error("BCP_warmstart_basis::update\n  \
Can't update a BCP_warmstart_basis with a different warmstart.\n");

  _var_stat.update(ch->_var_stat);
  _cut_stat.update(ch->_cut_stat);
}


//#############################################################################

BCP_warmstart* 
BCP_warmstart_basis::as_change(const BCP_warmstart* const old_ws,
			       const BCP_vec<int>& del_vars,
			       const BCP_vec<int>& del_cuts,
			       const double petol,
			       const double detol) const
{
  const BCP_warmstart_basis* const ows =
    dynamic_cast<const BCP_warmstart_basis* const>(old_ws);
  if (!ows)
    throw BCP_fatal_error("BCP_warmstart_basis::as_change\n  \
trying to describe a BCP_warmstart_basis as a change to a different ws!\n");

  if (storage() != BCP_Storage_Explicit)
    throw BCP_fatal_error("BCP_warmstart_basis::as_change\n  \
current data is not explicit!\n");

  if (ows->storage() == BCP_Storage_WrtParent)
    throw BCP_fatal_error("\
BCP_warmstart_dual::as_change : old_ws is WrtParent!\n");

  if (ows->storage() == BCP_Storage_NoData)
    return new BCP_warmstart_basis(*this);

  // Both are explicit
  BCP_warmstart_basis* ws = new BCP_warmstart_basis();

  new (&ws->_var_stat) BCP_vec_change<char>(ows->_var_stat, _var_stat,
					    del_vars);
  if (ws->_var_stat.storage_size() > ows->_var_stat.storage_size())
    ws->_var_stat = ows->_var_stat;

  new (&ws->_cut_stat) BCP_vec_change<char>(ows->_cut_stat, _cut_stat,
					    del_cuts);
  if (ws->_cut_stat.storage_size() > ows->_cut_stat.storage_size())
    ws->_cut_stat = ows->_cut_stat;

  return ws;
}
