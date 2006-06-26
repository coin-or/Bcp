// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

//------------------------------------------------------------------------------
#if HAVE_SYS_RESOURCE_H
#  include <sys/resources.h>
#else
#  define setpriority(x,y,z)
#endif
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

#endif
