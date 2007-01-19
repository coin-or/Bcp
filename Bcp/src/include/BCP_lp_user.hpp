// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_USER_H
#define _BCP_LP_USER_H

// This file is fully prepared for doxygen.

#include <vector>

#include "OsiSolverInterface.hpp"
#include "OsiAuxInfo.hpp"

#include "BCP_USER.hpp"

#include "BCP_buffer.hpp"
#include "BCP_vector.hpp"
#include "BCP_string.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_solution.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_matrix.hpp"

#include "BCP_enum.hpp"
#include "BCP_enum_branch.hpp"

#include "BCP_lp_param.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp_branch.hpp"

//#############################################################################

class BCP_lp_prob;

//#############################################################################

// All these functions are defined in BCP_lp_user.cpp, except for those that
// have no counterparts in p.defaults, i.e., for those that have no builtin
// pairs.

//#############################################################################

/**
   The BCP_lp_user class is the base class from which the user
   can derive a problem specific class to be used in the LP process.
   
   In that derived class the user can store data to be used in the methods she
   overrides. Also that is the object the user must return in the
   USER_initialize::lp_init() method.
   
   There are two kind of methods in the class. The non-virtual methods are
   helper functions for the built-in defaults, but the user can use them as
   well. The virtual methods execute steps in the BCP algorithm where the user
   might want to override the default behavior.

   The default implementations fall into three major categories. 
   <ul>
     <li> Empty; doesn't do anything and immediately returns (e.g., 
          unpack_module_data()).
     <li> There is no reasonable default, so throw an exception. This happens
          if the parameter settings drive the flow of in a way that BCP can't
	  perform the necessary function. This behavior is correct since such
	  methods are invoked only if the parameter settings drive the flow of
	  the algorithm that way, in which case the user better implement those
	  methods (e.g., create_indexed_var()).
     <li> A default is given. Frequently there are multiple defaults and
          parameters govern which one is selected (e.g., test_feasibility()).
   </ul>
*/

class BCP_lp_user {
private:
    BCP_lp_user(const BCP_lp_user&);
    BCP_lp_user& operator=(const BCP_lp_user&);

private:
    bool using_deprecated_set_user_data_for_children;
    BCP_lp_prob * p;
    OsiBabSolver* babSolver_;

public:
    /**@name Methods to set and get the pointer to the BCP_lp_prob
       object. It is unlikely that the users would want to muck around with
       these (especially with the set method!) but they are here to provide
       total control.
    */
    /*@{*/
    /// Set the pointer
    void setLpProblemPointer(BCP_lp_prob * ptr) { p = ptr; }
    /// Get the pointer
    BCP_lp_prob * getLpProblemPointer() { return p; }
    /*@}*/

    void setOsiBabSolver(OsiBabSolver* ptr) { babSolver_ = ptr; }
    OsiBabSolver* getOsiBabSolver() { return babSolver_; }
    
    /**@name Informational methods for the user. */
    /*@{*/
    /// Return what is the best known upper bound (might be BCP_DBL_MAX)
    double upper_bound() const;
    /// Return the phase the algorithm is in
    int current_phase() const;
    /// Return the level of the search tree node being processed
    int current_level() const;
    /// Return the internal index of the search tree node being processed
    int current_index() const;
    /// Return the iteration count within the search tree node being processed
    int current_iteration() const;
    /** Return a pointer to the BCP_user_data structure the user (may have)
	stored in this node */
    BCP_user_data* get_user_data();
    /*@}*/

    /**@name Methods to get/set BCP parameters on the fly */
    /*@{*/
    ///
    char              get_param(const BCP_lp_par::chr_params key) const;
    ///
    int               get_param(const BCP_lp_par::int_params key) const;
    ///
    double            get_param(const BCP_lp_par::dbl_params key) const;
    ///
    const BCP_string& get_param(const BCP_lp_par::str_params key) const;

