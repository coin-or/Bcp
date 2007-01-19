// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_TM_H
#define _MC_TM_H

#include "BCP_tm_user.hpp"
#include "BCP_parameters.hpp"

#include "MC_tm_param.hpp"
#include "MC_lp_param.hpp"

#include "MC.hpp"
#include "MC_solution.hpp"
#include "MC_cut.hpp"

class MC_tm : public BCP_tm_user {
public:
  BCP_parameter_set<MC_tm_par> tm_par;
  BCP_parameter_set<MC_lp_par> lp_par;
  //   BCP_parameter_set<MC_cg_par> cg_par;
  //   BCP_parameter_set<MC_cg_par> vg_par;

  MC_problem mc;
  MC_solution best_soln;

public:
  MC_tm() : tm_par(), lp_par(), mc(), best_soln() {}
  ~MC_tm() {}

  //--------------------------------------------------------------------------
  // pack the module data for the appropriate process
  void
  pack_module_data(BCP_buffer& buf, BCP_process_t ptype);
  //--------------------------------------------------------------------------
  // unpack an MIP feasible solution
  BCP_solution*
  unpack_feasible_solution(BCP_buffer& buf);
  //--------------------------------------------------------------------------
  // Pack an algorithmic cut
  void
  pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);
  //--------------------------------------------------------------------------
  /** Unpack an algorithmic cut */
  BCP_cut_algo*
  unpack_cut_algo(BCP_buffer& buf);
  //--------------------------------------------------------------------------
  // feasible solution displaying
  void
  display_solution(const BCP_solution* soln);
  //--------------------------------------------------------------------------
  // setting the core
  void
  initialize_core(BCP_vec<BCP_var_core*>& vars,
		  BCP_vec<BCP_cut_core*>& cuts,
		  BCP_lp_relax*& matrix);
  //--------------------------------------------------------------------------
  // create the root node
  void
  create_root(BCP_vec<BCP_var*>& added_vars,
	      BCP_vec<BCP_cut*>& added_cuts,
	      BCP_user_data*& user_data);

  //--------------------------------------------------------------------------
  // Display a feasible solution
  void
  display_feasible_solution(const BCP_solution* sol);
    
  //##########################################################################
};

#endif
