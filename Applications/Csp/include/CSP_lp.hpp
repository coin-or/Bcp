// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_LP_H
#define _CSP_LP_H

#include <cfloat>
#include <map>
#include <vector>

#include <OsiClpSolverInterface.hpp>

#include "BCP_parameters.hpp"
#include "BCP_lp_user.hpp"

#include "CSP_lp_param.hpp"
#include "CSP_var.hpp"
#include "CSP_colgen.hpp"

class CSP_lp : public BCP_lp_user {
  // private data
private:
  // this is where we store the 
  // columns generated in the compute_lower_bound method
  // for use elsewhere in our application. 
   std::vector<PATTERN*> improving_patterns_;

  // public data
public:
   BCP_parameter_set<CSP_lp_par> par;

  // pointer to the static problem information
  // there's only one LP module, so no need to declare this "static"
  CSPROBLEM* csproblem;

  CSP_colgen* colgen;

  // public methods
public:
  // constructor
  CSP_lp() : csproblem(0), colgen(0) {}

  // destructor
  ~CSP_lp() {
    std::for_each(improving_patterns_.begin(),
		  improving_patterns_.end(), DELETEOBJECT());
    delete csproblem;
    delete colgen;
  }

   //==========================================================================
   /** Unpack the initial information sent to the LP process by the Tree
       Manager. This information was packed by the method
       BCP_tm_user::pack_module_data() invoked with \c BCP_ProcessType_LP
       as the third (target process type) argument. */

  // inherited methods
   virtual void
   unpack_module_data(BCP_buffer & buf);

   //==========================================================================
    /** Create LP solver environment.
	Create the LP solver class that will be used for solving the LP
	relaxations. The default implementation picks up which
	\c COIN_USE_XXX is defined and initializes an lp solver of that type.
	This is probably OK for most users. The only reason to override this
	method is to be able to choose at runtime which lp solver to
	instantiate (maybe even different solvers on different processors).
	In this case she should probably also override the
	pack_warmstart() and unpack_warmstart() methods in this class and in
	the BCP_tm_user class. */
   virtual OsiSolverInterface *
   initialize_solver_interface();

   //==========================================================================
    /** Initializing a new search tree node.
	This method serves as hook for the user to do some preprocessing on a
	search tree node before the node is processed. Also, logical fixing
	results can be returned in the last four parameters. This might be very
	useful if the branching implies significant tightening.<br>
	Default: empty method. 
	@param vars       (IN) The variables in the current formulation 
	@param cuts       (IN) The cuts in the current formulation
	@param var_status (IN) The stati of the variables
	@param cut_status (IN) The stati of the cuts
	@param var_changed_pos (OUT) The positions of the variables whose
	                             bounds should be tightened
	@param var_new_bd      (OUT) The new lb/ub of those variables
	@param cut_changed_pos (OUT) The positions of the cuts whose bounds
	                             should be tightened
	@param cut_new_bd (OUT) The new lb/ub of those cuts
    */
   virtual void
   initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
				   const BCP_vec<BCP_cut*>& cuts,
				   const BCP_vec<BCP_obj_status>& var_status,
				   const BCP_vec<BCP_obj_status>& cut_status,
				   BCP_vec<int>& var_changed_pos,
				   BCP_vec<double>& var_new_bd,
				   BCP_vec<int>& cut_changed_pos,
				   BCP_vec<double>& cut_new_bd);

  //==========================================================================
   /** This method provides an opportunity for the user to change parameters
       of the LP solver before optimization in the LP solver starts. The
       second argument indicates whether the optimization is a "regular"
       optimization or it will take place in strong branching.
       Default: empty method. 
   */
   virtual void
   modify_lp_parameters(OsiSolverInterface* lp, const int changeType,
			bool in_strong_branching);

   //==========================================================================
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
   //==========================================================================

    /** Evaluate and return MIP feasibility of the current
	solution.

	If the solution is MIP feasible, return a solution object otherwise
	return a NULL pointer. The useris also welcome to heuristically
	generate a solution and return a pointer to that solution (although
	the user will have another chance (after cuts and variables are
	generated) to return/create heuristically generated solutions. (After
	all, it's quite possible that solutions are generated during
	cut/variable generation.)

	Default: test feasibility based on the \c FeeasibilityTest
	parameter in BCP_lp_par	which defults to \c BCP_FullTest_Feasible.

	@param lp_result the result of the most recent LP optimization
	@param vars      variables currently in the formulation
	@param cuts      variables currently in the formulation
    */
   // *LL* : sufficient to test binary-ness 
#if 0
   virtual BCP_solution*
   test_feasibility(const BCP_lp_result& lpres,
		    const BCP_vec<BCP_var*>& vars,
		    const BCP_vec<BCP_cut*>& cuts);
