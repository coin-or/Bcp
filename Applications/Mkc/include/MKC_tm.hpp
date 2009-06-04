// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_TM_H
#define _MKC_TM_H

#include <OsiClpSolverInterface.hpp>

#include "BCP_parameters.hpp"
#include "BCP_tm_user.hpp"

#include "MKC_tm_param.hpp"
#include "MKC_lp_param.hpp"
#include "MKC_knapsack.hpp"

//#############################################################################

class MKC_var;

//#############################################################################

class MKC_tm : public BCP_tm_user {
public:
  BCP_parameter_set<MKC_tm_par> tm_par;
  BCP_parameter_set<MKC_lp_par> lp_par;
  MKC_knapsack_set kss;
  BCP_vec<MKC_var*> input_vars;
  OsiClpSolverInterface* clp;
public:
  MKC_tm() : clp(0) {}
  ~MKC_tm() {
    if (clp) 
      delete clp;
    purge_ptr_vector(input_vars);
  }
  //===========================================================================
  // Here are the user defined functions. For each of them a default is given
  // which can be overridden when the concrete user class is defined.
  //===========================================================================
  //---------------------------------------------------------------------------
  // pack the initial info for the appropriate process
  virtual void
  pack_module_data(BCP_buffer& buf, BCP_process_t ptype);
  //---------------------------------------------------------------------------
  // unpack an MIP feasible solution
  virtual BCP_solution*
  unpack_feasible_solution(BCP_buffer& buf);
  //---------------------------------------------------------------------------
  // setting the base
  virtual void
  initialize_core(BCP_vec<BCP_var_core*>& vars,
		  BCP_vec<BCP_cut_core*>& cuts,
		  BCP_lp_relax*& matrix);
  //---------------------------------------------------------------------------
  // create the root node
  virtual void
  create_root(BCP_vec<BCP_var*>& added_vars,
	      BCP_vec<BCP_cut*>& added_cuts,
	      BCP_user_data*& user_data);
  //---------------------------------------------------------------------------
  // feasible solution displaying
  virtual void
  display_feasible_solution(const BCP_solution* soln);
  //---------------------------------------------------------------------------
  // various initializations before a new phase (e.g., pricing strategy)
  virtual void
  init_new_phase(int phase,
		 BCP_column_generation& colgen,
		 CoinSearchTreeBase*& candidates);
};

#endif
