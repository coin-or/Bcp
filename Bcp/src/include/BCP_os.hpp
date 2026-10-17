// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

#include <cstdio>
#include "BcpConfig.h"
#include "CoinTime.hpp"

//-----------------------------------------------------------------------------
// Wallclock time since the first call. A negative argument returns the time of
// the first call; a positive one, in the first call, sets it. The processes of
// a parallel run use this to measure time from the start of the tree manager.
// This is what CoinWallclockTime(double) did up to CoinUtils 2.11; newer
// versions of CoinUtils take no argument.
inline double BCP_wallclock_time(double callType = 0)
{
  double callTime = CoinGetTimeOfDay();
  static const double firstCall = callType > 0 ? callType : callTime;
  return callType < 0 ? firstCall : callTime - firstCall;
}

//-----------------------------------------------------------------------------
#ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#else
#  define setpriority(x,y,z)
#endif
//-----------------------------------------------------------------------------
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
#  include <process.h>
#endif

#ifndef HAVE_GETHOSTNAME
#  define gethostname(x,y)
#endif

#ifdef HAVE_GETPID
#  define GETPID (getpid())
#else
#  define GETPID (0)
#endif
//-----------------------------------------------------------------------------

#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif
static inline long BCP_free_mem()
{
#ifdef HAVE_SYSINFO
  struct sysinfo info;
  sysinfo(&info);
  return info.mem_unit*info.freeram;
#else
  return -1;
#endif
}
//-----------------------------------------------------------------------------

#ifdef HAVE_MALLINFO
#include <malloc.h>
#endif

static inline long BCP_used_heap()
{
#ifdef HAVE_MALLINFO
  struct mallinfo info = mallinfo();
  return info.usmblks + info.uordblks;;
#else
  return -1;
#endif
}
//-----------------------------------------------------------------------------

#endif
