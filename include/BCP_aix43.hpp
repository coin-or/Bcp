// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_AIX43
#define _BCP_AIX43

// This file is fully docified.
// There's nothing to docify...

#include <sys/resource.h>     // for getrusage()
#include <unistd.h>       // for setpriority() and gethostname()
typedef int BCP_IndexType;

#if defined(__GNUC__)
#  define BCP_DEFAULT_NAMESPACE

#  define NEED_IMPLICIT_TEMPLATE_FUNCTIONS 1
#  define NEED_IMPLICIT_TEMPLATE_CLASSES 1
#  define BCP_PtrDiff       long
#  define BCP_CONSTRUCT     std::construct
#  define BCP_DESTROY       std::destroy
#  define BCP_DESTROY_RANGE std::destroy

#elif defined(__IBMCPP__) && (__IBMCPP__ >= 5)

#  define BCP_PtrDiff       long
#  define BCP_CONSTRUCT     std::_Construct
#  define BCP_DESTROY       std::_Destroy
#  define BCP_DESTROY_RANGE(first, last)	\
          if (first != last)			\
             do {				\
                std::_Destroy(--last);		\
	     } while (first != last);
#endif

#endif