    ///
    void set_param(const BCP_lp_par::chr_params key, const bool val);
    /// 
    void set_param(const BCP_lp_par::chr_params key, const char val);
    ///
    void set_param(const BCP_lp_par::int_params key, const int val);
    ///
    void set_param(const BCP_lp_par::dbl_params key, const double val);
    ///
    void set_param(const BCP_lp_par::str_params key, const char * val);
    /*@}*/

    /**@name A methods to send a solution to the Tree Manager. The user can
       invoke this method at any time to send off a solution. */
    void send_feasible_solution(const BCP_solution* sol);
    //=========================================================================
    /**@name Constructor, Destructor */
    /*@{*/
    BCP_lp_user() : p(0), babSolver_(0) {}
    /** Being virtual, the destructor invokes the destructor for the real type
	of the object being deleted. */
    virtual ~BCP_lp_user() {}
    /*@}*/
   
    //=========================================================================
    /**@name Helper functions for selecting subset of entries from a double
       vector.
       The indices (their position with respect to <code>first</code>) of the
       variables satisfying the criteria are returned in the last argument. */
    /*@{*/
    /** Select all nonzero entries. Those are considered nonzero that have
        absolute value greater than <code>etol</code>. */
    void
    select_nonzeros(const double * first, const double * last,
		    const double etol, BCP_vec<int>& nonzeros) const;
    /** Select all zero entries. Those are considered zero that have
        absolute value less than <code>etol</code>. */
    void
    select_zeros(const double * first, const double * last,
		 const double etol, BCP_vec<int>& zeros) const;
    /** Select all positive entries. Those are considered positive that have
        value greater than <code>etol</code>. */
    void
    select_positives(const double * first, const double * last,
		     const double etol, BCP_vec<int>& positives) const;
    /** Select all fractional entries. Those are considered fractional that are
        further than <code>etol</code> away from any integer value. */
    void
    select_fractions(const double * first, const double * last,
		     const double etol, BCP_vec<int>& fractions) const;
    /*@}*/

    //=========================================================================
    /**@name Packing and unpacking methods */
    /*@{*/
    /** Unpack the initial information sent to the LP process by the Tree
        Manager. This information was packed by the method
        BCP_tm_user::pack_module_data() invoked with \c BCP_ProcessType_LP
	as the third (target process type) argument.
	
        Default: empty method. */
    virtual void
    unpack_module_data(BCP_buffer & buf);

