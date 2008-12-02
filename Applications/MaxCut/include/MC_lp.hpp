// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_LP_H
#define _MC_LP_H

#include "BCP_lp_user.hpp"
#include "BCP_parameters.hpp"

#include "MC.hpp"
#include "MC_solution.hpp"
#include "MC_lp_param.hpp"

class OsiVolSolverInterface;
class OsiSolverInterface;

class MC_lp : public BCP_lp_user {
private:
   MC_lp(const MC_lp&);
   MC_lp& operator=(const MC_lp&);
public:
   BCP_parameter_set<MC_lp_par> par;
   MC_problem mc;

   // hold a history of objective values (for tailing off, until it's
   // implemented in BCP
   int hist_len;
   double* objhist;
   // hold the solution generated during cycle cut generation and return it in
   // generate_heuristic_solution()
   MC_solution* soln;
   // Whether we have started to solve the problems to optimality
   bool started_exact;
   bool tried_hard_cuts_in_prev_major_iter;

   // When using the volume alg in general, but solving to optimality with
   // simplex using the reduced costs wrt. the duals of the volume, obj_shift
   // is constant term after the cost transformation
   double obj_shift;
   // the presolved best candidate
   BCP_presolved_lp_brobj* best_presolved;

public:
   MC_lp() : par(), mc(), objhist(0), soln(0), started_exact(false),
	     tried_hard_cuts_in_prev_major_iter(false), best_presolved(0) {}
   ~MC_lp() {
      delete[] objhist;
      delete soln;
   }

   //--------------------------------------------------------------------------
   // unpack the module data for the appropriate process
   virtual void
   unpack_module_data(BCP_buffer & buf);
   //--------------------------------------------------------------------------
   // Pack an algorithmic cut
   virtual void
   pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);
   //--------------------------------------------------------------------------
   // Unpack an algorithmic cut
   virtual BCP_cut_algo*
   unpack_cut_algo(BCP_buffer& buf);
   //--------------------------------------------------------------------------
   // Override the initializer so that we can choose between vol and simplex
   // at runtime.
   virtual OsiSolverInterface *
   initialize_solver_interface();
   
   //--------------------------------------------------------------------------
   // Opportunity to reset things before optimization
   virtual void
   modify_lp_parameters(OsiSolverInterface* lp, bool in_strong_branching);
   //--------------------------------------------------------------------------
   // Feasibility testing
   virtual BCP_solution*
   test_feasibility(const BCP_lp_result& lp_result,
		    const BCP_vec<BCP_var*>& vars,
		    const BCP_vec<BCP_cut*>& cuts);
    /** Try to generate a heuristic solution (or return one generated during
	cut/variable generation. */
   virtual BCP_solution*
   generate_heuristic_solution(const BCP_lp_result& lpres,
			       const BCP_vec<BCP_var*>& vars,
			       const BCP_vec<BCP_cut*>& cuts);
   /** A local helper function. */
   MC_solution*
   mc_generate_heuristic_solution(const double* x,
				  const BCP_vec<BCP_var*>& vars,
				  const BCP_vec<BCP_cut*>& cuts);
   //--------------------------------------------------------------------------
   // feasible solution packing
   virtual void
   pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol);
   //--------------------------------------------------------------------------
   // Convert the user generated cuts into rows
   virtual void
   cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
		BCP_vec<BCP_cut*>& cuts,       // what to expand
		BCP_vec<BCP_row*>& rows,       // the expanded rows
		// things that the user can use for lifting cuts if allowed
		const BCP_lp_result& lpres,
		BCP_object_origin origin, bool allow_multiple);
   //--------------------------------------------------------------------------
   virtual void
   generate_cuts_in_lp(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       BCP_vec<BCP_cut*>& new_cuts,
		       BCP_vec<BCP_row*>& new_rows);
   void
   generate_cuts_in_lp(const double* x, const double* lhs,
		       const double objval,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       BCP_vec<BCP_cut*>& new_cuts,
		       BCP_vec<BCP_row*>& new_rows);
   void
   unique_cycle_cuts(BCP_vec<BCP_cut*>& new_cuts,
		     BCP_vec<BCP_row*>& new_rows);
   void
   generate_mst_cuts(const double* x, const double* lhs,
		     const double objval,
		     const BCP_vec<BCP_var*>& vars,
		     const BCP_vec<BCP_cut*>& cuts,
		     BCP_vec<BCP_cut*>& new_cuts,
		     BCP_vec<BCP_row*>& new_rows);
   void
   generate_sp_cuts(const double* x, const double* lhs,
		    const double objval,
		    const BCP_vec<BCP_var*>& vars,
		    const BCP_vec<BCP_cut*>& cuts,
		    BCP_vec<BCP_cut*>& new_cuts,
		    BCP_vec<BCP_row*>& new_rows);
   double
   getMaxLpViol();
   //--------------------------------------------------------------------------
      virtual BCP_object_compare_result
   compare_cuts(const BCP_cut* c0, const BCP_cut* c1);
   //--------------------------------------------------------------------------
   virtual void
   logical_fixing(const BCP_lp_result& lpres,
		  const BCP_vec<BCP_var*>& vars,
		  const BCP_vec<BCP_cut*>& cuts,
		  const BCP_vec<BCP_obj_status>& var_status,
		  const BCP_vec<BCP_obj_status>& cut_status,
		  const int var_bound_changes_since_logical_fixing,
		  BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd);
   //--------------------------------------------------------------------------
   // Helper functions to check if there has been a tailing off in the past k
   // iterations
   //--------------------------------------------------------------------------
   bool
   is_gap_tailoff_rel(const int k, const double minimp,
		      const double objval) const;
   bool
   is_lb_tailoff_abs(const int k, const double minimp,
		     const double objval) const;
   bool
   is_lb_tailoff_rel(const int k, const double minimp,
		     const double objval) const;
   void
   tailoff_test(bool& tailoff_gap_rel, bool& tailoff_lb_abs,
		bool& tailoff_lb_rel, const double objval) const;
   //--------------------------------------------------------------------------
   OsiSolverInterface* 
   solveToOpt(OsiVolSolverInterface* vollp,
	      const BCP_lp_result& lpres,
	      const BCP_vec<BCP_var*>& vars,
	      const BCP_vec<BCP_cut*>& cuts,
	      double& exact_obj);
   //--------------------------------------------------------------------------
   virtual BCP_branching_decision
   select_branching_candidates(const BCP_lp_result& lpres,
			       const BCP_vec<BCP_var*>& vars,
			       const BCP_vec<BCP_cut*>& cuts,
			       const BCP_lp_var_pool& local_var_pool,
			       const BCP_lp_cut_pool& local_cut_pool,
			       BCP_vec<BCP_lp_branching_object*>& candidates);
   void
   perform_strong_branching(const BCP_lp_result& lpres,
			    OsiSolverInterface* exact_solver,
			    BCP_vec<BCP_lp_branching_object*>& cands);
   void
   choose_branching_vars(const BCP_vec<BCP_var*>& vars, const double * x,
			 const int cand_num,
			 BCP_vec<BCP_lp_branching_object*>& cands);
   //--------------------------------------------------------------------------
   virtual BCP_branching_object_relation
   compare_branching_candidates(BCP_presolved_lp_brobj* new_presolved,
				BCP_presolved_lp_brobj* old_presolved);
   virtual void
   set_actions_for_children(BCP_presolved_lp_brobj* best);
};

#endif
