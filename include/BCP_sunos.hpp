// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_SUNOS
#define _BCP_SUNOS

// This file is fully docified.
// There's nothing to docify...

typedef int BCP_IndexType;

#if defined(__GNUC__)

#  include <sys/time.h>     // for gettimeofday()
#  include <sys/resource.h> // for setpriority()
#  include <unistd.h>       // to get gethostname() from unistd.h
#  define NEED_IMPLICIT_TEMPLATE_FUNCTIONS 1
#  ifdef __OPTIMIZE__
#    define NEED_IMPLICIT_TEMPLATE_CLASSES 1
#  endif
#  define NEED_IMPLICIT_TEMPLATE_CLASSES 1
#  define BCP_PtrDiff       int
#  define BCP_CONSTRUCT     construct
#  define BCP_DESTROY       destroy
#  define BCP_DESTROY_RANGE destroy

#endif

#endif