#endif
   //==========================================================================
   /** Try to generate a heuristic solution (or return one generated during
       cut/variable generation. Return a pointer to the generated solution or
       return a NULL pointer.
   */
  virtual BCP_solution*
  generate_heuristic_solution(const BCP_lp_result& lpres,
			      const BCP_vec<BCP_var*>& vars,
			      const BCP_vec<BCP_cut*>& cuts);
  
   //==========================================================================
   /** Restoring feasibility.
       This method is invoked before fathoming a search tree node that has
       been found infeasible <em>and</em> the variable pricing did not generate
       any new variables.

       The user has to check all variables here. */
   // get one dual ray from the solver, and try to generate columns that cut
   // off the  dual ray. 
   virtual void
   restore_feasibility(const BCP_lp_result& lpres,
		       const std::vector<double*> dual_rays,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       BCP_vec<BCP_var*>& vars_to_add,
		       BCP_vec<BCP_col*>& cols_to_add);

   //==========================================================================
   /** Convert a set of variables into corresponding columns for
       the current LP relaxation. Converting means to compute for each
       variable the coefficients corresponding to each cut and create
       BCP_col objects that can be added to the formulation.
  
       See the documentation of cuts_to_rows() above for the use of
       this method (just reverse the role of cuts and variables.)
  
       @param cuts    the cuts currently in the relaxation (IN)
       @param vars    the variables to be converted (IN/OUT)
       @param cols    the colums the variables convert into (OUT)
       @param lpres   solution to the current LP relaxation (IN)
       @param origin  where the do the cuts come from (IN)
       @param allow_multiple whether multiple expansion, i.e., lifting, is
       allowed (IN)
  
       Default: throw an exception (if this method is invoked then the user
       must have generated variables and BCP has no way to know how to convert
       them).
   */
   virtual void
   vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // on what to expand
		BCP_vec<BCP_var*>& vars,       // what to expand
		BCP_vec<BCP_col*>& cols,       // the expanded cols
		// things that the user can use for lifting vars if allowed
		const BCP_lp_result& lpres,
		BCP_object_origin origin, bool allow_multiple);

   void
   vars_to_cols(BCP_vec<BCP_var*>& vars,
		BCP_vec<BCP_col*>& cols);

   //==========================================================================
   /** Generate cuts within the LP process. Sometimes too much information
       would need to be transmitted for cut generation (e.g., the full tableau
       for Gomory cuts) or the cut generation is so fast that transmitting the
       info would take longer than generating the cuts. In such cases it might
       better to generate the cuts locally. This routine provides the
       opportunity.<br>
       Default: empty for now. To be interfaced to Cgl.
       @param lpres    solution to the current LP relaxation (IN)
       @param vars     the variabless currently in the relaxation (IN)
       @param cuts     the cuts currently in the relaxation (IN)
       @param new_cuts the vector of generated cuts (OUT)
       @param new_rows the correspontding rows(OUT)
   */
   // *LL* : for now we don't have cuts, so the default is fine.
#if 0
   virtual void
   generate_cuts_in_lp(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       BCP_vec<BCP_cut*>& new_cuts,
		       BCP_vec<BCP_row*>& new_rows);
#endif

   //==========================================================================
   /** Generate variables within the LP process. Sometimes too much
       information would need to be transmitted for variable generation or
       the variable generation is so fast that transmitting the info would
       take longer than generating the variables. In such cases it might be
       better to generate the variables locally. This routine provides the
       opportunity.

       Default: empty method.
       @param lpres         solution to the current LP relaxation (IN)
       @param vars          the variabless currently in the relaxation (IN)
       @param cuts          the cuts currently in the relaxation (IN)
       @param before_fathom if true then BCP is about to fathom the node, so
       spend some extra effort generating variables if
       you want to avoid that...
       @param new_vars      the vector of generated variables (OUT)
       @param new_cols the correspontding columns(OUT) */
   virtual void
   generate_vars_in_lp(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts,
		       const bool before_fathom,
		       BCP_vec<BCP_var*>& new_vars,
		       BCP_vec<BCP_col*>& new_cols);

   //==========================================================================
   /** Compare two generated cuts. Cuts are generated in different iterations,
       they come from the Cut Pool, etc. There is a very real possibility that
       the LP process receives several cuts that are either identical or one
       of them is better then another (cuts off everything the other cuts
       off). This routine is used to decide which one to keep if not both.<br>
       Default: Return \c BCP_DifferentObjs.
   */
   // *LL* : for now we don't have cuts, so the default is fine.
#if 0
   virtual BCP_object_compare_result
   compare_cuts(const BCP_cut* c0, const BCP_cut* c1);
#endif

   //==========================================================================
   /** Compare two generated variables. Variables are generated in different
       iterations, they come from the Variable Pool, etc. There is a very real
       possibility that the LP process receives several variables that are
       either identical or one of them is better then another (e.g., almost
       identical but has much lower reduced cost). This routine is used to
       decide which one to keep if not both.<br>
       Default: Return \c BCP_DifferentObjs.
   */
   // *LL* : We just assume that every generated var is different, so default
   // *LL* : is fine. The worst that can happen is a minimal performance
   // *LL* : penalty.
#if 0
   virtual BCP_object_compare_result
   compare_vars(const BCP_var* v0, const BCP_var* v1);
