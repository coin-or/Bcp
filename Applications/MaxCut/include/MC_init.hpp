// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_INIT_H
#define _MC_INIT_H

#include "BCP_USER.hpp"

class MC_packer : public BCP_user_pack {
  /** Pack an algorithmic cut */
  virtual void pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);

  /** Unpack an algorithmic cut */
  virtual BCP_cut_algo* unpack_cut_algo(BCP_buffer& buf);
};

class MC_initialize : public USER_initialize {
  // Declare this function if not the default single process communication is
  // wanted
  //   BCP_message_environment * msgenv_init();

  BCP_user_pack* packer_init(BCP_user_class* p);
  BCP_tm_user* tm_init(BCP_tm_prob& p,
		       const int argnum, const char * const * arglist);
  BCP_lp_user* lp_init(BCP_lp_prob& p);
};

#endif
