// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

// This file is fully docified.
// There's nothing to docify...

#if defined(LINUX) || (defined(__GNUC__) && defined(__linux__))
#  include "BCP_linux.hpp"
#endif

#if defined(_AIX43)
#  include "BCP_aix43.hpp"
#endif

#if defined(__GNUC__) && defined(__sparc) && defined(__sun)
#  include "BCP_sunos.hpp"
#endif

#endif
