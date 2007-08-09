// Last edit: 5/19/07
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
