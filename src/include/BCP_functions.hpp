// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BCP_FUNCTIONS_H_
#define _BCP_FUNCTIONS_H_

#include "BcpConfig.h"

class BCP_warmstart;
class BCP_buffer;

BCPLIB_EXPORT void
BCP_pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf);

BCPLIB_EXPORT BCP_warmstart*
BCP_unpack_warmstart(BCP_buffer& buf);

#endif
