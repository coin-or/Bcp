// Last edit: 6/23/06
//
// Name:     BB_lp.cpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <vector>

#include "BB_lp.hpp"
#include "BB_cut.hpp"
#include "OsiClpSolverInterface.hpp"

#include "BCP_math.hpp"
#include "BCP_lp.hpp"
#include "BCP_problem_core.hpp"

using namespace std;

/************************************************************************/
void
BB_lp::unpack_module_data(BCP_buffer& buf)
{
  buf.unpack(p_desc);
  EPS = p_desc->EPSILON;
}

/************************************************************************/
void
BB_lp::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
  const BB_cut* bb_cut = dynamic_cast<const BB_cut*>(cut);
  if (!bb_cut)
    throw BCP_fatal_error("BB_lp::pack_cut_algo() : unknown cut type!\n");

  bb_cut->pack(buf);
  return;
}

/************************************************************************/
BCP_cut_algo*
BB_lp::unpack_cut_algo(BCP_buffer& buf)
{
  return new BB_cut(buf);
}

/************************************************************************/
OsiSolverInterface *
BB_lp::initialize_solver_interface()

  // Called once at the beginning, from the root node

{
  OsiClpSolverInterface * clp = new OsiClpSolverInterface;
  clp->messageHandler()->setLogLevel(0);

  return clp;
}

/************************************************************************/
void
BB_lp::initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
				       const BCP_vec<BCP_cut*>& cuts,
				       const BCP_vec<BCP_obj_status>& vstatus,
				       const BCP_vec<BCP_obj_status>& cstatus,
				       BCP_vec<int>& var_changed_pos,
				       BCP_vec<double>& var_new_bd,
				       BCP_vec<int>& cut_changed_pos,
				       BCP_vec<double>& cut_new_bd)

  // Called at the beginning of the optimization of a node. 
  // The node LP is not yet solved.

{
  in_strong = 0;

#ifdef USER_DATA
  MY_user_data *curr_ud = dynamic_cast<MY_user_data*> (get_user_data());
  curr_ud->is_processed = 1;
#endif

}

/************************************************************************/
void
BB_lp::modify_lp_parameters(OsiSolverInterface* lp, bool in_strong_branching)

  // Called each time the node LP is solved

{
   if (in_strong_branching) {
     in_strong = 1;
     lp->setIntParam(OsiMaxNumIterationHotStart, 50);
   }

   // write the current LP in file lpnode.lp
   lp->writeLp("lpnode", "lp");
   cout << "LP node written in file lpnode.lp" << endl;

   // to write the current LP file in file lpnode.mps use the following:
   // lp->writeMps("lpnode", "mps", 0.0);
   // cout << "LP node written in file lpnode.mps" << endl;

}

/************************************************************************/
BCP_solution*
BB_lp::test_feasibility(const BCP_lp_result& lp_result,
			const BCP_vec<BCP_var*>& vars,
			const BCP_vec<BCP_cut*>& cuts)

  // Called after each node LP has been solved.
  // Called even if the node LP was infeasible
  // Called also during strong branching

