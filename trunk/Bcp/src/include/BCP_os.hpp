// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

#include <cstdio>
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
/*
  The problem here is that Solaris defines a sysinfo function, but it does
  something completely different. This may not be an adequate guard when
  using GCC on Solaris. It may be that you'll need to change the test in
  configure.ac to check specifically for sysinfo.freeram with AC_CHECK_MEMBER.
  See http://developers.sun.com/solaris/articles/free_phys_ram.html for a way
  to do this on Solaris. Didn't try to implement it.  -- lh, 070810 --
*/
#if defined(HAVE_SYSINFO) && !defined(__sun)
#include <sys/sysinfo.h>
#endif
static inline long BCP_free_mem()
{
#if defined(HAVE_SYSINFO) && !defined(__sun)
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