#endif

   //==========================================================================
   /** This method provides an opportunity for the user to tighten the bounds
       of variables. The method is invoked after reduced cost fixing. The
       results are returned in the last two parameters.<br>
       Default: empty method.
       @param lpres the result of the most recent LP optimization,
       @param vars the variables in the current formulation,
       @param status the stati of the variables as known to the system,
       @param var_bound_changes_since_logical_fixing the number of variables
       whose bounds have changed (by reduced cost fixing) since the
       most recent invocation of this method that has actually forced
       changes returned something in the last two arguments,
       @param changed_pos the positions of the variables whose bounds should
       be changed
       @param new_bd the new bounds (lb/ub pairs) of these variables.
   */
   // *LL* : This is not a must, but would be nice to add setpacking matrix
   // *LL* : reduction techniques eventually. For now use the default.
#if 0
   virtual void
   logical_fixing(const BCP_lp_result& lpres,
		  const BCP_vec<BCP_var*>& vars,
		  const BCP_vec<BCP_cut*>& cuts,
		  const BCP_vec<BCP_obj_status>& var_status,
		  const BCP_vec<BCP_obj_status>& cut_status,
		  const int var_bound_changes_since_logical_fixing,
		  BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd);
#endif

   //==========================================================================
   /** Decide whether to branch or not and select a set of branching
       candidates if branching is decided upon. */
   virtual BCP_branching_decision
   select_branching_candidates(const BCP_lp_result& lpres,
			       const BCP_vec<BCP_var*>& vars,
			       const BCP_vec<BCP_cut*>& cuts,
			       const BCP_lp_var_pool& local_var_pool,
			       const BCP_lp_cut_pool& local_cut_pool,
			       BCP_vec<BCP_lp_branching_object*>& cands,
			       bool force_branch = false);
   BCP_lp_branching_object*
   branch_on_half(const BCP_lp_result& lpres,
		  const BCP_vec<BCP_var*>& vars);

   //==========================================================================
   /** Decide which branching object is preferred for branching.
  
       Based on the member fields of the two presolved candidate branching
       objects decide which one should be preferred for really branching on
       it. Possible return values are: <code>BCP_OldPresolvedIsBetter</code>,
       <code>BCP_NewPresolvedIsBetter</code> and
       <code>BCP_NewPresolvedIsBetter_BranchOnIt</code>. This last value
       (besides specifying which candidate is preferred) also indicates that
       no further candidates should be examined, branching should be done on
       this candidate.<br>
       Default: The behavior of this method is governed by the
       \c BranchingObjectComparison parameter in BCP_lp_par.
   */
   // *LL* : Use default for now
   // *LL* : Later we can play with integrality based branching
#if 0
   virtual BCP_branching_object_relation
   compare_branching_candidates(BCP_presolved_lp_brobj* new_solved,
				BCP_presolved_lp_brobj* old_solved);
#endif

   //==========================================================================
   /** Decide what to do with the children of the selected branching object.
       Fill out the <code>_child_action</code> field in <code>best</code>.
       This will specify for every child what to do with it. Possible values
       for each individual child are <code>BCP_PruneChild</code>,
       <code>BCP_ReturnChild</code> and <code>BCP_KeepChild</code>. There can
       be at most child with this last action specified. It means that in case
       of diving this child will be processed by this LP process as the next
       search tree node.

       Default: Every action is <code>BCP_ReturnChild</code>. However, if BCP
       dives then one child will be mark with <code>BCP_KeepChild</code>. The
       decision which child to keep is based on the \c ChildPreference
       parameter in BCP_lp_par. Also, if a child has a presolved lower bound
       that is higher than the current upper bound then that child is mark as
       <code>BCP_FathomChild</code>.<br>

       *THINK*: Should those children be sent back for processing in the next
       phase? 
   */
   virtual void
   set_actions_for_children(BCP_presolved_lp_brobj* best);
      
   //==========================================================================
   /** Selectively purge the list of slack cuts.

       When a cut becomes ineffective and is eventually purged from the LP
       formulation it is moved into <code>slack_pool</code>. The user might
       consider cuts might later for branching. This function enables the user
       to purge any cut from the slack pool (those she wouldn't consider
       anyway). Of course, the user is not restricted to these cuts when
       branching, this is only there to help to collect slack cuts.
       The user should put the indices of the cuts to be purged into the
       provided vector.

       Default: Purges the slack cut pool according to the
       \c SlackCutDiscardingStrategy rule in BCP_lp_par (purge
       everything before every iteration or before a new search tree node).

       @param slack_pool the pool of slacks. (IN)
       @param to_be_purged the indices of the cuts to be purged. (OUT) */
   // *LL* : Used only when branching on dynamically generated cuts is
   // *LL* : allowed, but we don't generate cuts...
#if 0
   virtual void
   purge_slack_pool(const BCP_vec<BCP_cut*>& slack_pool,
		    BCP_vec<int>& to_be_purged);
#endif

   //==========================================================================

};
#endif
