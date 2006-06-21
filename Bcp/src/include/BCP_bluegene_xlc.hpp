// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_BLUE_GENE_XLC
#define _BCP_BLUE_GENE_XLC

// This file is fully docified.
// There's nothing to docify...

typedef int int;

#include <sys/resource.h> // for setpriority()
#include <unistd.h>     // for gethostname()
#define BCP_PtrDiff       int
#define BCP_DEFAULT_NAMESPACE
#define NEED_TEMPLATE_CLASSES
#define NEED_TEMPLATE_FUNCTIONS
// #define NEED_STD_TEMPLATE_FUNCTIONS
// #define NEED_IMPLICIT_TEMPLATE_CLASSES
// #define NEED_IMPLICIT_TEMPLATE_FUNCTIONS
#define BCP_CONSTRUCT     std::_Construct
#define BCP_DESTROY       std::_Destroy
#define BCP_DESTROY_RANGE std::_Destroy

#endif