{
  // Don't test feasibility during strong branching

  if (in_strong) {
    return(0);
  }
  
  // Don't test feasibility if the node LP was infeasible or
  // termination was not clean

  if(getLpProblemPointer()->lp_solver->isAbandoned() ||
     getLpProblemPointer()->lp_solver->isProvenPrimalInfeasible() ||
     getLpProblemPointer()->lp_solver->isDualObjectiveLimitReached() ||
     getLpProblemPointer()->lp_solver->isIterationLimitReached()) {
    return(0);
  }

  int i, j, k, cn = p_desc->colnum;;
  const double *x = lp_result.x();
  double *check_lhs = new double[p_desc->indexed->getNumRows()];

  // check indexed constraints

  violated_cuts.clear();
  p_desc->indexed->times(lp_result.x(), check_lhs);
  for (i=0; i<p_desc->indexed->getNumRows(); ++i) {
    if ((check_lhs[i] < p_desc->rlb_indexed[i] - EPS) || 
	(check_lhs[i] > p_desc->rub_indexed[i] + EPS))
      violated_cuts.push_back(i);
  }

  delete[] check_lhs;

  OsiRowCut* rcut = new OsiRowCut();
  int *cutind = new int[cn], cut_nz;
  double* cutcoef = new double[cn], cutrhs = 1;

  // check algorithmic cuts

  for(i=0; i<cn; i++) {
    j = (i+1) % cn;
    k = (i+2) % cn;

    cutcoef[0] = 1;
    cutcoef[1] = 1;
    cutcoef[2] = 1;
    cut_nz = 3;

    if(x[i] + x[j] + x[k] > 1 + EPS) {

      // cut is violated 

      cutind[0] = i;
      cutind[1] = j;
      cutind[2] = k;
      
      rcut->setLb(-BCP_DBL_MAX);
      rcut->setUb(cutrhs);
      rcut->setRow(cut_nz, cutind, cutcoef);
      
      BB_cut* cut = new BB_cut(*rcut);
      algo_cuts.push_back(cut);
      
    }
  }
  
  delete rcut;
  delete[] cutind;
  delete[] cutcoef;

  // if all constraints are satisfied, check integrality of the vars

  return (violated_cuts.empty() +  algo_cuts.empty() == 2 ?
	  BCP_lp_user::test_feasibility(lp_result, vars, cuts) : 0);
}

/********************************************************************/
void
BB_lp::logical_fixing(const BCP_lp_result& lpres,
	       const BCP_vec<BCP_var*>& vars,
	       const BCP_vec<BCP_cut*>& cuts,
	       const BCP_vec<BCP_obj_status>& var_status,
	       const BCP_vec<BCP_obj_status>& cut_status,
	       const int var_bound_changes_since_logical_fixing,
	       BCP_vec<int>& changed_pos, BCP_vec<double>& new_bd) 

  // Called at each iteration, after test_feasibility, if the node is not
  // fathomable
{
}

/************************************************************************/
void
BB_lp::generate_cuts_in_lp(const BCP_lp_result& lpres,
			   const BCP_vec<BCP_var*>& vars,
			   const BCP_vec<BCP_cut*>& cuts,
			   BCP_vec<BCP_cut*>& new_cuts,
			   BCP_vec<BCP_row*>& new_rows)

  // Send to BCP the cuts generated in test_feasibility.
  // Use this function to generate standard cuts (Knapsack covers,
  // Lift-and-Project, odd holes, ...).

{
   int i;
   
   for (i=violated_cuts.size()-1; i>=0; --i) {
      const int ind = violated_cuts[i];
      new_cuts.push_back(new BCP_cut_indexed(ind, p_desc->rlb_indexed[ind], 
					     p_desc->rub_indexed[ind]));
   }
   cout << "generate_cuts_in_lp(): found " << new_cuts.size() 
	<< " indexed cuts" << endl;

   for(i=algo_cuts.size()-1; i>=0; --i) {
     new_cuts.push_back(algo_cuts[i]);
   }
   cout << "generate_cuts_in_lp(): found " << algo_cuts.size() 
	<< " algorithmic cuts" << endl;

   algo_cuts.clear();
}

