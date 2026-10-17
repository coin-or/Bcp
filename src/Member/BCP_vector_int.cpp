// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_vector.hpp"

//##############################################################################

/* Here only the non-inlined methods are defined */

//##############################################################################

template<> void BCP_vec<int>::insert_aux(iterator position, const_reference x)
{
	const size_t after_pos = finish - position;
	if (finish != end_of_storage) {
		memmove(position+1, position, after_pos * sizeof(int));
		*position = x;
		++finish;
	} else {
		const size_t len = (2*size() + 0x100);
		iterator tmp = allocate(len);
		const size_t until_pos = position - start;
		memmove(tmp, start, until_pos * sizeof(int));
		iterator tmp_finish = tmp + until_pos;
		*tmp_finish++ = x;
		memmove(tmp_finish, position, after_pos * sizeof(int));
		deallocate();
		start = tmp;
		finish = start + (until_pos + 1 + after_pos);
		end_of_storage = tmp + len;
	}
}

//##############################################################################

template<> BCP_vec<int>::BCP_vec(const size_t n, const_reference value) :
	start(0), finish(0), end_of_storage(0)
{
	if (n > 0) {
		start = finish = allocate(n);
		end_of_storage = start + n;
		while (finish != end_of_storage)
			*finish++ = value;
	}
}
//------------------------------------------------------------------------------
template<> BCP_vec<int>::BCP_vec(const_iterator first, const_iterator last) :
	start(0), finish(0), end_of_storage(0)
{
	const size_t len = last - first;
	if (len > 0) {
		start = allocate(len);
		memcpy(start, first, len * sizeof(int));
		end_of_storage = finish = start + len;
	}
}
//------------------------------------------------------------------------------
template<> BCP_vec<int>::BCP_vec(const int* x, const size_t num) :
	start(0), finish(0), end_of_storage(0)
{
	if (num > 0) {
		start = allocate(num);
		memcpy(start, x, num * sizeof(int));
		end_of_storage = finish = start + num;
	}
}

//##############################################################################

template<> void BCP_vec<int>::reserve(const size_t n)
{
	if (capacity() < n) {
		iterator tmp = allocate(n);
		const size_t oldsize = size();
		if (oldsize > 0)
			memcpy(tmp, start, oldsize * sizeof(int));
		deallocate();
		start = tmp;
		finish = start + oldsize;
		end_of_storage = start + n;
	}
}
//------------------------------------------------------------------------------
template<> BCP_vec<int>& BCP_vec<int>::operator=(const BCP_vec<int>& x)
{
	if (&x != this) {
		const size_t x_size = x.size();
		if (x_size > capacity()) {
			deallocate();
			start = allocate(x_size);
			end_of_storage = start + x_size;
		}
		if (x_size > 0)
			memcpy(start, x.start, x_size * sizeof(int));
		finish = start + x_size;
	}
	return *this;
}

//##############################################################################
// these two members serve to copy out entries from a buffer.

template<> void BCP_vec<int>::assign(const void* x, const size_t num)
{
	if (num > capacity()){
		deallocate();
		start = allocate(num);
		end_of_storage = start + num;
	}
	memcpy(start, x, num * sizeof(int));
	finish = start + num;
}
//------------------------------------------------------------------------------
template<> void BCP_vec<int>::insert(int* position,
									  const void* first, const size_t n)
{
	if (n == 0) return;
	if ((size_t) (end_of_storage - finish) >= n) {
		if (finish != position)
			memmove(position + n, position, (finish - position) * sizeof(int));
		memcpy(position, first, n * sizeof(int));
		finish += n;
	} else {
		const size_t new_size = (2*size() + n);
		iterator tmp = allocate(new_size);
		const size_t after_pos = finish - position;
		const size_t until_pos = position - start;
		memcpy(tmp, start, until_pos * sizeof(int));
		memcpy(tmp + until_pos, first, n * sizeof(int));
		memcpy(tmp + (until_pos + n), position, after_pos * sizeof(int));
		deallocate();
		start = tmp;
		finish = start + (until_pos + n + after_pos);
		end_of_storage = tmp + new_size;
	}
}
//------------------------------------------------------------------------------
template<> void BCP_vec<int>::insert(iterator position,
									  const_iterator first,
									  const_iterator last)
{
	if (first == last) return;
	const size_t n = last - first;
	if ((size_t) (end_of_storage - finish) >= n) {
		memmove(position + n, position, (finish - position) * sizeof(int));
		memcpy(position, first, n * sizeof(int));
		finish += n;
	} else {
		const size_t new_size = (2*size() + n);
		iterator tmp = allocate(new_size);
		const size_t after_pos = finish - position;
		const size_t until_pos = position - start;
		memcpy(tmp, start, until_pos * sizeof(int));
		memcpy(tmp + until_pos, first, n * sizeof(int));
		memcpy(tmp + (until_pos + n), position, after_pos * sizeof(int));
		deallocate();
		start = tmp;
		finish = start + (until_pos + n + after_pos);
		end_of_storage = tmp + new_size;
	}
}
//------------------------------------------------------------------------------
template<> void BCP_vec<int>::insert(iterator position, const size_t n,
									  const_reference x)
{
	if (n == 0) return;
	if ((size_t) (end_of_storage - finish) >= n) {
		memmove(position + n, position, (finish - position) * sizeof(int));
		finish += n;
		for (int i = n; i > 0; --i)
			*position++ = x;
	} else {
		const size_t new_size = (2*size() + n);
		iterator tmp = allocate(new_size);
		const size_t after_pos = finish - position;
		const size_t until_pos = position - start;
		memcpy(tmp, start, until_pos * sizeof(int));
		memcpy(tmp + (until_pos + n), position, after_pos * sizeof(int));
		deallocate();
		start = tmp;
		finish = start + (until_pos + n + after_pos);
		end_of_storage = tmp + new_size;
		position = tmp + until_pos;
		for (int i = n; i > 0; --i)
			*position++ = x;
	}
}

//##############################################################################
