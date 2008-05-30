// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstring>

//##############################################################################

/* The methods that are commented out are sort of generic methods that do not
   need specialization. Their implementation is in BCP_vector_general.hpp */

//##############################################################################

template<> inline void BCP_vec<double>::destroy(iterator pos)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::destroy_range(iterator first, iterator last)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::construct(iterator pos)
{
	*pos = 0;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::construct(iterator pos, const_reference x)
{
	*pos = x;
}

//##############################################################################

// template<> inline void BCP_vec<double>::allocate(size_t len)
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::deallocate() {
	if (start) {
		::operator delete(start);
	}
}
//------------------------------------------------------------------------------
template<> void BCP_vec<double>::insert_aux(iterator position, const_reference x);

//##############################################################################

// template<> void BCP_vec<double>::BCP_vec();
//------------------------------------------------------------------------------
// template<> void BCP_vec<double>::BCP_vec(const BCP_vec<double>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<double>::BCP_vec(const size_t n, const_reference value);
//------------------------------------------------------------------------------
template<> BCP_vec<double>::BCP_vec(const_iterator first, const_iterator last);
//------------------------------------------------------------------------------
template<> BCP_vec<double>::BCP_vec(const double* x, const size_t num);

//##############################################################################

template<> void BCP_vec<double>::reserve(const size_t n);
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<double>::swap(BCP_vec<double>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<double>& BCP_vec<double>::operator=(const BCP_vec<double>& x);

//##############################################################################
// these two members serve to copy out entries from a buffer.

template<> void BCP_vec<double>::assign(const void* x, const size_t num);
//------------------------------------------------------------------------------
template<> void BCP_vec<double>::insert(double* position,
									  const void* first, const size_t n);
//------------------------------------------------------------------------------
template<> void BCP_vec<double>::insert(iterator position,
									  const_iterator first,
									  const_iterator last);
//------------------------------------------------------------------------------
template<> void BCP_vec<double>::insert(iterator position, const size_t n,
									  const_reference x);
//------------------------------------------------------------------------------
template<> inline BCP_vec<double>::iterator
BCP_vec<double>::insert(iterator position, const_reference x)
{
	const size_t n = position - start;
	if (finish != end_of_storage && position == finish) {
		*finish++ = x;
	} else
		insert_aux(position, x);
	return start + n;
}

//##############################################################################

template<> inline void BCP_vec<double>::push_back(const_reference x)
{
	if (finish != end_of_storage)
		*finish++ = x;
	else
		insert_aux(finish, x);
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::unchecked_push_back(const_reference x)
{
	*finish++ = x;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::pop_back()
{
	--finish;
}
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<double>::clear();
//------------------------------------------------------------------------------
template<> inline void
BCP_vec<double>::unchecked_update(const BCP_vec<int>& positions,
								const BCP_vec<double>& values)
{
	if (positions.size() == 0)
		return;
	const_iterator val = values.begin();
	BCP_vec<int>::const_iterator pos = positions.begin();
	const BCP_vec<int>::const_iterator lastpos = positions.end();
	while (pos != lastpos)
		operator[](*pos++) = *val++;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::update(const BCP_vec<int>& positions,
											 const BCP_vec<double>& values)
{
	if (positions.size() != values.size())
		throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
	BCP_vec_sanity_check(positions.begin(), positions.end(), size());
	unchecked_update(positions, values);
}

//##############################################################################

template<> inline void BCP_vec<double>::keep(iterator pos)
{
	*start = *pos;
	finish = start + 1;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::keep(iterator first, iterator last)
{
	const size_t len = last - first;
	std::memmove(start, first, len * sizeof(double));
	finish = start + len;
}
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<double>::unchecked_keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 									   BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<double>::keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 						  BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<double>::keep_by_index(const BCP_vec<int>& positions);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<double>::unchecked_keep_by_index(const BCP_vec<int>& positions);

//##############################################################################

template<> inline void BCP_vec<double>::erase(iterator position)
{
	if (position + 1 != finish)
		std::memmove(position, position + 1, ((finish-position) - 1) * sizeof(double));
	--finish;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<double>::erase(iterator first, iterator last)
{
	if (first != last && last != finish)
		std::memmove(first, last, (finish - last) * sizeof(double));
	finish -= (last - first);
}
//------------------------------------------------------------------------------
// template <class T> inline void
// BCP_vec<T>::erase_by_index(const BCP_vec<int>& positions);
//------------------------------------------------------------------------------
// template <class T> inline void
// BCP_vec<T>::unchecked_erase_by_index(const BCP_vec<int>& positions);
//------------------------------------------------------------------------------
// template <class T> inline void
// BCP_vec<T>::erase_by_index(BCP_vec<int>::const_iterator firstpos,
// 						   BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template <class T> void
// BCP_vec<T>::unchecked_erase_by_index(BCP_vec<int>::const_iterator firstpos,
// 									 BCP_vec<int>::const_iterator lastpos);

//#############################################################################

