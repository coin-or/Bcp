// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_SIMUL_SORT_H
#define _BCP_SIMUL_SORT_H

// This file is fully docified.

#include "BCP_vector.hpp"

/** A template function that simultaneously sorts two arrays.
    
    Actually, the function sorts <code>[sfirst,slast)</code> (the <em>key<em>
    array and performs the same changes on
    <code>[tfirst,tfirst+(slast-sfirst))</code>.

    <code>RAI_S</code> and <code>RAI_T</code> are random access iterators.
    The entries in the key array are compared using <code>operator<</code>.
    The final order of equal entries in the key array is undefined.
*/
template <class RAI_S, class RAI_T> void
BCP_simul_sort(RAI_S sfirst, RAI_S slast, RAI_T tfirst);

/** A template function that simultaneosly stable sorts two arrays.
    
    Actually, the function stable sorts <code>[sfirst,slast)</code> (the
    <em>key<em> array and performs the same changes on
    <code>[tfirst,tfirst+(slast-sfirst))</code>.

    <code>RAI_S</code> and <code>RAI_T</code>are random access iterators.
    The entries in the key array are compared using <code>operator<</code>.
    The final order of equal entries in the key array is the same as their
    order was originally. 
*/
template <class RAI_S, class RAI_T> void
BCP_simul_stable_sort(RAI_S sfirst, RAI_S slast, RAI_T tfirst);

#endif