/************************************************************************/
BCP_solution*
BB_lp::generate_heuristic_solution(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts) 
{
#ifdef HEUR_SOL

  // Simple rounding heuristic 

  int i, j, k, cn = p_desc->colnum;
  const double *x = lpres.x();
  double *rounded_x = new double[cn];

  for(i=0; i<cn; i++) {
    if(x[i] > 0.5 + EPS) {
	rounded_x[i] = 1;
    }
    else {
      rounded_x[i] = 0;
    }
  }

  // check if rounded_x is feasible or not

  int nb_rows_core = p_desc->core->getNumRows();
  int nb_rows_indexed = p_desc->indexed->getNumRows();
  int max_core_indexed;

  if(nb_rows_indexed > nb_rows_core) {
    max_core_indexed = nb_rows_indexed;
  }
  else {
    max_core_indexed = nb_rows_core;
  }
  
  double *check_lhs = new double[max_core_indexed];

  // check core constraints

  p_desc->core->times(rounded_x, check_lhs);

  for (i=0; i<nb_rows_core; ++i) {
    if ((check_lhs[i] < p_desc->rlb_core[i] - EPS) || 
	(check_lhs[i] > p_desc->rub_core[i] + EPS)) {

      delete[] check_lhs;
      delete[] rounded_x;
      cout << "generate_heuristic_solution() returns nothing.\n" 
	   << endl;
      return(0);
    }
  }

  // check indexed constraints

  p_desc->indexed->times(rounded_x, check_lhs);
  for (i=0; i<nb_rows_indexed; ++i) {
    if ((check_lhs[i] < p_desc->rlb_indexed[i] - EPS) || 
	(check_lhs[i] > p_desc->rub_indexed[i] + EPS)) {

      delete[] check_lhs;
      delete[] rounded_x;
      cout << "generate_heuristic_solution() returns nothing.\n" 
	   << endl;
      return(0);
    }
  }

  delete[] check_lhs;

  // check algorithmic constraints

  for(i=0; i<cn; i++) {
    j = (i+1) % cn;
    k = (i+2) % cn;

    if(x[i] + x[j] + x[k] > 1 + EPS) {

      delete[] rounded_x;
      cout << "generate_heuristic_solution() returns nothing.\n" 
	   << endl;
      return(0);
    }
  }

  // rounded_x is feasible
    
  BCP_solution_generic *heur_sol = new BCP_solution_generic();
  heur_sol->_delete_vars = false; // otherwise BCP will delete the variables
                                  // appearing in the solution 

  BCP_vec<BCP_var_core *> core_vars = getLpProblemPointer()->core->vars;
  int ind;

  for(i=0; i<cn; i++) {
    ind = core_vars[i]->bcpind();
    if(rounded_x[ind] > EPS) {
      heur_sol->add_entry(core_vars[i], rounded_x[ind]);
    }
  }
  
  delete[] rounded_x;
  cout << "generate_heuristic_solution() returns a solution.\n" 
       << endl;
  return(heur_sol);

#else /* not HEUR_SOL */

  // return nothing

  return(0);
#endif
}

/************************************************************************/
void
BB_lp::cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
                    BCP_vec<BCP_cut*>& cuts,       // what to expand
                    BCP_vec<BCP_row*>& rows,       // the expanded rows
                    // things that the user can use for lifting cuts if allowed
                    const BCP_lp_result& lpres,
                    BCP_object_origin origin, bool allow_multiple)

  // Required function when indexed or algorithmic cuts are used.
  // Describes how to get a row of the matrix from the representation of the
  // cut.

{
  const int cutnum = cuts.size();
  for (int i=0; i<cutnum; ++i) {
      const BCP_cut_indexed *icut =
	dynamic_cast<const BCP_cut_indexed*>(cuts[i]);
      if (icut) {
	const int ind = icut->index();
	rows.push_back(new BCP_row(p_desc->indexed->getVector(ind),
				   p_desc->rlb_indexed[ind], 
				   p_desc->rub_indexed[ind]));
	continue;
      }
      
      const OsiRowCut* bcut = dynamic_cast<const BB_cut*>(cuts[i]);
      if (bcut) {
	rows.push_back(new BCP_row(bcut->row(), bcut->lb(), bcut->ub()));
	continue;
      }
      
      throw BCP_fatal_error("Unknown cut type in cuts_to_rows.\n");
  }
}

