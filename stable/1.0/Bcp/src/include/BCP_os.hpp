// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

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

/** need to revive this in a platform independent way */
#if 0
/** Return the amount of memory left in kilobytes. Note that this works only
    if sysinfo() is available, otherwise returns -1. */
#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
static inline int freemem() {
    struct sysinfo info;
    sysinfo(&info);
    return (info.mem_unit*info.freeram)/1024;
}
#else
static inline int freemem() {
    return -1;
}
#endif
#endif

#endif
