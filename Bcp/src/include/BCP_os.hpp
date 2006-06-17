// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OS_H
#define _BCP_OS_H

// This file is fully docified.
// There's nothing to docify...

#if (defined(__GNUC__) && defined(__linux__))
#  include "BCP_linux.hpp"
#endif

#if defined(BLUE_GENE_XLC)
#  include "BCP_bluegene_xlc.hpp"
#endif

#if defined(__CYGWIN__) && defined(__GNUC__) 
#  include "BCP_cygwin.hpp"
#endif

#if defined(_AIX43)
#  include "BCP_aix43.hpp"
#endif

#if defined(__hpux)
#  include "BCP_hpux.hpp"
#endif

#if defined(__sparc) && defined(__sun)
#  include "BCP_sunos.hpp"
#endif

#if defined(__MACH__) && defined(__GNUC__)
#  include "BCP_MACH.hpp"
#endif

#if (defined(__GNUC__) && defined(__FreeBSD__))
#  include "BCP_freebsd.hpp"
#endif


#if defined(_MSC_VER) && (_MSC_VER > 1300)
#  include "BCP_msvc.hpp"
#endif

#endif
