// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_os.hpp"
#include "BCP_timeout.hpp"

#ifndef BCP_USE_RUSAGE 
#  define BCP_USE_RUSAGE 1
#endif

double BCP_time_since_epoch() {
#if BCP_USE_RUSAGE == 1
   struct rusage ru;
   getrusage(RUSAGE_SELF, &ru);
   const struct timeval& tv = ru.ru_utime;
#else
   struct timeval tv;
   gettimeofday(&tv, (struct timezone*)0);
#endif
   return (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}
