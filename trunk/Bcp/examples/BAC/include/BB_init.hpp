// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BB_INIT_H
#define _BB_INIT_H

#include "BCP_USER.hpp"

class BB_init : public USER_initialize {

public:

   virtual BCP_tm_user * tm_init(BCP_tm_prob& p,
				 const int argnum,
				 const char * const * arglist);

   virtual BCP_lp_user * lp_init(BCP_lp_prob& p);

   virtual BCP_user_pack* packer_init(BCP_user_class* p);
};

#endif
