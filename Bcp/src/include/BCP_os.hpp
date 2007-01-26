// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

#include "BcpConfig.h"

//-----------------------------------------------------------------------------
#if HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#else
#  define setpriority(x,y,z)
#endif
//-----------------------------------------------------------------------------
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifndef HAVE_GETHOSTNAME
#  define gethostname(x,y)
#endif

#if HAVE_GETPID
#  define GETPID (getpid())
#else
#  define GETPID (0)
#endif

/** Return the amount of memory left in kilobytes. Note that this works only
    if sysinfo() is available, otherwise returns -1. */
#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
static inline void BCP_sysinfo_mem(long& totalmem, long& freemem) {
    struct sysinfo info;
    sysinfo(&info);
    freemem = info.mem_unit*info.freeram;
    totalmem = info.mem_unit*info.totalram;
}
#else
static inline void BCP_sysinfo_mem(long& totalmem, long& freemem) {
    totalmem = -1;
    freemem = -1;
}
#endif

#ifdef HAVE_MALLINFO
#include <malloc.h>
/** Returns the total amount of memory in the heap and the amount of memory
    used (both in bytes) */
static inline void BCP_mallinfo_mem(long& total, long& used)
{
    struct mallinfo info = mallinfo();
    used = info.smblks + info.uordblks;
    total = info.arena;
}
#else
static inline void BCP_mallinfo_mem(long& total, long& used)
{
    total = -1;
    used = -1;
}
#endif

#endif
