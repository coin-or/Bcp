// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

//##############################################################################

/* The methods that are commented out are sort of generic methods that do not
   need specialization. Their implementation is in BCP_vector_general.hpp */

//##############################################################################

template<> inline void BCP_vec<char>::destroy(iterator pos)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::destroy_range(iterator first, iterator last)
{
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::construct(iterator pos)
{
	*pos = 0;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::construct(iterator pos, const_reference x)
{
	*pos = x;
}

//##############################################################################

// template<> inline void BCP_vec<char>::allocate(size_t len)
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::deallocate() {
	if (start) {
		::operator delete(start);
	}
}
//------------------------------------------------------------------------------
template<> void BCP_vec<char>::insert_aux(iterator position, const_reference x);

//##############################################################################

// template<> void BCP_vec<char>::BCP_vec();
//------------------------------------------------------------------------------
// template<> void BCP_vec<char>::BCP_vec(const BCP_vec<char>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<char>::BCP_vec(const size_t n, const_reference value);
//------------------------------------------------------------------------------
template<> BCP_vec<char>::BCP_vec(const_iterator first, const_iterator last);
//------------------------------------------------------------------------------
template<> BCP_vec<char>::BCP_vec(const char* x, const size_t num);

//##############################################################################

template<> void BCP_vec<char>::reserve(const size_t n);
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<char>::swap(BCP_vec<char>& x);
//------------------------------------------------------------------------------
template<> BCP_vec<char>& BCP_vec<char>::operator=(const BCP_vec<char>& x);

//##############################################################################
// these two members serve to copy out entries from a buffer.

template<> void BCP_vec<char>::assign(const void* x, const size_t num);
//------------------------------------------------------------------------------
template<> void BCP_vec<char>::insert(char* position,
									  const void* first, const size_t n);
//------------------------------------------------------------------------------
template<> void BCP_vec<char>::insert(iterator position,
									  const_iterator first,
									  const_iterator last);
//------------------------------------------------------------------------------
template<> void BCP_vec<char>::insert(iterator position, const size_t n,
									  const_reference x);
//------------------------------------------------------------------------------
template<> inline BCP_vec<char>::iterator
BCP_vec<char>::insert(iterator position, const_reference x)
{
	const size_t n = position - start;
	if (finish != end_of_storage && position == finish) {
		*finish++ = x;
	} else
		insert_aux(position, x);
	return start + n;
}

//##############################################################################

template<> inline void BCP_vec<char>::push_back(const_reference x)
{
	if (finish != end_of_storage)
		*finish++ = x;
	else
		insert_aux(finish, x);
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::unchecked_push_back(const_reference x)
{
	*finish++ = x;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::pop_back()
{
	--finish;
}
//------------------------------------------------------------------------------
// template<> inline void BCP_vec<char>::clear();
//------------------------------------------------------------------------------
template<> inline void
BCP_vec<char>::unchecked_update(const BCP_vec<int>& positions,
								const BCP_vec<char>& values)
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
template<> inline void BCP_vec<char>::update(const BCP_vec<int>& positions,
											 const BCP_vec<char>& values)
{
	if (positions.size() != values.size())
		throw BCP_fatal_error("BCP_vec::update() called with unequal sizes.\n");
	BCP_vec_sanity_check(positions.begin(), positions.end(), size());
	unchecked_update(positions, values);
}

//##############################################################################

template<> inline void BCP_vec<char>::keep(iterator pos)
{
	*start = *pos;
	finish = start + 1;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::keep(iterator first, iterator last)
{
	const size_t len = last - first;
	memmove(start, first, len * sizeof(char));
	finish = start + len;
}
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<char>::unchecked_keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 									   BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<char>::keep_by_index(BCP_vec<int>::const_iterator firstpos,
// 						  BCP_vec<int>::const_iterator lastpos);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<char>::keep_by_index(const BCP_vec<int>& positions);
//------------------------------------------------------------------------------
// template<> inline void
// BCP_vec<char>::unchecked_keep_by_index(const BCP_vec<int>& positions);

//##############################################################################

template<> inline void BCP_vec<char>::erase(iterator position)
{
	if (position + 1 != finish)
		memmove(position, position + 1, ((finish-position) - 1) * sizeof(char));
	--finish;
}
//------------------------------------------------------------------------------
template<> inline void BCP_vec<char>::erase(iterator first, iterator last)
{
	if (first != last && last != finish)
		memmove(first, last, (finish - last) * sizeof(char));
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

