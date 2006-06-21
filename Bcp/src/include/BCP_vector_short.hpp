// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

//#############################################################################

template<> void
BCP_vec<short>::deallocate() {
   if (start) {
      ::operator delete(start);
   }
}

//#############################################################################

template<> void
BCP_vec<short>::insert_aux(iterator position, const_reference x){
   const size_t after_pos = finish - position;
   if (finish != end_of_storage) {
      memmove(position+1, position, after_pos * sizeof(short));
      *position = x;
      ++finish;
   } else {
      const size_t len = (2*size() + 0x100);
      iterator tmp = allocate(len);
      const size_t until_pos = position - start;
      memmove(tmp, start, until_pos * sizeof(short));
      iterator tmp_finish = tmp + until_pos;
      *tmp_finish++ = x;
      memmove(tmp_finish, position, after_pos * sizeof(short));
      deallocate();
      start = tmp;
      finish = start + (until_pos + 1 + after_pos);
      end_of_storage = tmp + len;
   }
}

//#############################################################################

template <>
BCP_vec<short>::BCP_vec(const size_t n, const_reference value) :
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
BCP_vec<short>::BCP_vec(const_iterator first, const_iterator last) :
   start(0), finish(0), end_of_storage(0)
{
   const size_t len = last - first;
   if (len > 0) {
      start = allocate(len);
      memcpy(start, first, len * sizeof(short));
      end_of_storage = finish = start + len;
   }
}

template <>
BCP_vec<short>::BCP_vec(const short* x, const size_t num) :
   start(0), finish(0), end_of_storage(0)
{
   if (num > 0) {
      start = allocate(num);
      memcpy(start, x, num * sizeof(short));
      end_of_storage = finish = start + num;
   }
}

//#############################################################################

template <> inline void
BCP_vec<short>::push_back(const_reference x) {
   if (finish != end_of_storage)
      *finish++ = x;
   else
      insert_aux(finish, x);
}

template <> inline void
BCP_vec<short>::unchecked_push_back(const_reference x) {
   *finish++ = x;
}

template <> inline void
BCP_vec<short>::pop_back() {
   --finish;
}

//#############################################################################

template <> void
BCP_vec<short>::keep(iterator pos) {
   *start = *pos;
   finish = start + 1;
}

template <> void
BCP_vec<short>::keep(iterator first, iterator last) {
   const size_t len = last - first;
   memmove(start, first, len * sizeof(short));
   finish = start + len;
}

//#############################################################################

template <> void
BCP_vec<short>::erase(iterator position) {
   if (position + 1 != finish)
      memmove(position, position + 1, ((finish-position) - 1) *sizeof(short));
   --finish;
}

template <> void
BCP_vec<short>::erase(iterator first, iterator last) {
   if (first != last && last != finish)
      memmove(first, last, (finish - last) * sizeof(short));
   finish -= (last - first);
}

//#############################################################################
//#############################################################################

template<> void
BCP_vec<short>::reserve(const size_t n){
   if (capacity() < n) {
      iterator tmp = allocate(n);
      const size_t oldsize = size();
      if (oldsize > 0)
	 memcpy(tmp, start, oldsize * sizeof(short));
      deallocate();
      start = tmp;
      finish = start + oldsize;
      end_of_storage = start + n;
   }
}

//#############################################################################

template<> void
BCP_vec<short>::swap(BCP_vec<short>& x) {
   std::swap(start, x.start);
   std::swap(finish, x.finish);
   std::swap(end_of_storage, x.end_of_storage);
}
   
//#############################################################################

template<> BCP_vec<short>&
BCP_vec<short>::operator=(const BCP_vec<short>& x){
   if (&x != this) {
      const size_t x_size = x.size();
      if (x_size > capacity()) {
	 deallocate();
	 start = allocate(x_size);
	 end_of_storage = start + x_size;
      }
      if (x_size > 0)
	 memcpy(start, x.start, x_size * sizeof(short));
      finish = start + x_size;
   }
   return *this;
}

//#############################################################################
// these two members serve to copy out entries from a buffer.

template<> void
BCP_vec<short>::assign(const void* x, const size_t num){
   if (num > capacity()){
      deallocate();
      start = allocate(num);
      end_of_storage = start + num;
   }
   memcpy(start, x, num * sizeof(short));
   finish = start + num;
}

