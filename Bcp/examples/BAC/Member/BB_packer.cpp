// Last edit: 5/19/07
//
// Name:     BB_packer.cpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     5/18/07
//-----------------------------------------------------------------------------
// Copyright (C) 2007, Francois Margot, IBM and others.  All Rights Reserved.

#include "BCP_buffer.hpp"
#include "BB_cut.hpp"
#include "BB_user_data.hpp"
#include "BB_packer.hpp"

using namespace std;

/****************************************************************************/

void
BB_packer::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
    int typ;
    const BB_indexed_cut* bb_icut = dynamic_cast<const BB_indexed_cut*>(cut);
    if (bb_icut) {
	typ = 0;
	buf.pack(typ);
	bb_icut->pack(buf);
	return;
    }
    const BB_cut* bb_cut = dynamic_cast<const BB_cut*>(cut);
    if (bb_cut) {
	typ = 1;
	buf.pack(typ);
	bb_cut->pack(buf);
	return;
    }
    throw BCP_fatal_error("BB_pack_cut(): unknown cut type.");
}

/****************************************************************************/

BCP_cut_algo* 
BB_packer::unpack_cut_algo(BCP_buffer& buf)
{
    int typ;
    buf.unpack(typ);
    switch (typ) {
    case 0:
	return new BB_indexed_cut(buf);
    case 1:
	return new BB_cut(buf);
    default:
	throw BCP_fatal_error("BB_unpack_cut(): unknown cut type.");
	break;
    }
    return NULL; // fake return
}

/****************************************************************************/

void 
BB_packer::pack_user_data(const BCP_user_data* ud, BCP_buffer& buf)
  // Normally, no modifications required.
{
  const MY_user_data *mud = dynamic_cast<const MY_user_data*> (ud);
  if(!mud)
    throw BCP_fatal_error("BB_lp::pack_user_data() : unknown data type!\n");

  printf("BB_lp::pack_user_data:\n");
  mud->print();
  mud->pack(buf);
}

/****************************************************************************/

BCP_user_data* 
BB_packer::unpack_user_data(BCP_buffer& buf)
  // Normally, no modifications required.
{
  MY_user_data *p_ud = new MY_user_data(buf);
  printf("BB_lp::unpack_user_data:\n");
  p_ud->print();

  if (p_ud->is_processed) {
    p_ud->p_rud = NULL;
    delete(p_ud);
    p_ud = NULL;
    printf("user_data deleted\n");
  }

  return(p_ud); 
}
