// Last edit: 5/19/07
//
// Name:     BB_user_data.hpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot. All Rights Reserved.

#ifndef _BB_UD_H
#define _BB_UD_H

#include "BCP_USER.hpp"

class BCP_buffer;

/**Class handling user data */

/**************************************************************************/
class real_user_data {

  /* Add: fields for real_user_data */

public:

  //@name Public data members */
  //@{
  /// Maximum number of variables that may be set to zero by branching
  /// decisions
  int max_card_set_zero;

  /// Number of variables set to zero by branching descisions 
  int card_set_zero;
  
  /// Variables that have been set to zero by branching decisions
  int *set_zero;
  //@}

  /**@name Constructors and destructors */
  //@{
  /// Constructor 
  real_user_data(const int max_size);

  /// Desctructor
  ~real_user_data();
  //@}

  /// Dump the fields of the class
  void print() const;
};

/** Class taking care of interaction between user data and Bcp */

/**************************************************************************/
class MY_user_data : public BCP_user_data {
public:

  /// Indicator for mmory management: If is_processed = 1, the associated 
  /// user data may be erased
  int is_processed;

  /// Pointer on an object holding the user data itself
  real_user_data *p_rud;

public:

  /**@name Constructors and destructors */
  //@{
  /// Initial constructor 
  MY_user_data(const int max_size);

  /// Constructor from buffer content
  MY_user_data(BCP_buffer& buf);

  /// Destructor
  ~MY_user_data();

  /// Packing  to buffer
  void pack(BCP_buffer& buf) const;

  /// Dump the fields of the class
  void print() const;
};

#endif


