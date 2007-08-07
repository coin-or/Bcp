// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

#include "BcpConfig.h"

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

#ifdef HAVE_SYSINFO
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