/********************************************************************/
BCP_branching_decision
BB_lp::select_branching_candidates(const BCP_lp_result& lpres,
                                   const BCP_vec<BCP_var*>& vars,
                                   const BCP_vec<BCP_cut*>& cuts,
                                   const BCP_lp_var_pool& local_var_pool,
                                   const BCP_lp_cut_pool& local_cut_pool,
                                   BCP_vec<BCP_lp_branching_object*>& cands)
{
#ifdef CUSTOM_BRANCH

  // Called at the end of each iteration. Possible return values are:

  // BCP_DoNotBranch_Fathomed : The node should be fathomed without branching

  // BCP_DoNotBranch :          BCP should continue to work on this node

  // BCP_DoBranch :             Branching must be done. In this case the 
  //                            method returns the branching object candidates 
  //                            in one of the arguments

  // Don't branch if cutting planes have been generated

  if(local_cut_pool.size() > 0) {
    
    cout << "select_branching_candidates() returns BCP_DoNotBranch" 
	 << endl;
    
    return(BCP_DoNotBranch);
  }
  else {
   
    // Branch on the first fractional variable

    int i;
    const double *x = lpres.x();
    BCP_vec<int> select_pos;
    
    for(i=0; i<p_desc->colnum; i++) {
      
      if((x[i] > EPS) && (x[i] < 1-EPS)) {
	select_pos.push_back(i);

	// put in cands all variables whose index are in select_pos

	append_branching_vars(lpres.x(), vars, select_pos, cands);

	cout << "Branching on variable: " << i << " (" << x[i]
	     << ")   depth: " << current_level() << endl;
	break;
      }
    }
  
    if (cands.size() == 0) {
      throw BCP_fatal_error("select_banching_candidate() : No cut in pool but couldn't select branching variable.\n");
    }


    cout << "select_branching_candidates() returns BCP_DoBranch" << endl;

    return BCP_DoBranch;
  }

#else 
  return(BCP_lp_user::select_branching_candidates(lpres, vars, cuts, 
						  local_var_pool, 
						  local_cut_pool, cands));
#endif
}

/**************************************************************************/
void
BB_lp::pack_user_data(const BCP_user_data* ud, BCP_buffer& buf)

  // Normally, no modifications required.
{
  const MY_user_data *mud = dynamic_cast<const MY_user_data*> (ud);
  if(!mud)
    throw BCP_fatal_error("BB_lp::pack_user_data() : unknown data type!\n");

  printf("BB_lp::pack_user_data:\n");
  mud->print();
  mud->pack(buf);

} /* pack_user_data */
    
/**************************************************************************/
MY_user_data*
BB_lp::unpack_user_data(BCP_buffer& buf)

  // Normally, no modifications required.
{
  p_ud = new MY_user_data(buf);
  printf("BB_lp::unpack_user_data:\n");
  p_ud->print();
  return(p_ud); 

} /* unpack_user_data */

/**************************************************************************/
void
BB_lp::set_user_data_for_children(BCP_presolved_lp_brobj* best, 
                                  const int selected)

  // Given the branching decision (parameter "best"; "selected" is the
  // index of the chosen branching decision in the candidate list), 
  // set the user data for the children.

{
#ifdef USER_DATA
  BCP_lp_branching_object *cand = best->candidate();
  MY_user_data *curr_ud = dynamic_cast<MY_user_data*> (get_user_data());
  real_user_data *curr_rud = curr_ud->p_rud;

  for(int i=0; i<cand->child_num; i++) {
    MY_user_data *ud = new MY_user_data(curr_rud->max_card_set_zero);
    real_user_data *rud = ud->p_rud;

    rud->card_set_zero = curr_rud->card_set_zero;

    for(int j=0; j<curr_rud->card_set_zero; j++) {
      rud->set_zero[j] = curr_rud->set_zero[j];
    }

    int ind_br = (*(cand->forced_var_pos))[0];

    if((*(cand->forced_var_bd))[2*i + 1] < EPS) {
      rud->set_zero[curr_rud->card_set_zero] = ind_br;
      (rud->card_set_zero)++;
    }
    best->user_data()[i] = ud;
  }
#endif /* USER_DATA */
} /* set_user_data_for_children */