template<> void
BCP_vec<short>::insert(short* position, const void* first, const size_t n)
{
   if (n == 0) return;
   if ((size_t) (end_of_storage - finish) >= n) {
      if (finish != position)
	 memmove(position + n, position, (finish - position) * sizeof(short));
      memcpy(position, first, n * sizeof(short));
      finish += n;
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(short));
      memcpy(tmp + until_pos, first, n * sizeof(short));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(short));
      deallocate();
      start = tmp;
      finish = start + (until_pos + n + after_pos);
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template<> void
BCP_vec<short>::insert(iterator position,
			const_iterator first, const_iterator last){
   if (first == last) return;
   const size_t n = last - first;
   if ((size_t) (end_of_storage - finish) >= n) {
      memmove(position + n, position, (finish - position) * sizeof(short));
      memcpy(position, first, n * sizeof(short));
      finish += n;
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(short));
      memcpy(tmp + until_pos, first, n * sizeof(short));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(short));
      deallocate();
      start = tmp;
      finish = start + (until_pos + n + after_pos);
      end_of_storage = tmp + new_size;
   }
}

//#############################################################################

template<> void
BCP_vec<short>::insert(iterator position, const size_t n, const_reference x) {
   if (n == 0) return;
   if ((size_t) (end_of_storage - finish) >= n) {
      memmove(position + n, position, (finish - position) * sizeof(short));
      finish += n;
      for (int i = n; i > 0; --i)
	 *position++ = x;
   } else {
      const size_t new_size = (2*size() + n);
      iterator tmp = allocate(new_size);
      const size_t after_pos = finish - position;
      const size_t until_pos = position - start;
      memcpy(tmp, start, until_pos * sizeof(short));
      memcpy(tmp + (until_pos + n), position, after_pos * sizeof(short));
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

template<> BCP_vec<short>::iterator
BCP_vec<short>::insert(iterator position, const_reference x){
   const size_t n = position - start;
   if (finish != end_of_storage && position == finish) {
      *finish++ = x;
   } else
      insert_aux(position, x);
   return start + n;
}

//#############################################################################

template<> void
BCP_vec<short>::unchecked_keep_by_index(BCP_vec<int>::const_iterator firstpos,
					 BCP_vec<int>::const_iterator lastpos)
{
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
BCP_vec<short>::unchecked_erase_by_index(BCP_vec<int>::const_iterator firstpos,
					  BCP_vec<int>::const_iterator lastpos)
{
   if (firstpos == lastpos)
      return;
   --lastpos;
   int source;
   iterator target = entry(*firstpos);
   while (firstpos != lastpos){
      source = *firstpos + 1;
      ++firstpos;
      if (*firstpos > source) {
	 memmove(target, start + source, (*firstpos - source) * sizeof(short));
	 target += (*firstpos - source);
      }
   }
   source = *firstpos + 1;
   int oldsize = size();
   memmove(target, start + source, (oldsize - source) * sizeof(short));
   finish = target + (oldsize - source);
}

//#############################################################################

template<> void
BCP_vec<short>::unchecked_keep_by_index(const BCP_vec<int>& positions){
   unchecked_keep_by_index(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<short>::keep_by_index(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_keep_by_index(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<short>::keep_by_index(BCP_vec<int>::const_iterator firstpos,
			       BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_keep_by_index(firstpos, lastpos);
}

//#############################################################################

template<> void
BCP_vec<short>::unchecked_erase_by_index(const BCP_vec<int>& positions){
   unchecked_erase_by_index(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<short>::erase_by_index(const BCP_vec<int>& positions){
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_erase_by_index(positions.begin(), positions.end());
}

//-----------------------------------------------------------------------------

template<> void
BCP_vec<short>::erase_by_index(BCP_vec<int>::const_iterator firstpos,
				BCP_vec<int>::const_iterator lastpos) {
   BCP_vec_sanity_check(firstpos, lastpos, size());
   unchecked_erase_by_index(firstpos, lastpos);
}

//#############################################################################

template<> void
BCP_vec<short>::unchecked_update(const BCP_vec<int>& positions,
				  const BCP_vec<short>& values){
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
BCP_vec<short>::update(const BCP_vec<int>& positions,
			const BCP_vec<short>& values){
   if (positions.size() != values.size())
      throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
   BCP_vec_sanity_check(positions.begin(), positions.end(), size());
   unchecked_update(positions, values);
}
