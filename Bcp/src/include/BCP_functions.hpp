// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BCP_FUNCTIONS_H_
#define _BCP_FUNCTIONS_H_

class BCP_warmstart;
class BCP_buffer;

void
BCP_pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf);

BCP_warmstart*
BCP_unpack_warmstart(BCP_buffer& buf);

#endif
