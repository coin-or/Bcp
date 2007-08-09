// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_USER_FUN_H
#define _BCP_TM_USER_FUN_H

// This file is fully docified.

#include "BCP_math.hpp"
#include "BCP_enum.hpp"
#include "BCP_enum_process_t.hpp"
#include "BCP_vector.hpp"
#include "BCP_string.hpp"
#include "BCP_buffer.hpp"
#include "BCP_solution.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_matrix.hpp"
#include "BCP_tm_param.hpp"
#include "BCP_tm_node.hpp"
#include "BCP_enum_tm.hpp"

class BCP_lp_statistics;

//#############################################################################

/**
   The BCP_tm_user class is the base class from which the user
   can derive a problem specific class to be used in the TM process.

   In that derived class the user can store data to be used in the methods she
   overrides. Also that is the object the user must return in the
   USER_initialize::tm_init() method.
   
   There are two kind of methods in the class. The non-virtual methods are
   helper functions for the built-in defaults, but the user can use them as
   well. The virtual methods execute steps in the BCP algorithm where the user
   might want to override the default behavior.

   The default implementations fall into three major categories. 
   <ul>
     <li> Empty; doesn't do anything and immediately returns. (E.g., 
          pack_module_data().
     <li> There is no reasonable default, so throw an exception. This happens
          if the parameter settings drive the flow of in a way that BCP can't
	  perform the necessary function. This behavior is correct since such
	  methods are invoked only if the parameter settings drive the flow of
	  the algorithm that way, in which case the user better implement those
	  methods. (At the momemnt there is no such method in TM.)
     <li> A default is given. Frequently there are multiple defaults and
          parameters govern which one is selected (e.g.,
	  compare_tree_nodes()).
   </ul>
*/

class BCP_tm_user {
private:
  BCP_tm_prob * p;
public:
  /**@name Methods to set and get the pointer to the BCP_tm_prob
     object. It is unlikely that the users would want to muck around with
     these (especially with the set method!) but they are here to provide
     total control.
  */
  /*@{*/
    /// Set the pointer
    void setTmProblemPointer(BCP_tm_prob * ptr) { p = ptr; }
    /// Get the pointer
    BCP_tm_prob * getTmProblemPointer() { return p; }
  /*@}*/

  /**@name Informational methods for the user. */
  /*@{*/
    /// Return what is the best known upper bound (might be BCP_DBL_MAX)
    double upper_bound() const;
  /*@}*/

  /**@name Methods to get/set BCP parameters on the fly */
  /*@{*/
    ///
    char              get_param(const BCP_tm_par::chr_params key) const;
    ///
    int               get_param(const BCP_tm_par::int_params key) const;
    ///
    double            get_param(const BCP_tm_par::dbl_params key) const;
    ///
    const BCP_string& get_param(const BCP_tm_par::str_params key) const;

    ///
    void set_param(const BCP_tm_par::chr_params key, const bool val);
    /// 
    void set_param(const BCP_tm_par::chr_params key, const char val);
    ///
    void set_param(const BCP_tm_par::int_params key, const int val);
    ///
    void set_param(const BCP_tm_par::dbl_params key, const double val);
    ///
    void set_param(const BCP_tm_par::str_params key, const char * val);
  /*@}*/

  //===========================================================================
  /**@name Constructor, Destructor */
  /*@{*/
    BCP_tm_user() : p(0) {}
    /** Being virtual, the destructor invokes the destructor for the real type
	of the object being deleted. */
    virtual ~BCP_tm_user() {}
  /*@}*/

  //===========================================================================
  // Here are the user defined functions. For each of them a default is given
  // which can be overridden when the concrete user class is defined.
  //===========================================================================

  /**@name Packing and unpacking methods */

  /*@{*/
    /** Pack the initial information (info that the user wants to send over)
	for the process specified by the last argument. The information packed
	here will be unpacked in the <code>unpack_module_data()</code> method
	of the user defined class in the appropriate process. <br>
	Default: empty method.
    */
    virtual void
    pack_module_data(BCP_buffer& buf, BCP_process_t ptype);

    /** Unpack a MIP feasible solution that was packed by the
	BCP_lp_user::pack_feasible_solution() method.

	Default: Unpacks a BCP_solution_generic object. The built-in default
	should be used if and only if the built-in default was used
	in BCP_lp_user::pack_feasible_solution().
    */
    virtual BCP_solution*
    unpack_feasible_solution(BCP_buffer& buf);

    /** Decide whether to replace old_sol with new_sol. When this method is
	invoked it has already been tested that they have the same objective
	function value. The purpose of the method is that the user can have a
	secondary objective function. */
    virtual bool
    replace_solution(const BCP_solution* old_sol, const BCP_solution* new_sol);

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

  //--------------------------------------------------------------------------
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
  //--------------------------------------------------------------------------
  /**@name Initial setup (creating core and root) */
  /*@{*/
    /** Create the core of the problem by filling out the last three arguments.
	These variables/cuts will always stay in the LP relaxation and the
	corresponding matrix is described by the specified matrix. If there is
	no core variable or cut then the returned pointer for to the matrix
	should be a null pointer.

	Default: empty method, meaning that there are no variables/cuts in the
	core and this the core matrix is empty (0 pointer) as well.
    */
     virtual void
     initialize_core(BCP_vec<BCP_var_core*>& vars,
		     BCP_vec<BCP_cut_core*>& cuts,
		     BCP_lp_relax*& matrix);
    //-------------------------------------------------------------------------
    /** Create the set of extra variables and cuts that should be added to the
        formulation in the root node. Also decide how variable pricing shuld be
        done, that is, if column generation is requested in the
        init_new_phase() method of this class then column
        generation should be performed according to \c pricing_status.

        Default: empty method, meaning that no variables/cuts are added, there
	is no user data and no pricing should be done.
    */
     virtual void
     create_root(BCP_vec<BCP_var*>& added_vars,
		 BCP_vec<BCP_cut*>& added_cuts,
		 BCP_user_data*& user_data,
		 BCP_pricing_status& pricing_status);
  /*@}*/

  //--------------------------------------------------------------------------
  /** Display a feasible solution */
  virtual void
  display_feasible_solution(const BCP_solution* sol);
    
  //--------------------------------------------------------------------------
  /** Display user information just before a new node is sent to the LP or
      diving into a node is acknowledged. */
  virtual void
  display_node_information(BCP_tree& search_tree,
			   const BCP_tm_node& node);
    
  //--------------------------------------------------------------------------
   /** Display information after BCP finished processing the search tree. */
  virtual void
  display_final_information(const BCP_lp_statistics& lp_stat);
    
  //---------------------------------------------------------------------------
  /**@name Initialize new phase */
  /*@{*/
    /** Do whatever initialization is necessary before the
        <code>phase</code>-th phase. (E.g., setting the pricing strategy.) */
    virtual void
    init_new_phase(int phase, BCP_column_generation& colgen);
  /*@}*/

  //---------------------------------------------------------------------------
  /**@name Search tree node comparison */
  /*@{*/
    /**@name Compare two search tree nodes. Return true if the first node
       should be processed before the second one.

       Default: The default behavior is controlled by the
       \c TreeSearchStrategy  parameter which is set to
       0 (\c BCP_BestFirstSearch) by default.
    */
    virtual bool compare_tree_nodes(const BCP_tm_node* node0,
				    const BCP_tm_node* node1);
  /*@}*/
};

//#############################################################################

#endif
