// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

//##############################################################################

/* The methods that are commented out are sort of generic methods that do not
   need specialization. Their implementation is in BCP_vector_general.hpp */

//##############################################################################

template<> inline void BCP_vec<short>::destroy(iterator pos)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::destroy_range(iterator first, iterator last)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::construct(iterator pos)
{
	*pos = 0;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::construct(iterator pos, const_reference x)
{
	*pos = x;
}

//##############################################################################

// template<> inline void BCP_vec<short>::allocate(size_t len)
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::deallocate() {
	if (start) {
		::operator delete(start);
	}
}
//------------------------------------------------------------------------------
template<> void BCP_vec<short>::insert_aux(iterator position, const_reference x);

//##############################################################################

// template<> void BCP_vec<short>::BCP_vec();
//------------------------------------------------------------------------------
// template<> void BCP_vec<short>::BCP_vec(const BCP_vec<short>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<short>::BCP_vec(const size_t n, const_reference value);
//------------------------------------------------------------------------------
template<> BCP_vec<short>::BCP_vec(const_iterator first, const_iterator last);
//------------------------------------------------------------------------------
template<> BCP_vec<short>::BCP_vec(const short* x, const size_t num);

//##############################################################################

template<> void BCP_vec<short>::reserve(const size_t n);
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<short>::swap(BCP_vec<short>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<short>& BCP_vec<short>::operator=(const BCP_vec<short>& x);

//##############################################################################
// these two members serve to copy out entries from a buffer.

template<> void BCP_vec<short>::assign(const void* x, const size_t num);
//------------------------------------------------------------------------------
template<> void BCP_vec<short>::insert(short* position,
									  const void* first, const size_t n);
//------------------------------------------------------------------------------
template<> void BCP_vec<short>::insert(iterator position,
									  const_iterator first,
									  const_iterator last);
//------------------------------------------------------------------------------
template<> void BCP_vec<short>::insert(iterator position, const size_t n,
									  const_reference x);
//------------------------------------------------------------------------------
template<> inline BCP_vec<short>::iterator
BCP_vec<short>::insert(iterator position, const_reference x)
{
	const size_t n = position - start;
	if (finish != end_of_storage && position == finish) {
		*finish++ = x;
	} else
		insert_aux(position, x);
	return start + n;
}

//##############################################################################

template<> inline void BCP_vec<short>::push_back(const_reference x)
{
	if (finish != end_of_storage)
		*finish++ = x;
	else
		insert_aux(finish, x);
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::unchecked_push_back(const_reference x)
{
	*finish++ = x;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::pop_back()
{
	--finish;
}
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<short>::clear();
//------------------------------------------------------------------------------
template<> inline void
BCP_vec<short>::unchecked_update(const BCP_vec<int>& positions,
								const BCP_vec<short>& values)
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
template<> inline void BCP_vec<short>::update(const BCP_vec<int>& positions,
											 const BCP_vec<short>& values)
{
	if (positions.size() != values.size())
		throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
	BCP_vec_sanity_check(positions.begin(), positions.end(), size());
	unchecked_update(positions, values);
}

//##############################################################################

template<> inline void BCP_vec<short>::keep(iterator pos)
{
	*start = *pos;
	finish = start + 1;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::keep(iterator first, iterator last)
{
	const size_t len = last - first;
	memmove(start, first, len * sizeof(short));
	finish = start + len;
}
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<short>::unchecked_keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 									   BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<short>::keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 						  BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<short>::keep_by_index(const BCP_vec<int>& positions);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<short>::unchecked_keep_by_index(const BCP_vec<int>& positions);

//##############################################################################

template<> inline void BCP_vec<short>::erase(iterator position)
{
	if (position + 1 != finish)
		memmove(position, position + 1, ((finish-position) - 1) * sizeof(short));
	--finish;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<short>::erase(iterator first, iterator last)
{
	if (first != last && last != finish)
		memmove(first, last, (finish - last) * sizeof(short));
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

