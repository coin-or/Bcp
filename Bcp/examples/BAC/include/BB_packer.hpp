// Last edit: 5/20/07
//
// Name:     BB_packer.hpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     5/18/07
//-----------------------------------------------------------------------------
// Copyright (C) 2007, Francois Margot, IBM and others. All Rights Reserved.

#ifndef _BB_PACKER_H
#define _BB_PACKER_H

#include "BCP_USER.hpp"

class BCP_buffer;
class BB_user_data;

/** Class taking care of packing/unpacking */

class BB_packer : public BCP_user_pack {
public:
  
  //-------------------------------------------------------------------------
  /** Pack warmstarting information */
  virtual void
  pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf,
		 bool report_if_default = false)
  {
    if (report_if_default) {
      printf("BCP_user_pack : default pack_warmstart() executed.\n");
    }
    BCP_pack_warmstart(ws, buf);
  }
  
  /** Unpack warmstarting information */
  virtual BCP_warmstart*
  unpack_warmstart(BCP_buffer& buf,
		   bool report_if_default = false)
  {
    if (report_if_default) {
      printf("BCP_user_pack : default unpack_warmstart() executed.\n");
    }
    return BCP_unpack_warmstart(buf);
  }
  
  //-------------------------------------------------------------------------
  /** Pack an algorithmic variable */
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
  {
    throw BCP_fatal_error("\
BCP_user_pack::pack_var_algo() invoked but not overridden!\n");
  }
  
  /** Unpack an algorithmic variable */
  virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf)
  {
    throw BCP_fatal_error("\
BCP_user_pack::unpack_var_algo() invoked but not overridden!\n");
    return 0; // to satisfy aCC on HP-UX
  }
  
  //-------------------------------------------------------------------------
  /** Pack an algorithmic cut */
  virtual void pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);
  
  /** Unpack an algorithmic cut */
  virtual BCP_cut_algo* unpack_cut_algo(BCP_buffer& buf);
  
  //-------------------------------------------------------------------------
  /** Pack an user data */
  virtual void pack_user_data(const BCP_user_data* ud, BCP_buffer& buf);
  
  /** Unpack an user data */
  virtual BCP_user_data* unpack_user_data(BCP_buffer& buf);
};


#endif


