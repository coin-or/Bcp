// Last edit: 2/10/05
//
// Name:     BB_tm.hpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BB_TM_H
#define _BB_TM_H

#include "BCP_tm_user.hpp"
#include "BB.hpp"

class BB_tm : public BCP_tm_user {

private:

  /**@name Private data member */
  //@{
  /// Pointer to the description of the problem
   BB_prob desc;
  //@}

public:

  /**@name Constructors and destructors */
  //@{
  /// Default constructor 
  BB_tm() {}

  /// Default destructor
  virtual ~BB_tm() {}
  //@}

  /// Read input and set up data in class BB_prob
  void readInput(const char* filename);


  /**@name Packing and unpacking methods */
  //@{
  /// Pack data into a buffer; will not work in parallel environment
  /// as it uses pointer 
  virtual void pack_module_data(BCP_buffer& buf, BCP_process_t ptype);

  /// Packing of algorithmic cuts
  virtual void pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);

  /// Unpacking of algorithmic cuts
  virtual BCP_cut_algo* unpack_cut_algo(BCP_buffer& buf);

  /// Packing of user data
  virtual void pack_user_data(const BCP_user_data* ud, BCP_buffer& buf);

  /// Unpacking of user_data
  virtual BCP_user_data* unpack_user_data(BCP_buffer& buf);

  //@}

  /// Pass the core constraints and core variables to bcp
  virtual void initialize_core(BCP_vec<BCP_var_core*>& vars,
		  BCP_vec<BCP_cut_core*>& cuts,
		  BCP_lp_relax*& matrix);
  
  /// Create the root node of the enumeration
  virtual void create_root(BCP_vec<BCP_var*>& added_vars,
			   BCP_vec<BCP_cut*>& added_cuts,
			   BCP_user_data*& user_data);

  /// Print a feasible solution
  virtual void display_feasible_solution(const BCP_solution* sol);

};
#endif
