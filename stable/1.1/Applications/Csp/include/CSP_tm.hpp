// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_TM_H
#define _CSP_TM_H

#include <OsiClpSolverInterface.hpp>

#include "BCP_parameters.hpp"
#include "BCP_tm_user.hpp"

#include "CSP_tm_param.hpp"
#include "CSP_lp_param.hpp"

#include "CSP.hpp"
#include "CSP_var.hpp"

//#############################################################################

class CSP_var;

//#############################################################################

class CSP_tm : public BCP_tm_user {
public:
   BCP_parameter_set<CSP_tm_par> tm_par;
   BCP_parameter_set<CSP_lp_par> lp_par;
   CSPROBLEM* csproblem;

public:
  // constructor
  CSP_tm() : csproblem(0) {}

  //destructor
  ~CSP_tm() { 
    delete csproblem; 
  }

  //==========================================================================
  // Here are the user defined functions. For each of them a default is given
  // which can be overridden when the concrete user class is defined.
  //==========================================================================
  
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
  // *LL* : we use the generic solution, so the default is OK.
  // *LL* : otherwise override this AND CSP_lp::pack_feasible_solution()
  virtual BCP_solution*
  unpack_feasible_solution(BCP_buffer& buf) {
    return BCP_tm_user::unpack_feasible_solution(buf);
  }
  
  /** Pack an algorithmic variable */
  // *LL* : done
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf) {
    CSP_var_pack(var, buf);
  }
  /** Unpack an algorithmic variable */
  // *LL* : done
  virtual BCP_var_algo*
  unpack_var_algo(BCP_buffer& buf) {
    return CSP_var_unpack(buf);
  }
  
  /** Pack an algorithmic cut */
  // *LL* : needs to be written when we start to add cuts. Not for now.
  virtual void
  pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf) {
    BCP_tm_user::pack_cut_algo(cut, buf);
  }
  /** Unpack an algorithmic cut */
  // *LL* : needs to be written when we start to add cuts. Not for now.
  virtual BCP_cut_algo*
  unpack_cut_algo(BCP_buffer& buf) {
    return BCP_tm_user::unpack_cut_algo(buf);
  }
  
  //--------------------------------------------------------------------------
  /** Create the core of the problem by filling out the last three arguments.
      These variables/cuts will always stay in the LP relaxation and the
      corresponding matrix is described by the specified matrix. If there is
      no core variable or cut then the returned pointer for to the matrix
      should be a null pointer.
      
      Default: empty method, meaning that there are no variables/cuts in the
      core and this the core matrix is empty (0 pointer) as well.
  */
  // *LL* : set packing constraints are the core constraint, no core vars, no
  // *LL* : core matrix.
  virtual void
  initialize_core(BCP_vec<BCP_var_core*>& vars,
		  BCP_vec<BCP_cut_core*>& cuts,
		  BCP_lp_relax*& matrix);
  //--------------------------------------------------------------------------
  /** Create the set of extra variables and cuts that should be added to the
      formulation in the root node. Also decide how variable pricing shuld be
      done, that is, if column generation is requested in the
      init_new_phase() method of this class then column
      generation should be performed according to \c pricing_status.
      
      Default: empty method, meaning that no variables/cuts are added and no
      pricing should be done.
  */
  // *LL* : the enumerated vars are the added vars, and since now we
  // *LL* : enumerate everything, the pricing status is price nothing.
  virtual void
  create_root(BCP_vec<BCP_var*>& added_vars,
			  BCP_vec<BCP_cut*>& added_cuts,
			  BCP_user_data*& user_data,
			  BCP_pricing_status& pricing_status);
  
  //--------------------------------------------------------------------------
  /** Display a feasible solution */
  virtual void
  display_feasible_solution(const BCP_solution* sol);
  
  //--------------------------------------------------------------------------
  /** Do whatever initialization is necessary before the
       <code>phase</code>-th phase. (E.g., setting the pricing strategy.) */
  // *LL* : default is fine, there's just one phase
  virtual void
  init_new_phase(int phase, BCP_column_generation& colgen);
  
  //--------------------------------------------------------------------------
  /**@name Compare two search tree nodes. Return true if the first node
     should be processed before the second one.
     
     Default: The default behavior is controlled by the
     \c TreeSearchStrategy  parameter which is set to
     0 (\c BCP_BestFirstSearch) by default.
  */
  // *LL* : default is fine
  virtual bool compare_tree_nodes(const BCP_tm_node* node0,
				  const BCP_tm_node* node1) {
    return BCP_tm_user::compare_tree_nodes(node0, node1);
  }
};

#endif
