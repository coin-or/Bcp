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
#  define BCP_PtrDiff       int

#  if (__GNUC__ >= 3)

#    define NEED_TEMPLATE_CLASSES
#    define NEED_TEMPLATE_FUNCTIONS
// #    define NEED_STD_TEMPLATE_FUNCTIONS
// #    define NEED_IMPLICIT_TEMPLATE_CLASSES
// #    define NEED_IMPLICIT_TEMPLATE_FUNCTIONS
#    define BCP_CONSTRUCT     std::_Construct
#    define BCP_DESTROY       std::_Destroy
#    define BCP_DESTROY_RANGE std::_Destroy

#  else

#    define NEED_TEMPLATE_CLASSES
#    define NEED_TEMPLATE_FUNCTIONS
// #    define NEED_STD_TEMPLATE_FUNCTIONS
// #    define NEED_IMPLICIT_TEMPLATE_CLASSES
// #    define NEED_IMPLICIT_TEMPLATE_FUNCTIONS
#    define BCP_CONSTRUCT     std::construct
#    define BCP_DESTROY       std::destroy
#    define BCP_DESTROY_RANGE std::destroy

#  endif

#endif

#endif
