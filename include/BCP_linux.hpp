// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LINUX
#define _BCP_LINUX

// This file is fully docified.
// There's nothing to docify...

typedef int BCP_IndexType;

#if defined(__GNUC__)

#  include <sys/time.h>     // for gettimeofday()
#  include <sys/resource.h> // for setpriority()
#  ifndef __USE_BSD
#    define __USE_BSD  // to get gethostname() from unistd.h
#    include <unistd.h>
#    undef __USE_BSD
#  else
#    include <unistd.h>
#  endif
#  define NEED_IMPLICIT_TEMPLATE_FUNCTIONS 1
#  ifdef __OPTIMIZE__
#    define NEED_IMPLICIT_TEMPLATE_CLASSES 1
#  endif
#  define NEED_IMPLICIT_TEMPLATE_CLASSES 1
#  define BCP_PtrDiff       int
#  if (__GNUC__ >= 3)
#    define BCP_CONSTRUCT     std::_Construct
#    define BCP_DESTROY       std::_Destroy
#    define BCP_DESTROY_RANGE std::_Destroy
#  else
#    define BCP_CONSTRUCT     std::construct
#    define BCP_DESTROY       std::destroy
#    define BCP_DESTROY_RANGE std::destroy
#  endif

#endif

#endif
