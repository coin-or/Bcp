// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_SET_INTERSECTS_H
#define _BCP_SET_INTERSECTS_H

// This file is fully docified.

/**Intersection tester using <code>operator<</code> for comparison.
   Returns true if the ordered intervals <code>[__first1,__last1)</code> and
   <code>[__first2,__last2)</code> intersect each other. The intervals are
   ordered by <code>operator<</code>.
*/
template <class _InputIter1, class _InputIter2>
const bool intersects(_InputIter1 __first1, _InputIter1 __last1,
		      _InputIter2 __first2, _InputIter2 __last2) {
   while (__first1 != __last1 && __first2 != __last2)
      if (*__first2 < *__first1)
	 ++__first2;
      else if(*__first1 < *__first2) 
	 ++__first1;
      else
	 return true;
   return false;
}

/**Intersection tester using a function object for comparison.
   Returns true if the ordered intervals <code>[__first1,__last1)</code> and
   <code>[__first2,__last2)</code> intersect each other. The intervals are
   ordered by the function object <code>_comp</code>, which takes two
   arguments and returns true if and only if the first argument is considered
   smaller than the second one.
*/
template <class _InputIter1, class _InputIter2, class _Compare>
const bool intersects(_InputIter1 __first1, _InputIter1 __last1,
		      _InputIter2 __first2, _InputIter2 __last2,
		      _Compare __comp) {
   while (__first1 != __last1 && __first2 != __last2)
      if (__comp(*__first2, *__first1))
	 ++__first2;
      else if(__comp(*__first1, *__first2)) 
	 ++__first1;
      else
	 return true;
   return false;
}

#endif
