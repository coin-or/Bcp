// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_error.hpp"
#include "BCP_vector.hpp"

//#############################################################################

void BCP_vec_sanity_check(BCP_vec<int>::const_iterator firstpos,
			  BCP_vec<int>::const_iterator lastpos,
			  const int maxsize)
{
   if (firstpos == lastpos)
      return;
   if (*firstpos < 0)
      throw BCP_fatal_error("Negative entry in a BCP_vec_sanity_check.\n");
   if (*(lastpos - 1) >= maxsize)
      throw BCP_fatal_error("Too big entry in a BCP_vec_sanity_check.\n");
   int prev = -1;
   while (firstpos != lastpos){
      if (*firstpos < prev)
	 throw BCP_fatal_error
	    ("Entry list is not ordered in BCP_vec_sanity_check.\n");
      if (*firstpos == prev)
	 throw BCP_fatal_error
	    ("Duplicate entry in BCP_vec_sanity_check.\n");
      prev = *firstpos;
      ++firstpos;
   }
}

