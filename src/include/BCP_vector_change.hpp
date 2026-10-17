// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_VECTOR_CHANGE_H
#define _BCP_VECTOR_CHANGE_H

// This file is fully docified.

#include "CoinHelperFunctions.hpp"
#include "BCP_enum.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"

/** This class stores a vector explicitly or relatively to another vector.
    The class can be used when we want to store a sequence of vectors in as
    small a space as possible. The first vector must be stored explicitly, the
    others can be either explicit ones or stored with respect to the previous
    vector.
*/

template <class T> class BCP_vec_change {
 private:
  /** The storage type of the the vector. */
  BCP_storage_t _storage;
  /** Empty in case of explicit storage. Otherwise these entries are to be
      deleted from the parent vector. */
  BCP_vec<int>  _del_pos;
  /** Empty in case of explicit storage. Otherwise these entries are to be
      changed in the parent vector <em>after</em> the ones to be deleted are
      gone. */
  BCP_vec<int>  _change_pos;
  /** In case of explicit storage these are the values in the vector.
      Otherwise the first _change_pos.size() entries are the new
      values for the changed entries and the rest are new entries. */
  BCP_vec<T>    _values;

 public:
  /**@name Constructors and destructor */
  /*@{*/
    /** Construct a vector change that's empty and is of the given storage.
	If the storage is explicit then this creates an explicit vector with no
	entry. If it is relative, then the data describes that there is no
	change with respect to the parent vector. */
    BCP_vec_change(const BCP_storage_t st) : _storage(st) {}

    /** Construct an explicit description describing the vector bounded by the
	two iterators. */
    BCP_vec_change(const T* first, const T* last) :
    _storage(BCP_Storage_Explicit) {
      _values.append(first, last);
    }

    /** Construct a relative description. 
	@param new_vec the vector that should result after the change is
	       applied to
	@param old_vec the original vector
        @param del_pos specifies which entries are to be deleted from old_vec
               before the change in this object can be applied.
    */
    BCP_vec_change(const BCP_vec_change<T>& old_vec,
		   const BCP_vec_change<T>& new_vec,
		   const BCP_vec<int>& del_pos) :
    _storage(BCP_Storage_WrtParent), _del_pos(del_pos) {
      const BCP_vec<T>& ov = old_vec.explicit_vector();
      const BCP_vec<T>& nv = new_vec.explicit_vector();
      const int new_size = nv.size();
      const int old_size = ov.size();
      const int del_size = del_pos.size();
      int i, j, k; // runs on old_vec/new_vec/del_pos
      for (i = 0, j = 0, k = 0; i < old_size && k < del_size; ++i) {
	if (del_pos[k] == i) {
	  ++k;
	  continue;
	}
	if (ov[i] != nv[j]) {
	  _change_pos.push_back(j);
	}
	++j;
      }
      if (old_size - i > new_size - j)
	throw BCP_fatal_error("BCP_vec_change::BCP_vec_change() : \n  \
old_vec has entries not in new_vec but not listed in del_pos.\n");
      for ( ; i < old_size; ++i, ++j) {
	if (ov[i] != nv[j]) {
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
      
    /** Construct a relative description. Same as the previous constructor,
	except that there is an extra argument, the epsilon tolerance for
	equality testing. This makes sense only if \c is \c double, so this
	constructor should be used only in that case.
    */
    BCP_vec_change(const BCP_vec_change<T>& old_vec,
		   const BCP_vec_change<T>& new_vec,
		   const BCP_vec<int>& del_pos, const double etol) :
    _storage(BCP_Storage_WrtParent), _del_pos(del_pos) {
      const BCP_vec<T>& ov = old_vec.explicit_vector();
      const BCP_vec<T>& nv = new_vec.explicit_vector();
      const int new_size = nv.size();
      const int old_size = ov.size();
      const int del_size = del_pos.size();
      int i, j, k; // runs on old_vec/new_vec/del_pos
      for (i = 0, j = 0, k = 0; i < old_size && k < del_size; ++i) {
	if (del_pos[k] == i) {
	  ++k;
	  continue;
	}
	if (CoinAbs(ov[i] - nv[j]) > etol) {
	  _change_pos.push_back(j);
	}
	++j;
      }
      if (old_size - i > new_size - j)
	throw BCP_fatal_error("BCP_vec_change::BCP_vec_change() : \n  \
old_vec has entries not in new_vec but not listed in del_pos.\n");
      for ( ; i < old_size; ++i, ++j) {
	if (CoinAbs(ov[i] - nv[j]) > etol) {
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

    /** Construct the object by unpacking it from a buffer. */
    BCP_vec_change(BCP_buffer& buf) {
      unpack(buf);
    }

    /** The destructor need not do anything. */
    ~BCP_vec_change() {}
  /*@}*/

  /**@name Query methods */
  /*@{*/
    /** Return the storage type of the vector */
    BCP_storage_t storage() const { return _storage; }

    /** Return a const reference to the vector if it is explicitly stored,
	otherwise throw an exception */
    const BCP_vec<T>& explicit_vector() const {
      if (_storage != BCP_Storage_Explicit)
	throw BCP_fatal_error("\
BCP_vec_change::explicit_vector() : non-explicit storage!\n");
      return _values;
    }
      
    /** Return how much memory it'll take to pack this info. It is
	used when comparing which sort of storage is smaller */
    int storage_size() const {
      return (_del_pos.size() + _change_pos.size()) * sizeof(int) +
	_values.size() * sizeof(T);
    }
      
  /*@}*/

  /**@name Modifying methods */
  /*@{*/
    /** Update the current vector with the argument vector.
	If the argument vector is explicit then just replace the current
	vector. If it is relative and the current vector is explicit, then
	perform the update. Otherwise throw an exception (the operation is
	impossible.) */
    void update(const BCP_vec_change<T>& change) {
      switch (change.storage()) {
      case BCP_Storage_Explicit:
	_storage = BCP_Storage_Explicit;
	_del_pos.clear();
	_change_pos.clear();
	_values = change._values;
	return;

      case BCP_Storage_NoData:
	_storage = BCP_Storage_NoData;
	_del_pos.clear();
	_change_pos.clear();
	_values.clear();
	return;

      default: // must be BCP_Storage_WrtParent:
	if (_storage != BCP_Storage_Explicit)
	  throw BCP_fatal_error("\
trying to update a non-explicit storage with another non-explicit!\n");
	_values.erase_by_index(change._del_pos);
	const int ch_size = change._change_pos.size();
	for (int i = 0; i < ch_size; ++i) {
	  _values[change._change_pos[i]] = change._values[i];
	}
	_values.append(change._values.entry(ch_size), change._values.end());
      }
    }
  /*@}*/

  /**@name Packing and unpacking */
  /*@{*/
    /** Pack the data into a buffer. */
    void pack(BCP_buffer& buf) const {
      const int st = _storage;
      buf.pack(st).pack(_del_pos).pack(_change_pos).pack(_values);
    }
    /** Unpack the data from a buffer. */
    void unpack(BCP_buffer& buf) {
      _del_pos.clear();
      _change_pos.clear();
      _values.clear();
      int st;
      buf.unpack(st).unpack(_del_pos).unpack(_change_pos).unpack(_values);
      _storage = static_cast<BCP_storage_t>(st);
    }
  /*@}*/
};

#endif
