// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_LP_H
#define _MKC_LP_H

#include <cfloat>

#include "BCP_parameters.hpp"
#include "BCP_lp_user.hpp"

#include "MKC_lp_param.hpp"
#include "MKC_knapsack.hpp"
#include "MKC_var.hpp"

//#############################################################################

class MKC_lp : public BCP_lp_user {
public:
  BCP_parameter_set<MKC_lp_par> par;
  double* objhist;
  MKC_knapsack_set kss;

  MKC_knapsack_fixing* ks_fixings;
  BCP_vec<MKC_var*>* enumerated_ks;
  BCP_vec<MKC_var*> input_vars;

  BCP_vec<BCP_var*> generated_vars;

  double best_lb_in_root;
  double start_time;

public:
  MKC_lp() :
    objhist(0), ks_fixings(0), enumerated_ks(0), best_lb_in_root(-DBL_MAX) {}
  ~MKC_lp() {
    for (int i = 0; i < kss.ks_num; ++i) {
      purge_ptr_vector(enumerated_ks[i]);
    }
    delete[] enumerated_ks;
    delete[] ks_fixings;
    delete[] objhist;
    purge_ptr_vector(input_vars);
  }
  //###########################################################################
  // unpack the initial info for the appropriate process
  virtual void
  unpack_module_data(BCP_buffer& buf);
  //---------------------------------------------------------------------------
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);
  //---------------------------------------------------------------------------
  virtual BCP_var_algo*
  unpack_var_algo(BCP_buffer& buf);
  
  //---------------------------------------------------------------------------
  virtual OsiSolverInterface *
  initialize_solver_interface();
  //---------------------------------------------------------------------------
  void
  modify_lp_parameters(OsiSolverInterface* lp, bool in_strong_branching);
  //---------------------------------------------------------------------------
  virtual void
  initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
				  const BCP_vec<BCP_cut*>& cuts,
				  const BCP_vec<BCP_obj_status>& var_status,
				  const BCP_vec<BCP_obj_status>& cut_status,
				  BCP_vec<int>& var_changed_pos,
				  BCP_vec<double>& var_new_bd,
				  BCP_vec<int>& cut_changed_pos,
				  BCP_vec<double>& cut_new_bd);
  //---------------------------------------------------------------------------
  /** Compute a true lower bound for the
      subproblem.

      In case column generation is done the lower bound for the subproblem
      might not be the same as the objective value of the current LP
      relaxation. Here the user has an option to return a true lower
      bound.<br>
      The default implementation returns the objective value of the current
      LP relaxation if no column generation is done, otherwise returns the
      current (somehow previously computed) true lower bound.
  */
  virtual double
  compute_lower_bound(const double old_lower_bound,
		      const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const BCP_vec<BCP_cut*>& cuts);
  //---------------------------------------------------------------------------
  // override default feasiblity testing.
  // we want to do heuristics if it's not feasible
  virtual BCP_solution*
  test_feasibility(const BCP_lp_result& lp_result,
		   const BCP_vec<BCP_var*>& vars,
		   const BCP_vec<BCP_cut*>& cuts);
  //---------------------------------------------------------------------------
  // override default feasible solution packing
  // since we generate heuristic solutions in test_feasibility, we got to
  // pack them here.
  virtual void
  pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol);

  //---------------------------------------------------------------------------
  // restoring feasibility
  // not needed, since the LP formulation is a set packing problem,
  // thus it'll always be feasible. BCP will abort if any of them is called.

  //---------------------------------------------------------------------------
  // Expand the user generated vars into columns
  virtual void
  vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // on what to expand
	       BCP_vec<BCP_var*>& vars,       // what to expand
	       BCP_vec<BCP_col*>& cols,       // the expanded cols
	       // things that the user can use for lifting vars if allowed
	       const BCP_lp_result& lpres,
	       BCP_object_origin origin, bool allow_multiple);
  // This is NOT AN INHERITED FUNCTION, just a shorter form of the previous
  void
  vars_to_cols(const BCP_vec<BCP_cut*>& cuts,  // on what to expand
	       BCP_vec<BCP_var*>& vars,        // what to expand
	       BCP_vec<BCP_col*>& cols);       // the expanded cols
   
  //---------------------------------------------------------------------------
  virtual void
  generate_vars_in_lp(const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const BCP_vec<BCP_cut*>& cuts,
		      const bool before_fathom,
		      BCP_vec<BCP_var*>& new_vars,
		      BCP_vec<BCP_col*>& new_cols);

  void
  generate_vars(const BCP_lp_result& lpres,
		const BCP_vec<BCP_var*>& vars,
		const double rc_bound,
		BCP_vec<BCP_var*>& new_vars);

  //---------------------------------------------------------------------------
  virtual void
  logical_fixing(const BCP_lp_result& lpres,
		 const BCP_vec<BCP_var*>& vars,
		 const BCP_vec<BCP_cut*>& cuts,
		 const BCP_vec<BCP_obj_status>& var_status,
		 const BCP_vec<BCP_obj_status>& cut_status,
		 const int var_bound_changes_since_logical_fixing,
		 BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd);
  //---------------------------------------------------------------------------
  virtual BCP_branching_decision
  select_branching_candidates(const BCP_lp_result& lpres,
			      const BCP_vec<BCP_var*>& vars,
			      const BCP_vec<BCP_cut*>& cuts,
			      const BCP_lp_var_pool& local_var_pool,
			      const BCP_lp_cut_pool& local_cut_pool,
			      BCP_vec<BCP_lp_branching_object*>& candidates);
};

#endif
