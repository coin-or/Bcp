// Last edit: 5/5/04
//
// Name:     BB_user_data.cpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot.  All Rights Reserved.

#include "BCP_buffer.hpp"
#include "BB_user_data.hpp"
#include "BB_cut.hpp"

using namespace std;

/****************************************************************************/
  MY_user_data::MY_user_data(const int max_size)
    : is_processed(0) {

  // Normally, no modifications required, except possibly modifying
  // the parameters to the constructor

  p_rud = new real_user_data(max_size);
}

/****************************************************************************/
MY_user_data::MY_user_data(BCP_buffer& buf)
{
  // Normally, no modifications required.

  buf.unpack(is_processed);
  buf.unpack(p_rud);
}

/****************************************************************************/
MY_user_data::~MY_user_data() { 

  // Normally, no modifications required.

  if(is_processed && (p_rud != NULL)) {
    delete p_rud;
    p_rud = NULL;
  }
}

/****************************************************************************/
void
MY_user_data::pack(BCP_buffer& buf) const
{
  // Normally, no modifications required.

  buf.pack(is_processed);
  buf.pack(p_rud);
} /* pack */


/****************************************************************************/
void
MY_user_data::print() const
{
  // Normally, no modifications required.

  printf("is_processed: %d\n", is_processed);
  if(p_rud != NULL) {
    p_rud->print();
  }
} /* print */


/****************************************************************************/
real_user_data::real_user_data(const int max_size) {

  /* Add : initialize fields of real_user_data */

  max_card_set_zero = max_size;
  card_set_zero = 0;
  set_zero = new int[max_card_set_zero];
}

/****************************************************************************/
real_user_data::~real_user_data() { 

  /* Add : deletion of complex fields in real_user_data */

  delete[] set_zero;
}

/****************************************************************************/
void
real_user_data::print() const {

  /* Add : print fields of real_user_data */

  printf("max_card_set_zero: %d\n", max_card_set_zero);
  printf("card_set_zero: %d\n", card_set_zero);
  if(card_set_zero > 0) {
    printf("set_zero:\n");
    for(int i=0; i<card_set_zero; i++) {
      printf(" %4d", set_zero[i]);
    }
    printf("\n");
  }
  printf("\n");
} /* print */

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
