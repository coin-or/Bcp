// Last edit: 1/21/09
//
// Name:     BB_lp.hpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BB_LP_H
#define _BB_LP_H

#include "BB.hpp"
#include "BCP_lp_user.hpp"

#include "BB_user_data.hpp"

  /** Class for operations at the node level */

class BB_lp : public BCP_lp_user {

  /// Pointer on the description of the problem
  BB_prob *p_desc;     

  /// Pointer on the description of the user data
  MY_user_data *p_ud;  
  
  /// in_strong = 1 if and only if in strong branching phase
  int in_strong;       

  /// Hold the value of EPSILON of class BB_prob
  double EPS;          

  /// Hold pointers on generated algorithmic cuts
  BCP_vec<BCP_cut*> algo_cuts;    

  /// Hold indices of violated indexed cuts
  std::vector<int> violated_cuts; 

public:

  /**@name Constructors and destructors */
  //@{
  /// Default constructor 
  BB_lp(){}
  
  /// Destructor
  virtual ~BB_lp() {}
  //@}

  /// Unpack data sent from the tree manager
  virtual void unpack_module_data(BCP_buffer & buf);

  // Tell Bcp which solver to use and set parameters of solver.
  /// Called only once at the beginning, from the root node
  virtual OsiSolverInterface *initialize_solver_interface();
  
  /// Initialize data members at the start of processing a new subproblem
  virtual void initialize_new_search_tree_node(
                                 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts,
				 const BCP_vec<BCP_obj_status>& var_status,
				 const BCP_vec<BCP_obj_status>& cut_status,
				 BCP_vec<int>& var_changed_pos,
				 BCP_vec<double>& var_new_bd,
				 BCP_vec<int>& cut_changed_pos,
				 BCP_vec<double>& cut_new_bd);
  
  /// Modify the parameters of the LP solver.
  /// Called at the beginning of the optimization of a node when
  /// the node LP is not yet solved.
  virtual void modify_lp_parameters(OsiSolverInterface* lp,
				    const int changeType,
				    bool in_strong_branching);
  
  /// Test feasibility of the LP solution.
  /// Called after each node LP has been solved.
  /// Called even if the node LP was infeasible
  /// Called also during strong branching
  virtual BCP_solution* test_feasibility(const BCP_lp_result& lp_result,
					 const BCP_vec<BCP_var*>& vars,
					 const BCP_vec<BCP_cut*>& cuts);
  
  /// Perform fixing of variables.  
  /// Called at each iteration, after test_feasibility, if the node is not
  /// fathomable
  virtual void logical_fixing(const BCP_lp_result& lpres,
			      const BCP_vec<BCP_var*>& vars,
			      const BCP_vec<BCP_cut*>& cuts,
			      const BCP_vec<BCP_obj_status>& var_status,
			      const BCP_vec<BCP_obj_status>& cut_status,
			      const int var_bound_changes_since_logical_fixing,
			      BCP_vec<int>& changed_pos, 
			      BCP_vec<double>& new_bd);
  
  /// Cut generation. 
  /// Send to BCP the cuts generated in test_feasibility.
  /// Use this function to generate standard cuts (Knapsack covers,
  /// Lift-and-Project, odd holes, ...).
  virtual void generate_cuts_in_lp(const BCP_lp_result& lpres,
		      const BCP_vec<BCP_var*>& vars,
		      const BCP_vec<BCP_cut*>& cuts,
		      BCP_vec<BCP_cut*>& new_cuts,
		      BCP_vec<BCP_row*>& new_rows);

  /// Simple rounding heuristic 
  virtual BCP_solution* generate_heuristic_solution(
                               const BCP_lp_result& lpres,
			       const BCP_vec<BCP_var*>& vars,
			       const BCP_vec<BCP_cut*>& cuts);

  /// Describes how to get a row of the matrix from the representation of the
  /// cut.
  /// Required method when indexed or algorithmic cuts are used.
  virtual void
  cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
	       BCP_vec<BCP_cut*>& cuts,       // what to expand
	       BCP_vec<BCP_row*>& rows,       // the expanded rows
	       const BCP_lp_result& lpres,
	       BCP_object_origin origin, bool allow_multiple);

  /// Called at the end of each iteration. Possible return values are:
  /// BCP_DoNotBranch_Fathomed : The node should be fathomed without branching;
  /// BCP_DoNotBranch :         BCP should continue to work on this node;
  /// BCP_DoBranch :            Branching must be done. In this case the 
  ///                           method returns the branching object candidates
  ///                           in one of the arguments.

  virtual BCP_branching_decision
  select_branching_candidates(const BCP_lp_result& lpres,
			      const BCP_vec<BCP_var*>& vars,
			      const BCP_vec<BCP_cut*>& cuts,
			      const BCP_lp_var_pool& local_var_pool,
			      const BCP_lp_cut_pool& local_cut_pool,
			      BCP_vec<BCP_lp_branching_object*>& cands,
			      bool force_branch = false);

  /// Set up the user data for the children according to the chosen
  /// branching object
  virtual void set_user_data_for_children(BCP_presolved_lp_brobj* best, 
					  const int selected);
};

#endif
