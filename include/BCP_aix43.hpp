// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_AIX43
#define _BCP_AIX43

// This file is fully docified.
// There's nothing to docify...

typedef int BCP_IndexType;

#if defined(__IBMCPP__) && (__IBMCPP__ >= 5)
#  include <sys/resource.h>
#  include <unistd.h>
#  define BCP_DEFAULT_NAMESPACE
#  define BCP_PtrDiff       long
#    define NEED_TEMPLATE_CLASSES
#    define NEED_TEMPLATE_FUNCTIONS
// #    define NEED_STD_TEMPLATE_CLASSES
// #    define NEED_STD_TEMPLATE_FUNCTIONS
// #    define NEED_IMPLICIT_TEMPLATE_CLASSES
// #    define NEED_IMPLICIT_TEMPLATE_FUNCTIONS
#  define BCP_CONSTRUCT     std::_Construct
#  define BCP_DESTROY       std::_Destroy
#  define BCP_DESTROY_RANGE(first, last)	\
          if (first != last)			\
             do {				\
                std::_Destroy(--last);		\
	     } while (first != last);

#endif

#if defined(__GNUC__)
#  define BCP_DEFAULT_NAMESPACE
#  include <sys/resource.h> // for setpriority()
#  ifndef __USE_BSD
#    define __USE_BSD  // to get gethostname() from unistd.h
#    include <unistd.h>
#    undef __USE_BSD
#  else
#    include <unistd.h>
#  endif
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

#endif // #ifndef _BCP_AIX43
