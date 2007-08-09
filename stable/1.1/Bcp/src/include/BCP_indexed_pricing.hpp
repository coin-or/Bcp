// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_INDEXED_PRICING_H
#define _BCP_INDEXED_PRICING_H

#include "BCP_enum.hpp"
#include "BCP_vector.hpp"

class BCP_buffer;

class BCP_indexed_pricing_list {
private:
   inline void clean() { _del_pos.clear(); _indices.clear(); }
private:
   BCP_pricing_status _pr_status;
   BCP_storage_t _storage;
   BCP_vec<int> _del_pos;
   BCP_vec<int> _indices;
public:
   BCP_indexed_pricing_list() :
      _pr_status(BCP_PriceNothing), _storage(BCP_Storage_Explicit),
      _del_pos(), _indices() {}
   ~BCP_indexed_pricing_list() {}

   // default assignment operator and copy constructors are fine

   inline BCP_pricing_status get_status() const { return _pr_status; }
   inline BCP_storage_t get_storage() const{ return _storage; }
   inline const BCP_vec<int>& get_indices() const { return _indices; }

   inline void empty(const BCP_storage_t storage) {
      clean();
      _storage = storage;
   }

   inline void set_status(BCP_pricing_status new_stat) {
     _pr_status = new_stat; }
   inline void set_indices(BCP_vec<int>& new_indices) {
     _indices.swap(new_indices); }

   void swap(BCP_indexed_pricing_list& x);
   void update(const BCP_indexed_pricing_list& change); 
   BCP_indexed_pricing_list*
   as_change(const BCP_indexed_pricing_list& old_list) const;

   int pack_size() const;      // *INLINE ?*
   void pack(BCP_buffer& buf) const;      // *INLINE ?*
   void unpack(BCP_buffer& buf);      // *INLINE ?*
};

#endif