    /**@name Methods that pack/unpack warmstart, var_algo and cut_algo objects.

       The packing methods take an object and a buffer as
       an argument and the user is supposed to pack the object into the buffer.

       The argument of the unpacking methods is just the buffer. The user
       is supposed to return a pointer to the unpacked object.
    */
    /*@{*/
    /** Pack warmstarting information */
    virtual void
    pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf);
    /** Unpack warmstarting information */
    virtual BCP_warmstart*
    unpack_warmstart(BCP_buffer& buf);
    
    /** Pack an algorithmic variable */
    virtual void
    pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);
    /** Unpack an algorithmic variable */
    virtual BCP_var_algo*
    unpack_var_algo(BCP_buffer& buf);
      
    /** Pack an algorithmic cut */
    virtual void
    pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf);
    /** Unpack an algorithmic cut */
    virtual BCP_cut_algo*
    unpack_cut_algo(BCP_buffer& buf);

    /** Pack an user data */
    virtual void
    pack_user_data(const BCP_user_data* ud, BCP_buffer& buf);
    /** Unpack an user data */
    virtual BCP_user_data*
    unpack_user_data(BCP_buffer& buf);
    /*@}*/
    /*@}*/

    //=========================================================================
    /** What is the process id of the current process */
    const BCP_proc_id* process_id() const;
    /** Send a message to a particular process */
    void
    send_message(const BCP_proc_id* const target, const BCP_buffer& buf);
    /** Broadcast the message to all processes of the given type */
    void
    broadcast_message(const BCP_process_t proc_type, const BCP_buffer& buf);
    /** Process a message that has been sent by another process' user part to
	this process' user part. */
    virtual void
    process_message(BCP_buffer& buf);
    //=========================================================================
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
    
    //=========================================================================
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

    //=========================================================================
    /** Modify parameters of the LP solver before optimization.
	This method provides an opportunity for the user to change parameters
	of the LP solver before optimization in the LP solver starts. The
	second argument indicates whether the optimization is a "regular"
	optimization or it will take place in strong branching.
	Default: empty method. 
    */
    virtual void
    modify_lp_parameters(OsiSolverInterface* lp, bool in_strong_branching);

  //===========================================================================
    /** Process the result of an iteration. This includes:
	- computing a true lower bound on the subproblem. <br>
	  In case column generation is done the lower bound for the subproblem
	  might not be the same as the objective value of the current LP
	  relaxation. Here the user has an option to return a true lower
	  bound.
	- test feasibility of the solution (or generate a heuristic solution)
	- generating cuts and/or variables
	.
	The reason for the existence of this method is that (especially when
	column generation is done) these tasks are so intertwined that it is
	much easier to execute them in one method instead of in several
	separate methods.

	The default behavior is to do nothing and invoke the individual
	methods one-by-one.

	@param lp_result the result of the most recent LP optimization (IN)
	@param vars      variables currently in the formulation (IN)
	@param cuts      variables currently in the formulation (IN)
	@param old_lower_bound the previously known best lower bound (IN)
        @param new_cuts  the vector of generated cuts (OUT)
        @param new_rows  the correspontding rows(OUT)
        @param new_vars      the vector of generated variables (OUT)
        @param new_cols the correspontding columns(OUT)
    */
    virtual void
    process_lp_result(const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const BCP_vec<BCP_cut*>& cuts,
		      const double old_lower_bound,
		      double& true_lower_bound,
		      BCP_solution*& sol,
		      BCP_vec<BCP_cut*>& new_cuts,
		      BCP_vec<BCP_row*>& new_rows,
		      BCP_vec<BCP_var*>& new_vars,
		      BCP_vec<BCP_col*>& new_cols);

    //=========================================================================
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
       
    //=========================================================================
    /**@name MIP feasibility testing of LP solutions and heuristics */
    /*@{*/
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
    virtual BCP_solution*
    test_feasibility(const BCP_lp_result& lp_result,
		     const BCP_vec<BCP_var*>& vars,
		     const BCP_vec<BCP_cut*>& cuts);
    /**@name
       Helper functions for \c test_feasibility.

       If the solution is feasible a pointer to a BCP_solution_generic object
       is returned. Note that the solutions generated by these helper
       functions <b>DO NOT OWN</b> the pointers in the \c _vars member of the
       solution. Also note that all of these functions assume that the
       specified integer tolerance in larger than the LP primal tolerance
       extracted from \c lpres and that the solution in \c lpres do not
       violate the bounds by more than the LP tolerance.
    */
    /*@{*/
    /** Test whether all variables are 0/1. Note that this method assumes that
	all variables are binary, i.e., their original lower/upper bounds are
	0/1. */
    BCP_solution_generic*
    test_binary(const BCP_lp_result& lpres, const BCP_vec<BCP_var*>& vars,
		const double etol) const;
    /** Test whether all variables are integer. Note that this method assumes
	that all variables are integer. */
    BCP_solution_generic*
    test_integral(const BCP_lp_result& lpres, const BCP_vec<BCP_var*>& vars,
		  const double etol) const;
    /** Test whether the variables specified as integers are really integer. */
    BCP_solution_generic*
    test_full(const BCP_lp_result& lpres, const BCP_vec<BCP_var*>& vars,
	      const double etol) const;
    /*@}*/

    /** Try to generate a heuristic solution (or return one generated during
	cut/variable generation. Return a pointer to the generated solution or
	return a NULL pointer.

	Default: Return a NULL pointer
    */
    virtual BCP_solution*
    generate_heuristic_solution(const BCP_lp_result& lpres,
				const BCP_vec<BCP_var*>& vars,
				const BCP_vec<BCP_cut*>& cuts);
    /*@}*/
  
    //=========================================================================
    /**@name Packing of solutions */
    /*@{*/
    /** Pack a MIP feasible solution into a
	buffer.

	The solution will be unpacked in the Tree Manager by the 
	BCP_tm_user::unpack_feasible_solution() method.

	Default: The default implementation assumes that \c sol is a
	BCP_solution_generic object (containing variables at nonzero level)
	and packs it.
	@param buf (OUT) the buffer to pack into
	@param sol (IN)  the solution to be packed
    */
    virtual void
    pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol);

    //-------------------------------------------------------------------------
    /** Pack the information necessary for cut generation into the
	buffer.

	Note that the name of the method is pack_primal_solution because most
	likely that (or some part of that) will be needed for cut generation.
	However, if the user overrides the method she is free to pack anything
	(of course she'll have to unpack it in CG).

	This information will be sent to the Cut Generator (and possibly to
	the Cut Pool) where the user has to unpack it. If the user uses the
	built-in method here, then the built-in method will be used in the Cut
	Generator as well.

	Default: The content of the message depends on the value of the
	\c PrimalSolForCG parameter in BCP_lp_par. By default
	the variables at nonzero level are packed.
	@param buf       (OUT) the buffer to pack into
	@param lp_result (IN) the result of the most recent LP optimization
	@param vars      (IN) variables currently in the formulation
	@param cuts      (IN) cuts currently in the formulation
    */
    virtual void
    pack_primal_solution(BCP_buffer& buf,
			 const BCP_lp_result& lp_result,
			 const BCP_vec<BCP_var*>& vars,
			 const BCP_vec<BCP_cut*>& cuts);
    //-------------------------------------------------------------------------
    /** Pack the information necessary for variable generation into the
	buffer.

	Note that the name of the method is pack_dual_solution because most
	likely that (or some part of that) will be needed for variable
	generation. However, if the user overrides the method she is free to
	pack anything (of course she'll have to unpack it in CG).

	This information will be sent to the Variable Generator (and
	possibly to the Variable Pool) where the user has to unpack it. If the
	user uses the built-in method here, then the built-in method will be
	used in the Variable Generator as well.

	Default: The content of the message depends on the value of the
	\c DualSolForVG parameter in BCP_lp_par. By default
	the full dual solution is packed.
	@param buf       (OUT) the buffer to pack into
	@param lp_result (IN) the result of the most recent LP optimization
	@param vars      (IN) variables currently in the formulation
	@param cuts      (IN) cuts currently in the formulation
    */
    virtual void
    pack_dual_solution(BCP_buffer& buf,
		       const BCP_lp_result& lp_result,
		       const BCP_vec<BCP_var*>& vars,
		       const BCP_vec<BCP_cut*>& cuts);
    /*@}*/

    //=========================================================================
    /**@name Displaying of LP solutions */
    /*@{*/
    /** Display the result of most recent LP
	optimization.

	This method is invoked every time an LP relaxation is optimized and
	the user can display (or not display) it.

	Note that this method is invoked only if \c final_lp_solution is true
	(i.e., no cuts/variables were found) and the
	\c LpVerb_FinalRelaxedSolution parameter of BCP_lp_par is set
	to true (or alternatively, \c final_lp_solution is false and 
	\c LpVerb_RelaxedSolution is true).

	Default: display the solution if the appropriate verbosity code entry
	is set.
	@param lp_result (IN) the result of the most recent LP optimization
	@param vars      (IN) variables currently in the formulation
	@param final_lp_solution (IN) whether the lp solution is final or not.
    */
    virtual void
    display_lp_solution(const BCP_lp_result& lp_result,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts,
			const bool final_lp_solution);
    /*@}*/

    //=========================================================================
    // Functions related to indexed vars. Must be written if BCP is supposed to
    // track the indexed variables yet to be priced out, i.e., if the parameter
    // MaintainIndexedVarPricingList is set to true.

    /**@name
       Methods related to indexed variables.

       These methods must be overridden if and only if BCP is supposed to
       track the indexed variables yet to be priced out, i.e., if the
       parameter MaintainIndexedVarPricingList is set to true.
    */
    // *FIXME* : A link here to the description of various variable types?
    /*@{*/
    /** Return the index of the indexed variable following
	<code>prev_index</code>.
	Return -1 if there are no more indexed variables. If
	<code>prev_index</code> is -1 then return the index of the first
	indexed variable. <br> Default: Return -1. */
    virtual int
    next_indexed_var(int prev_index);
    /** Create the variable corresponding to the given <code>index</code>. The
	routine should return a pointer to a newly created indexed variable
	and return the corresponding column in <code>col</code>.
	<br>
	Default: throw an exception.
    */
    virtual BCP_var_indexed*
    create_indexed_var(int index, const BCP_vec<BCP_cut*>& cuts,
		       BCP_col& col);
    /*@}*/

    //=========================================================================
    /** Restoring feasibility.

        This method is invoked before fathoming a search tree node that has
	been found infeasible <em>and</em> the variable pricing did not
	generate any new variables.

	If the MaintainIndexedVarPricingList is set to true then BCP will take
	care of going through the indexed variables to see if any will restore
	feasibility and the user has to check only the algorithmic variables.
	Otherwise the user has to check all variables here.
    */
    virtual void
    restore_feasibility(const BCP_lp_result& lpres,
			const std::vector<double*> dual_rays,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts,
			BCP_vec<BCP_var*>& vars_to_add,
			BCP_vec<BCP_col*>& cols_to_add);

    //=========================================================================
    /**@name Converting cuts and variables into rows and columns */
    /*@{*/
    /** Convert (and possibly lift) a set of cuts into corresponding rows for
        the current LP relaxation. Converting means computing for each cut the
        coefficients corresponding to each variable and creating
        BCP_row objects that can be added to the formulation.
    
        This method has different purposes depending on the value of the last
        argument. If multiple expansion is not allowed then the user must
        generate a unique row for each cut. This unique row must always be the
        same for any given cut. This kind of operation is needed so that
        an LP relaxation can be exactly recreated.
    
        On the other hand if multiple expansion is allowed then the user has
        (almost) free reign over what she returns. She can delete some of the
        <code>cuts</code> or append new ones (e.g., lifted ones) to the end.
        The result of the LP relaxation and the origin of the cuts are there to
        help her to make a decision about what to do. For example, she might
        want to lift cuts coming from the Cut Generator, but not those coming
        from the Cut Pool. The only requirement is that when this method
        returns the number of cuts and rows must be the same and the i-th row
        must be the unique row corresponding to the i-th cut.
        
        @param vars    the variables currently in the relaxation (IN)
        @param cuts    the cuts to be converted (IN/OUT)
        @param rows    the rows into which the cuts are converted (OUT)
        @param lpres   solution to the current LP relaxation (IN)
        @param origin  where the cuts come from (IN)
        @param allow_multiple whether multiple expansion, i.e., lifting, is
	allowed (IN)
    
        Default: throw an exception (if this method is invoked then the user
        must have generated cuts and BCP has no way to know how to convert
        them).
    */
    virtual void
    cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
		 BCP_vec<BCP_cut*>& cuts,       // what to expand
		 BCP_vec<BCP_row*>& rows,       // the expanded rows
		 // things that the user can use for lifting cuts if allowed
		 const BCP_lp_result& lpres,
		 BCP_object_origin origin, bool allow_multiple);
    //-------------------------------------------------------------------------
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
    /*@}*/

    //=========================================================================
    /**@name Generating cuts and variables */
    /*@{*/
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
    virtual void
    generate_cuts_in_lp(const BCP_lp_result& lpres,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts,
			BCP_vec<BCP_cut*>& new_cuts,
			BCP_vec<BCP_row*>& new_rows);
    //-------------------------------------------------------------------------
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
    //-------------------------------------------------------------------------
    /** Compare two generated cuts. Cuts are generated in different iterations,
        they come from the Cut Pool, etc. There is a very real possibility that
        the LP process receives several cuts that are either identical or one
        of them is better then another (cuts off everything the other cuts
        off). This routine is used to decide which one to keep if not both.<br>
        Default: Return \c BCP_DifferentObjs.
    */
    virtual BCP_object_compare_result
    compare_cuts(const BCP_cut* c0, const BCP_cut* c1);
    //-------------------------------------------------------------------------
    /** Compare two generated variables. Variables are generated in different
        iterations, they come from the Variable Pool, etc. There is a very real
        possibility that the LP process receives several variables that are
        either identical or one of them is better then another (e.g., almost
        identical but has much lower reduced cost). This routine is used to
        decide which one to keep if not both.<br>
        Default: Return \c BCP_DifferentObjs.
    */
    virtual BCP_object_compare_result
    compare_vars(const BCP_var* v0, const BCP_var* v1);
    /*@}*/

    //=========================================================================
    virtual void
    select_vars_to_delete(const BCP_lp_result& lpres,
			  const BCP_vec<BCP_var*>& vars,
			  const BCP_vec<BCP_cut*>& cuts,
			  const bool before_fathom,
			  BCP_vec<int>& deletable);
    virtual void
    select_cuts_to_delete(const BCP_lp_result& lpres,
			  const BCP_vec<BCP_var*>& vars,
			  const BCP_vec<BCP_cut*>& cuts,
			  const bool before_fathom,
			  BCP_vec<int>& deletable);
    //=========================================================================
    /**@name Logical fixing */
    /*@{*/
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
    virtual void
    logical_fixing(const BCP_lp_result& lpres,
		   const BCP_vec<BCP_var*>& vars,
		   const BCP_vec<BCP_cut*>& cuts,
		   const BCP_vec<BCP_obj_status>& var_status,
		   const BCP_vec<BCP_obj_status>& cut_status,
		   const int var_bound_changes_since_logical_fixing,
		   BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd);
    /*@}*/

    /** Reduced cost fixing. This is not exactly a helper function, but the user
	might want to invoke it... */
    void
    reduced_cost_fixing(const double* dj, const double* x, const double gap,
			BCP_vec<BCP_var*>& vars, int& newly_changed);

    //=========================================================================
    /**@name Branching related methods */
    /*@{*/
    /** Decide whether to branch or not and select a set of branching
        candidates if branching is decided upon.
        
        The return value indicates what should be done: branching, continuing
        with the same node or abandoning the node completely.

	Default: Branch if both local pools are empty. If branching is done
	then several (based on the \c StrongBranch_CloseToHalfNum
	and \c StrongBranch_CloseToOneNum parameters in
	BCP_lp_par) variables are selected for strong branching.

	"Close-to-half" variables are those that should be integer and are at a
	fractional level. The measure of their fractionality is their distance
	from the closest integer. The most fractional variables will be
	selected, i.e., those that are close to half. If there are too many
	such variables then those with higher objective value have priority.

	"Close-to-on" is interpreted in a more literal sense. It should be used
	only if the integer variables are binary as it select those fractional
	variables which are away from 1 but are still close. If there are too
	many such variables then those with lower objective value have
	priority.

	@param lpres the result of the most recent LP optimization.
	@param vars the variables in the current formulation.
	@param cuts the cuts in the current formulation.
	@param local_var_pool the local pool that holds variables with negative
	reduced cost. In case of continuing with the node the best so
	many variables will be added to the formulation (those with the
	most negative reduced cost).
	@param local_cut_pool the local pool that holds violated cuts. In case
	of continuing with the node the best so many cuts will be added
	to the formulation (the most violated ones).
	@param cands the generated branching candidates. */
    virtual BCP_branching_decision
    select_branching_candidates(const BCP_lp_result& lpres,
				const BCP_vec<BCP_var*>& vars,
				const BCP_vec<BCP_cut*>& cuts,
				const BCP_lp_var_pool& local_var_pool,
				const BCP_lp_cut_pool& local_cut_pool,
				BCP_vec<BCP_lp_branching_object*>& cands);
    /**@name Helper functions for select_branching_candidates() */
    /*@{*/
    /** Select the "close-to-half" variables for strong branching. Variables
	that are at least <code>etol</code> away from integrality are
	considered and <code>to_be_selected</code> of them will be picked up.
    */
    void
    branch_close_to_half(const BCP_lp_result& lpres,
			 const BCP_vec<BCP_var*>& vars,
			 const int to_be_selected,
			 const double etol,
			 BCP_vec<BCP_lp_branching_object*>& candidates);
    /** Select the "close-to-one" variables for strong branching. Variables
	that are at least <code>etol</code> away from integrality are
	considered and <code>to_be_selected</code> of them will be picked up.
    */
    void
    branch_close_to_one(const BCP_lp_result& lpres,
			const BCP_vec<BCP_var*>& vars,
			const int to_be_selected,
			const double etol,
			BCP_vec<BCP_lp_branching_object*>& candidates);
    /** This helper method creates branching variable candidates and appends
	them to <code>cans</code>. The indices (in the current formulation)
	of the variables from which candidates should be created are listed
	in <code>select_pos</code>. */
    void
    append_branching_vars(const double* x,
			  const BCP_vec<BCP_var*>& vars,
			  const BCP_vec<int>& select_pos,
			  BCP_vec<BCP_lp_branching_object*>& candidates);
    /*@}*/
  
    /** Decide which branching object is preferred for
	branching.
  
	Based on the member fields of the two presolved candidate branching
	objects decide which one should be preferred for really branching on
	it. Possible return values are: <code>BCP_OldPresolvedIsBetter</code>,
	\c BCP_NewPresolvedIsBetter and \c BCP_NewPresolvedIsBetter_BranchOnIt.
	This last value (besides specifying which candidate is preferred) also
	indicates that no further candidates should be examined, branching
	should be done on this candidate.

	Default: The behavior of this method is governed by the
	\c BranchingObjectComparison parameter in BCP_lp_par.
    */
    virtual BCP_branching_object_relation
    compare_branching_candidates(BCP_presolved_lp_brobj* new_solved,
				 BCP_presolved_lp_brobj* old_solved);
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

    /** For each child create a user data object and put it into the
	appropriate entry in <code>best->user_data()</code>. When this
	function is called the <code>best->user_data()</code> vector is
	already the right size and is filled will 0 pointers. The second
	argument is usefule if strong branching was done. It is the index of
	the branching candidate that was selected for branching (the one
	that's the source of <code>best</code>.
    */
    virtual void
    set_user_data_for_children(BCP_presolved_lp_brobj* best, 
			       const int selected);
    /** Deprecated version of the previos method (it does not pass the index
        of the selected branching candidate).
    */
    virtual void
    set_user_data_for_children(BCP_presolved_lp_brobj* best);
    /*@}*/
      
    //=========================================================================
    // purging the slack cut pool (candidates for branching on cut)
    /**@name Purging the slack pool */
    /*@{*/
    /** Selectively purge the list of slack
	cuts.

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
	@param to_be_purged the indices of the cuts to be purged. (OUT)
    */
    virtual void
    purge_slack_pool(const BCP_vec<BCP_cut*>& slack_pool,
		     BCP_vec<int>& to_be_purged);
    /*@}*/
};

//#############################################################################

#endif
