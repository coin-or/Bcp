// Copyright (C) 2003, International Business Machines
// Corporation and Jorg Herbers and others.  All Rights Reserved.

#ifndef _BCP_MSVC
#define _BCP_MSVC

// This file is fully docified.
// There's nothing to docify...

typedef int BCP_IndexType;

#define setpriority(x,y,z)
#define BCP_USE_RUSAGE 0
#include <Windows.h>
#define gethostname(c, l) { DWORD cnlen = l; GetComputerName(c, &cnlen); }
#include <process.h>
#define BCP_PtrDiff       int
#define BCP_DEFAULT_NAMESPACE

#define NEED_TEMPLATE_CLASSES
// #    define NEED_TEMPLATE_FUNCTIONS
// #    define NEED_STD_TEMPLATE_FUNCTIONS
// #    define NEED_IMPLICIT_TEMPLATE_CLASSES
// #    define NEED_IMPLICIT_TEMPLATE_FUNCTIONS
#define BCP_CONSTRUCT     std::_Construct
#define BCP_DESTROY       std::_Destroy
//#    define BCP_DESTROY_RANGE std::_Destroy_range

template<class T> inline void BCP_DESTROY_RANGE(T *first, T *last)
{
   for (; first != last; ++first)
      BCP_DESTROY(first);
}

#endif

