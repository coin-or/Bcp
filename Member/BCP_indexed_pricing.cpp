// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include "BCP_buffer.hpp"
#include "BCP_error.hpp"
#include "BCP_indexed_pricing.hpp"

//-----------------------------------------------------------------------------

void
BCP_indexed_pricing_list::swap(BCP_indexed_pricing_list& x) {
   std::swap(_pr_status, x._pr_status);
   std::swap(_storage, x._storage);
   _del_pos.swap(x._del_pos);
   _indices.swap(x._indices);
}

void
BCP_indexed_pricing_list::update(const BCP_indexed_pricing_list& change) {
   switch (change.get_storage()){
    case BCP_Storage_Explicit:
    case BCP_Storage_NoData:
      operator=(change);
      break;
    case BCP_Storage_WrtParent:
      // in this case 'this' must be the parent, and _storage must be
      // explicit
      if (_storage != BCP_Storage_Explicit)
	 throw BCP_fatal_error("\
BCP_indexed_pricing_list::update() : Bad storage.\n");
      break;
    default:
      throw BCP_fatal_error("\
BCP_indexed_pricing_list::update() : unrecognized change list storage.\n");
   }
   // we come here only if
   // change.storage == WrtParent && _storage == Explicit
   _indices.erase_by_index(change._del_pos);
   _indices.append(change._indices);
}

BCP_indexed_pricing_list*
BCP_indexed_pricing_list::
as_change(const BCP_indexed_pricing_list& old_list) const
{
   // note that whatever is new in 'this' list must be at the end of this.
   if (old_list.get_storage() != BCP_Storage_Explicit ||
       _storage != BCP_Storage_Explicit)
      throw BCP_fatal_error("\
BCP_indexed_pricing_list::as_change() : Bad storage.\n");
   BCP_indexed_pricing_list* change = new BCP_indexed_pricing_list;
   BCP_vec<int>::const_iterator oldi = old_list._indices.begin();
   BCP_vec<int>::const_iterator lastoldi = old_list._indices.end();
   BCP_vec<int>::const_iterator newi = _indices.begin();
   BCP_vec<int>::const_iterator lastnewi = _indices.end();
   BCP_vec<int>& ch_del = change->_del_pos;
   ch_del.reserve(old_list._indices.size());
   int pos;
   for (pos = 0; oldi != lastoldi && newi != lastnewi; ++oldi, ++pos) {
      if (*oldi < *newi){ // a deleted one from old
	 ch_del.unchecked_push_back(pos);
      }else{ // they must be equal
	 ++newi;
      }
   }
   // whatever is left in old is deleted
   for ( ; oldi != lastoldi; ++oldi, ++pos)
      ch_del.unchecked_push_back(pos);
   // whaever is left in new as the added
   change->_indices.insert(change->_indices.end(), newi, lastnewi);
   return change;
}

//#############################################################################

int
BCP_indexed_pricing_list::pack_size() const {
   return sizeof(_pr_status) + sizeof(_storage) + 2 * sizeof(int) +
      sizeof(int) * (_del_pos.size() + _indices.size());
}

void
BCP_indexed_pricing_list::pack(BCP_buffer& buf) const {
   buf.pack(_pr_status);
   if (_pr_status & BCP_PriceIndexedVars){
      buf.pack(_storage);
      if (_storage != BCP_Storage_NoData)
	 buf.pack(_del_pos).pack(_indices);
   }
}

void
BCP_indexed_pricing_list::unpack(BCP_buffer& buf) {
   buf.unpack(_pr_status);
   if (_pr_status & BCP_PriceIndexedVars){
      buf.unpack(_storage);
      if (_storage != BCP_Storage_NoData)
	 buf.unpack(_del_pos).unpack(_indices);
   }
}
