// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>

#include "BCP_vector_change.hpp"

#include "BCP_error.hpp"
#include "BCP_buffer.hpp"

//#############################################################################

BCP_dblvec_change::BCP_dblvec_change(BCP_buffer& buf)
{
   unpack(buf);
}

//#############################################################################

BCP_dblvec_change::BCP_dblvec_change(const double* first, const double* last) :
  _storage(BCP_Storage_Explicit)
{
  _values.append(first, last);
}

BCP_dblvec_change::BCP_dblvec_change(const BCP_dblvec_change& old_vec,
				     const BCP_dblvec_change& new_vec,
				     const BCP_vec<int>& del_pos,
				     const double etol) :
   _storage(BCP_Storage_WrtParent), _del_pos(del_pos)
{
   if (old_vec.storage() != BCP_Storage_Explicit ||
       new_vec.storage() != BCP_Storage_Explicit) {
      throw BCP_fatal_error("\
One of the vecs is not explicit when creating relative dblvec_change.\n");
   }

   const BCP_vec<double>& ov = old_vec.explicit_vector();
   const BCP_vec<double>& nv = new_vec.explicit_vector();
   const int new_size = nv.size();
   const int old_size = ov.size();
   const int del_size = del_pos.size();
   int i; // runs on old_vec
   int j; // runs on new_vec
   int k; // runs on del_pos
   for (i = 0, j = 0, k = 0; i < old_size && k < del_size; ++i) {
      if (del_pos[k] == i) {
	 ++k;
	 continue;
      }
      if (abs(ov[i] - nv[j]) > etol) {
	 _change_pos.push_back(j);
      }
      ++j;
   }
   for ( ; i < old_size; ++i, ++j) {
      if (abs(ov[i] - nv[j]) > etol) {
	 _change_pos.push_back(j);
      }
   }
   const int change_size = _change_pos.size();
   _values.reserve(change_size + new_size - j);
   for (i = 0; i < change_size; ++i) {
      _values.unchecked_push_back(nv[_change_pos[i]]);
   }
   _values.append(nv.entry(j), nv.end());
}

//#############################################################################

const BCP_vec<double>&
BCP_dblvec_change::explicit_vector() const
{
   if (_storage != BCP_Storage_Explicit)
      throw BCP_fatal_error("explicit_vector : non-explicit storage!\n");
   return _values;
}

//#############################################################################

void
BCP_dblvec_change::update(const BCP_dblvec_change& change)
{
   if (change.storage() == BCP_Storage_Explicit) {
      _storage = BCP_Storage_Explicit;
      _del_pos.clear();
      _change_pos.clear();
      _values = change._values;
      return;
   }

   if (change.storage() == BCP_Storage_NoData) {
      _storage = BCP_Storage_NoData;
      _del_pos.clear();
      _change_pos.clear();
      _values.clear();
      return;
   }

   if (_storage != BCP_Storage_Explicit)
      throw BCP_fatal_error("\
trying to update a non-explicit storage with another non-explicit!\n");

   _values.erase_by_index(change._del_pos);
   const int change_size = change._change_pos.size();
   for (int i = 0; i < change_size; ++i) {
      _values[change._change_pos[i]] = change._values[i];
   }
   _values.append(change._values.entry(change_size), change._values.end());
}

//#############################################################################

int
BCP_dblvec_change::storage_size() const
{
   return (_del_pos.size() + _change_pos.size()) * sizeof(int) +
      _values.size() * sizeof(double);
}

//#############################################################################

void
BCP_dblvec_change::pack(BCP_buffer& buf) const
{
   const int st = _storage;
   buf.pack(st)
      .pack(_del_pos)
      .pack(_change_pos)
      .pack(_values);
}

//#############################################################################

void
BCP_dblvec_change::unpack(BCP_buffer& buf)
{
   _del_pos.clear();
   _change_pos.clear();
   _values.clear();

   int st;
   buf.unpack(st)
      .unpack(_del_pos)
      .unpack(_change_pos)
      .unpack(_values);
   _storage = static_cast<BCP_storage_t>(st);
}   
