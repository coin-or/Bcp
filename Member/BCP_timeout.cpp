// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_os.hpp"
#include "BCP_timeout.hpp"

double BCP_time_since_epoch() {
   BCP_DEFAULT_NAMESPACE;
   static struct rusage ru;
   getrusage(RUSAGE_SELF, &ru);
   const struct timeval& tv = ru.ru_utime;
   return (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}
