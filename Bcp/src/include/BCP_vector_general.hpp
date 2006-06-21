// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include "BCP_error.hpp"
#include "BCP_vector.hpp"
#include "BCP_vector_sanity.hpp"

//#############################################################################

template <class T> void
BCP_vec<T>::deallocate() {
   if (start) {
      while (finish != start) {
	 BCP_DESTROY(--finish);
      }
      ::operator delete(start);
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::insert_aux(iterator position, const_reference x){
   if (finish != end_of_storage) {
      BCP_CONSTRUCT(finish, *(finish - 1));
      std::copy_backward(position, finish - 1, finish);
      *position = x;
      ++finish;
   } else {
      const size_t len = (2*size() + 0x100);
      iterator tmp = allocate(len);
      iterator tmp_finish = std::uninitialized_copy(start, position, tmp);
      BCP_CONSTRUCT(tmp_finish++, x);
      tmp_finish = std::uninitialized_copy(position, finish, tmp_finish);
      deallocate();
      start = tmp;
      finish = tmp_finish;
      end_of_storage = tmp + len;
   }
}

//#############################################################################

template <class T>
BCP_vec<T>::BCP_vec(const size_t n, const_reference value) :
   start(0), finish(0), end_of_storage(0)
{
   if (n > 0) {
      start = allocate(n);
      end_of_storage = finish = start + n;
      std::uninitialized_fill_n(start, n, value);
   }
}

template <class T>
BCP_vec<T>::BCP_vec(const_iterator first, const_iterator last) :
   start(0), finish(0), end_of_storage(0)
{
   const size_t len = last - first;
   if (len > 0) {
      start = allocate(len);
      end_of_storage = finish = std::uninitialized_copy(first, last, start);
   }
}

template <class T>
BCP_vec<T>::BCP_vec(const T* x, const size_t num) :
   start(0), finish(0), end_of_storage(0)
{
   if (num > 0) {
      finish = start = allocate(num);
      const T* const lastx = x + num;
      while (x != lastx)
	 BCP_CONSTRUCT(finish++, *x++);
      end_of_storage = finish;
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::keep(iterator pos) {
   iterator oldfinish = finish;
   finish = std::copy(pos, pos + 1, start);
   BCP_DESTROY_RANGE(finish, oldfinish);
}

template <class T> void
BCP_vec<T>::keep(iterator first, iterator last) {
   iterator oldfinish = finish;
   finish = std::copy(first, last, start);
   BCP_DESTROY_RANGE(finish, oldfinish);
}

//#############################################################################

template <class T> void
BCP_vec<T>::erase(iterator position) {
   if (position + 1 != finish)
      std::copy(position + 1, finish, position);
   BCP_DESTROY(--finish);
}

template <class T> void
BCP_vec<T>::erase(iterator first, iterator last) {
   iterator oldfinish = finish;
   finish = std::copy(last, finish, first);
   BCP_DESTROY_RANGE(finish, oldfinish);
}

//#############################################################################
//#############################################################################

template <class T> void
BCP_vec<T>::reserve(const size_t n){
   if (capacity() < n) {
      iterator tmp = allocate(n);
      iterator tmp_finish = std::uninitialized_copy(start, finish, tmp);
      deallocate();
      start = tmp;
      finish = tmp_finish;
      end_of_storage = start + n;
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::swap(BCP_vec<T>& x) {
   std::swap(start, x.start);
   std::swap(finish, x.finish);
   std::swap(end_of_storage, x.end_of_storage);
}
   
//#############################################################################

template <class T> BCP_vec<T>&
BCP_vec<T>::operator=(const BCP_vec<T>& x){
   if (&x != this) {
      const size_t x_size = x.size();
      if (x_size > capacity()) {
	 deallocate();
	 start = allocate(x_size);
	 end_of_storage = start + x_size;
	 finish = std::uninitialized_copy(x.begin(), x.end(), start);
      } else {
	 if (x_size < size()) {
	    iterator oldfinish = finish;
	    finish = std::copy(x.begin(), x.end(), start);
	    BCP_DESTROY_RANGE(finish, oldfinish);
	 } else {
	    std::copy(x.begin(), x.entry(size()), start);
	    finish = std::uninitialized_copy(x.entry(size()), x.end(), finish);
	 }
      }
   }
   return *this;
}

//#############################################################################
// need the void* here, since we occasionally want to copy out of a buffer

template <class T> void
BCP_vec<T>::assign(const void* x, const size_t num){
   if (num > capacity()){
      deallocate();
      start = allocate(num);
      end_of_storage = start + num;
   } else {
      BCP_DESTROY_RANGE(start, finish);
   }
   T entry;
   finish = start;
   const char* charx = static_cast<const char*>(x);
   for (int i = num; i > 0; --i) {
      memcpy(&entry, charx, sizeof(T));
      BCP_CONSTRUCT(finish++, entry);
      charx += sizeof(T);
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::insert(iterator position, const void* first, const size_t n){
   if (n == 0) return;
   T entry;
   const char* charx = static_cast<const char*>(first);
   if ((size_t) (end_of_storage - finish) >= n) {
      const size_t to_move = finish - position;
      if (to_move <= n) {
	 std::uninitialized_copy(position, finish, position + n);
	 finish += n;
	 size_t i = n;
	 for ( ; i > to_move; --i) {
	    memcpy(&entry, charx, sizeof(T));
	    BCP_CONSTRUCT(position++, entry);
	    charx += sizeof(T);
	 }
	 for ( ; i > 0; --i) {
	    memcpy(&entry, charx, sizeof(T));
	    *position++ = entry; 
	    charx += sizeof(T);
	 }
      } else {
	 std::uninitialized_copy(finish - n, finish, finish);
	 std::copy_backward(position, finish - n, finish);
	 finish += n;
	 for (int i = n; i > 0; --i) {
	    memcpy(&entry, charx, sizeof(T));
	    *position++ = entry; 
	    charx += sizeof(T);
	 }
      }
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      iterator tmp_finish = std::uninitialized_copy(start, position, tmp);
      for (int i = n; i > 0; --i) {
	 memcpy(&entry, charx, sizeof(T));
	 BCP_CONSTRUCT(tmp_finish++, entry);
	 charx += sizeof(T);
      }
      tmp_finish = std::uninitialized_copy(position, finish, tmp_finish);
      deallocate();
      start = tmp;
      finish = tmp_finish;
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::insert(iterator position,
		   const_iterator first, const_iterator last){
   if (first == last) return;
   const size_t n = last - first;
   if ((size_t) (end_of_storage - finish) >= n) {
      const size_t to_move = finish - position;
      if (to_move <= n) {
	 std::uninitialized_copy(position, finish, position + n);
	 std::copy(first, first + to_move, position);
	 std::uninitialized_copy(first + to_move, last, finish);
      } else {
	 std::uninitialized_copy(finish - n, finish, finish);
	 std::copy_backward(position, finish - n, finish);
	 std::copy(first, last, position);
      }
      finish += n;
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      iterator tmp_finish = std::uninitialized_copy(start, position, tmp);
      tmp_finish = std::uninitialized_copy(first, last, tmp_finish);
      tmp_finish = std::uninitialized_copy(position, finish, tmp_finish);
      deallocate();
      start = tmp;
      finish = tmp_finish;
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::insert(iterator position, const size_t n, const_reference x) {
   if (n == 0) return;
   if ((size_t) (end_of_storage - finish) >= n) {
      const size_t to_move = finish - position;
      if (to_move <= n) {
	 std::uninitialized_copy(position, finish, position + n);
	 std::fill_n(position, to_move, x);
	 std::uninitialized_fill_n(finish, n - to_move, x);
      } else {
	 std::uninitialized_copy(finish - n, finish, finish);
	 std::copy_backward(position, finish - n, finish);
	 std::fill_n(position, n, x);
      }
      finish += n;
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      iterator tmp_finish = std::uninitialized_copy(start, position, tmp);
      std::uninitialized_fill_n(tmp_finish, n, x);
      tmp_finish += n;
      tmp_finish = std::uninitialized_copy(position, finish, tmp_finish);
      deallocate();
      start = tmp;
      finish = tmp_finish;
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template <class T> typename BCP_vec<T>::iterator
BCP_vec<T>::insert(iterator position, const_reference x){
   const size_t n = position - start;
   if (finish != end_of_storage && position == finish) {
      BCP_CONSTRUCT(finish++, x);
   } else {
      insert_aux(position, x);
   }
   return start + n;
}

//#############################################################################


template <class T> void
BCP_vec<T>::keep_by_index(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_keep_by_index(positions.begin(), positions.end());
}
//-----------------------------------------------------------------------------

template <class T> void
BCP_vec<T>::unchecked_keep_by_index(const BCP_vec<int>& positions){
   unchecked_keep_by_index(positions.begin(), positions.end());
}

//#############################################################################

template <class T> void
BCP_vec<T>::erase_by_index(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_erase_by_index(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template <class T> void
BCP_vec<T>::unchecked_erase_by_index(const BCP_vec<int>& positions){
   unchecked_erase_by_index(positions.begin(), positions.end());
}

//#############################################################################

template <class T> void
BCP_vec<T>::keep_by_index(BCP_vec<int>::const_iterator firstpos,
			  BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_keep_by_index(firstpos, lastpos);
}

//-----------------------------------------------------------------------------

template <class T> void
BCP_vec<T>::unchecked_keep_by_index(BCP_vec<int>::const_iterator firstpos,
				    BCP_vec<int>::const_iterator lastpos) {
   if (firstpos == lastpos) {
      clear();
   } else {
      iterator target = start;
      while ( firstpos != lastpos )
	 *target++ = operator[](*firstpos++);
      BCP_DESTROY_RANGE(target, finish);
      finish = target;
   }
}

//#############################################################################

template <class T> void
BCP_vec<T>::erase_by_index(BCP_vec<int>::const_iterator firstpos,
			   BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_erase_by_index(firstpos, lastpos);
}

//-----------------------------------------------------------------------------

template <class T> void
BCP_vec<T>::unchecked_erase_by_index(BCP_vec<int>::const_iterator firstpos,
				     BCP_vec<int>::const_iterator lastpos) {
   if (firstpos == lastpos)
      return;
   --lastpos;
   int source;
   iterator target = entry(*firstpos);
   while (firstpos != lastpos){
      source = *firstpos + 1;
      ++firstpos;
      if (*firstpos > source)
	 target = std::copy( entry(source), entry(*firstpos), target );
   }
   iterator oldfinish = finish;
   finish = std::copy( entry(*firstpos + 1), end(), target );
   BCP_DESTROY_RANGE(finish, oldfinish);
}

//#############################################################################

template <class T> void
BCP_vec<T>::update(const BCP_vec<int>& positions,
		   const BCP_vec<T>& values){
   if (positions.size() != values.size())
      throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_update(positions, values);
}

//-----------------------------------------------------------------------------

template <class T> void
BCP_vec<T>::unchecked_update(const BCP_vec<int>& positions,
			     const BCP_vec<T>& values){
   if (positions.size() == 0)
      return;
   const_iterator val = values.begin();
   BCP_vec<int>::const_iterator pos = positions.begin();
   const BCP_vec<int>::const_iterator lastpos = positions.end();
   while (pos != lastpos)
      operator[](*pos++) = *val++;
}

//#############################################################################

template <class T>
bool operator==(const BCP_vec<T>& x, const BCP_vec<T>& y) {
   return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
}

template <class T>
bool operator< (BCP_vec<T>& x, BCP_vec<T>& y) {
   return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}
