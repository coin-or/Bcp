// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm> // this gets pair<>, too

#include "BCP_simul_sort.hpp"
#include "BCP_temporary.hpp"

template <class RAI_S, class RAI_T> void
BCP_simul_sort(RAI_S sfirst, RAI_S slast, RAI_T tfirst)
{
   if (std::distance(sfirst, slast) <= 1)
      return;

   typedef typename std::iterator_traits<RAI_S>::value_type S;
   typedef typename std::iterator_traits<RAI_T>::value_type T;

   BCP_vec< pair<S, T> > x;
   x.reserve(slast - sfirst);

   RAI_S scurrent = sfirst;
   RAI_T tcurrent = tfirst;
   while (scurrent != slast)
      x.unchecked_push_back( std::make_pair(*scurrent++, *tcurrent++) );

   sort(x.begin(), x.end());

   scurrent = sfirst;
   tcurrent = tfirst;
   for (size_t i = 0; i < x.size(); ++i) {
      *scurrent++ = x[i].first;
      *tcurrent++ = x[i].second;
   }
}

//#############################################################################

template <class RAI_S, class RAI_T> void
BCP_simul_stable_sort(RAI_S sfirst, RAI_S slast, RAI_T tfirst)
{
   if (std::distance(sfirst, slast) <= 1)
      return;

   typedef typename std::iterator_traits<RAI_S>::value_type S;
   typedef typename std::iterator_traits<RAI_T>::value_type T;

   BCP_vec< pair<S, T> > x;
   x.reserve(slast - sfirst);

   RAI_S scurrent = sfirst;
   RAI_T tcurrent = tfirst;
   while (scurrent != slast)
      x.unchecked_push_back( std::make_pair(*scurrent++, *tcurrent++) );

   stable_sort(x.begin(), x.end());

   scurrent = sfirst;
   tcurrent = tfirst;
   for (size_t i = 0; i < x.size(); ++i) {
      *scurrent++ = x[i].first;
      *tcurrent++ = x[i].second;
   }
}
