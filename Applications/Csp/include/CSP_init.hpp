// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_INIT_H
#define _CSP_INIT_H

#include "BCP_USER.hpp"

class CSP_packer : public BCP_user_pack {
  /** Pack an algorithmic var */
  virtual void pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);

  /** Unpack an algorithmic var */
  virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf);

  /** Pack an algorithmic cut */
  // *LL* : needs to be written when we start to add cuts. Not for now.
  virtual void pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf) {
    BCP_user_pack::pack_cut_algo(cut, buf);
  }

  /** Unpack an algorithmic cut */
  // *LL* : needs to be written when we start to add cuts. Not for now.
  virtual BCP_cut_algo* unpack_cut_algo(BCP_buffer& buf) {
    return BCP_user_pack::unpack_cut_algo(buf);
  }
};

class CSP_initialize : public USER_initialize {
  // Declare this function if not the default single process communication is
  // wanted
  // BCP_message_environment * msgenv_init();
  BCP_user_pack* packer_init(BCP_user_class* p);
  BCP_tm_user * tm_init(BCP_tm_prob& p,
			const int argnum, const char * const * arglist);
  BCP_lp_user * lp_init(BCP_lp_prob& p);
};

#endif
