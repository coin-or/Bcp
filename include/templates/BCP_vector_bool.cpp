// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include "BCP_error.hpp"
#include "BCP_vector.hpp"
#include "BCP_vector_sanity.hpp"

//#############################################################################

template<> void
BCP_vec<bool>::deallocate() {
   if (start) {
      ::operator delete(start);
   }
}

//#############################################################################

template<> void
BCP_vec<bool>::insert_aux(iterator position, const_reference x){
   const size_t after_pos = finish - position;
   if (finish != end_of_storage) {
      memmove(position+1, position, after_pos * sizeof(bool));
      *position = x;
      ++finish;
   } else {
      const size_t len = (2*size() + 0x1000) * sizeof(bool);
      iterator tmp = allocate(len);
      const size_t until_pos = position - start;
      memmove(tmp, start, until_pos * sizeof(bool));
      iterator tmp_finish = tmp + until_pos;
      *tmp_finish++ = x;
      memmove(tmp_finish, position, after_pos * sizeof(bool));
      deallocate();
      start = tmp;
      finish = start + (until_pos + 1 + after_pos);
      end_of_storage = tmp + len;
   }
}

//#############################################################################

template <>
BCP_vec<bool>::BCP_vec(const size_t n, const_reference value) :
   start(0), finish(0), end_of_storage(0)
{
   if (n > 0) {
      start = finish = allocate(n);
      end_of_storage = start + n;
      while (finish != end_of_storage)
	 *finish++ = value;
   }
}

template <>
BCP_vec<bool>::BCP_vec(const_iterator first, const_iterator last) :
   start(0), finish(0), end_of_storage(0)
{
   const size_t len = last - first;
   if (len > 0) {
      start = allocate(len);
      memcpy(start, first, len * sizeof(bool));
      end_of_storage = finish = start + len;
   }
}

template <>
BCP_vec<bool>::BCP_vec(const bool* x, const size_t num) :
   start(0), finish(0), end_of_storage(0)
{
   if (num > 0) {
      start = allocate(num);
      memcpy(start, x, num * sizeof(bool));
      end_of_storage = finish = start + num;
   }
}

//#############################################################################

template <> inline void
BCP_vec<bool>::push_back(const_reference x) {
   if (finish != end_of_storage)
      *finish++ = x;
   else
      insert_aux(finish, x);
}

template <> inline void
BCP_vec<bool>::unchecked_push_back(const_reference x) {
   *finish++ = x;
}

template <> inline void
BCP_vec<bool>::pop_back() {
   --finish;
}

//#############################################################################

template <> void
BCP_vec<bool>::keep(iterator pos) {
   *start = *pos;
   finish = start + 1;
}

template <> void
BCP_vec<bool>::keep(iterator first, iterator last) {
   const size_t len = last - first;
   memmove(start, first, len * sizeof(bool));
   finish = start + len;
}

//#############################################################################

template <> void
BCP_vec<bool>::erase(iterator position) {
   if (position + 1 != finish)
      memmove(position, position + 1, ((finish-position) - 1) * sizeof(bool));
   --finish;
}

template <> void
BCP_vec<bool>::erase(iterator first, iterator last) {
   if (first != last && last != finish)
      memmove(first, last, (finish - last) * sizeof(bool));
   finish -= (last - first);
}

//#############################################################################
//#############################################################################

template<> void
BCP_vec<bool>::reserve(const size_t n){
   if (capacity() < n) {
      iterator tmp = allocate(n);
      const size_t oldsize = size();
      if (oldsize > 0)
	 memcpy(tmp, start, oldsize * sizeof(bool));
      deallocate();
      start = tmp;
      finish = start + oldsize;
      end_of_storage = start + n;
   }
}

//#############################################################################

template<> void
BCP_vec<bool>::swap(BCP_vec<bool>& x) {
   std::swap(start, x.start);
   std::swap(finish, x.finish);
   std::swap(end_of_storage, x.end_of_storage);
}
   
//#############################################################################

template<> BCP_vec<bool>&
BCP_vec<bool>::operator=(const BCP_vec<bool>& x){
   if (&x != this) {
      const size_t x_size = x.size();
      if (x_size > capacity()) {
	 deallocate();
	 start = allocate(x_size);
	 end_of_storage = start + x_size;
      }
      if (x_size > 0)
	 memcpy(start, x.start, x_size * sizeof(bool));
      finish = start + x_size;
   }
   return *this;
}

//#############################################################################
// these two members serve to copy out entries from a buffer.

template<> void
BCP_vec<bool>::assign(const void* x, const size_t num){
   if (num > capacity()){
      deallocate();
      start = allocate(num);
      end_of_storage = start + num;
   }
   memcpy(start, x, num * sizeof(bool));
   finish = start + num;
}

template<> void
BCP_vec<bool>::insert(bool* position, const void* first, const size_t n)
{
   if (n == 0) return;
   if ((size_t) (end_of_storage - finish) >= n) {
      if (finish != position)
	 memmove(position + n, position, (finish - position) * sizeof(bool));
      memcpy(position, first, n * sizeof(bool));
      finish += n;
   } else {
      const size_t new_size = (2*size() + n) * sizeof(bool);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(bool));
      memcpy(tmp + until_pos, first, n * sizeof(bool));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(bool));
      deallocate();
      start = tmp;
      finish = start + (until_pos + n + after_pos);
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template<> void
BCP_vec<bool>::insert(iterator position,
			const_iterator first, const_iterator last){
   if (first == last) return;
   const size_t n = last - first;
   if ((size_t) (end_of_storage - finish) >= n) {
      memmove(position + n, position, (finish - position) * sizeof(bool));
      memcpy(position, first, n * sizeof(bool));
      finish += n;
   } else {
      const size_t new_size = (2*size() + n) * sizeof(bool);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(bool));
      memcpy(tmp + until_pos, first, n * sizeof(bool));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(bool));
      deallocate();
      start = tmp;
      finish = start + (until_pos + n + after_pos);
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template<> void
BCP_vec<bool>::insert(iterator position, const size_t n, const_reference x) {
   if (n == 0) return;
   if ((size_t) (end_of_storage - finish) >= n) {
      memmove(position + n, position, (finish - position) * sizeof(bool));
      finish += n;
      for (int i = n; i > 0; --i)
	 *position++ = x;
   } else {
      const size_t new_size = (2*size() + n) * sizeof(bool);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(bool));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(bool));
      deallocate();
      start = tmp;
      finish = start + (until_pos + n + after_pos);
      end_of_storage = tmp + new_size;
      position = tmp + until_pos;
      for (int i = n; i > 0; --i)
	 *position++ = x;
   }
}

//#############################################################################

template<> BCP_vec<bool>::iterator
BCP_vec<bool>::insert(iterator position, const_reference x){
   const size_t n = position - start;
   if (finish != end_of_storage && position == finish) {
      *finish++ = x;
   } else
      insert_aux(position, x);
   return start + n;
}

//#############################################################################

#if 0
template<> void
BCP_vec<bool>::unchecked_keep(BCP_vec<int>::const_iterator firstpos,
				BCP_vec<int>::const_iterator lastpos) {
   if (firstpos == lastpos) {
      clear();
   } else {
      iterator target = start;
      while ( firstpos != lastpos )
	 *target++ = operator[](*firstpos++);
      finish = target;
   }
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::unchecked_erase(BCP_vec<int>::const_iterator firstpos,
				 BCP_vec<int>::const_iterator lastpos) {
   if (firstpos == lastpos)
      return;
   --lastpos;
   int source;
   iterator target = entry(*firstpos);
   while (firstpos != lastpos){
      source = *firstpos + 1;
      ++firstpos;
      if (*firstpos > source) {
	 memmove(target, start + source, (*firstpos - source) * sizeof(bool));
	 target += (*firstpos - source);
      }
   }
   source = *firstpos + 1;
   int oldsize = size();
   memmove(target, start + source, (oldsize - source) * sizeof(bool));
   finish = target + (oldsize - source);
}

//#############################################################################

template<> void
BCP_vec<bool>::unchecked_keep(const BCP_vec<int>& positions){
   unchecked_keep(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::keep(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_keep(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::keep(BCP_vec<int>::const_iterator firstpos,
		      BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_keep(firstpos, lastpos);
}

//#############################################################################

template<> void
BCP_vec<bool>::unchecked_erase(const BCP_vec<int>& positions){
   unchecked_erase(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::erase(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_erase(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::erase(BCP_vec<int>::const_iterator firstpos,
		       BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_erase(firstpos, lastpos);
}
#endif

//#############################################################################

template<> void
BCP_vec<bool>::unchecked_update(const BCP_vec<int>& positions,
				  const BCP_vec<bool>& values){
   if (positions.size() == 0)
      return;
   const_iterator val = values.begin();
   BCP_vec<int>::const_iterator pos = positions.begin();
   const BCP_vec<int>::const_iterator lastpos = positions.end();
   while (pos != lastpos)
      operator[](*pos++) = *val++;
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<bool>::update(const BCP_vec<int>& positions,
			const BCP_vec<bool>& values){
   if (positions.size() != values.size())
      throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_update(positions, values);
}
