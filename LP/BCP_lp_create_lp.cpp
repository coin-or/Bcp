// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "OsiSolverInterface.hpp"
#include "CoinWarmStart.hpp"

#include "BCP_matrix.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_functions.hpp"

void BCP_lp_create_lp(BCP_lp_prob& p)
{
  BCP_var_set& vars = p.node->vars;
  BCP_cut_set& cuts = p.node->cuts;
  const int varnum = vars.size();
  const int cutnum = cuts.size();

  BCP_vec<BCP_col*> cols;
  BCP_vec<BCP_row*> rows;

  int bvarnum = p.core->varnum();
  int bcutnum = p.core->cutnum();

  if (varnum == 0 || cutnum == 0){
    throw BCP_fatal_error("No rows or no cols to create a matrix from!\n");
  }

  BCP_lp_relax* m = 0;
  if (bvarnum == 0) {
    // no core vars. doesn't matter if there're any core cuts, the starting
    // matrix is computed the same way
    cols.reserve(varnum);
    p.user->vars_to_cols(cuts, vars, cols,
			 *p.lp_result, BCP_Object_FromTreeManager, false);
    BCP_vec<double> RLB;
    BCP_vec<double> RUB;
    RLB.reserve(cutnum);
    RUB.reserve(cutnum);
    BCP_cut_set::const_iterator ci = cuts.begin();
    BCP_cut_set::const_iterator lastci = cuts.end();
    for ( ; ci != lastci; ++ci) {
      RLB.unchecked_push_back((*ci)->lb());
      RUB.unchecked_push_back((*ci)->ub());
    }
    m = new BCP_lp_relax(cols, RLB, RUB);
    purge_ptr_vector(cols);
  } else {
    if (bcutnum == 0) {
      rows.reserve(cutnum);
      p.user->cuts_to_rows(vars, cuts, rows,
			   *p.lp_result, BCP_Object_FromTreeManager, false);
      BCP_vec<double> CLB;
      BCP_vec<double> CUB;
      BCP_vec<double> OBJ;
      CLB.reserve(varnum);
      CUB.reserve(varnum);
      OBJ.reserve(varnum);
      BCP_var_set::const_iterator vi = vars.begin();
      BCP_var_set::const_iterator lastvi = vars.end();
      for ( ; vi != lastvi; ++vi) {
	CLB.unchecked_push_back((*vi)->lb());
	CUB.unchecked_push_back((*vi)->ub());
	OBJ.unchecked_push_back((*vi)->obj());
      }
      m = new BCP_lp_relax(rows, CLB, CUB, OBJ);
      purge_ptr_vector(rows);
    } else {
      // has core vars and cuts, the starting matrix is the core matrix
      m = p.core->matrix;
    }
  }
   
  // We have the description in p.node. Load it into the lp solver.
  // First load the core matrix
  p.lp_solver->loadProblem(*m,
			   m->ColLowerBound().begin(),
			   m->ColUpperBound().begin(),
			   m->Objective().begin(),
			   m->RowLowerBound().begin(),
			   m->RowUpperBound().begin());

  if (bvarnum > 0 && bcutnum > 0) {
    //-----------------------------------------------------------------------
    // We have to add the 'added' stuff only if we had a core matrix
    //-----------------------------------------------------------------------
    // Add the Named and Algo cols if there are any (and if we have any cols)
    if (varnum > bvarnum && bcutnum > 0) {
      BCP_vec<BCP_var*> added_vars(vars.entry(bvarnum), vars.end());
      BCP_vec<BCP_cut*> core_cuts(cuts.begin(), cuts.entry(bcutnum));
      cols.reserve(vars.size());
      p.user->vars_to_cols(core_cuts, added_vars, cols,
			   *p.lp_result, BCP_Object_FromTreeManager, false);
      BCP_lp_add_cols_to_lp(cols, p.lp_solver);
      purge_ptr_vector(cols);
    }
    //-----------------------------------------------------------------------
    // Add the Named and Algo rows if there are any (and if we have any such
    // rows AND if there are core vars)
    if (cutnum > bcutnum) {
      BCP_vec<BCP_cut*> added_cuts(cuts.entry(bcutnum), cuts.end());
      rows.reserve(added_cuts.size());
      p.user->cuts_to_rows(vars, added_cuts, rows,
			   *p.lp_result, BCP_Object_FromTreeManager, false);
      BCP_lp_add_rows_to_lp(rows, p.lp_solver);
      purge_ptr_vector(rows);
    }
  } else {
    // Otherwise (i.e., if we had no core matrix) we just have to get rid of
    // 'm'.
    delete m;
  }

  // Now fix the bounds
  const int num = std::max(varnum, cutnum);
  BCP_vec<int> ind;
  ind.reserve(num);
  int i = -1;
  while (++i < num)
    ind.unchecked_push_back(i);

  BCP_vec<double> bd;
  bd.reserve(2 * num);
  BCP_var_set::const_iterator vi = vars.begin();
  BCP_var_set::const_iterator lastvi = vars.end();
  for ( ; vi != lastvi; ++vi) {
    bd.unchecked_push_back((*vi)->lb());
    bd.unchecked_push_back((*vi)->ub());
  }
  p.lp_solver->setColSetBounds(ind.begin(), ind.entry(varnum), bd.begin());

  bd.clear();
  BCP_cut_set::const_iterator ci = cuts.begin();
  BCP_cut_set::const_iterator lastci = cuts.end();
  for ( ; ci != lastci; ++ci) {
    bd.unchecked_push_back((*ci)->lb());
    bd.unchecked_push_back((*ci)->ub());
  }
  p.lp_solver->setRowSetBounds(ind.begin(), ind.entry(cutnum), bd.begin());

  // The rows/cols corresponding to the cached cuts/vars are not valid
  if (p.local_cut_pool)
    p.local_cut_pool->rows_are_valid(false);
  if (p.local_var_pool)
    p.local_var_pool->cols_are_valid(false);

  //--------------------------------------------------------------------------
  // The last step is to initialize warmstarting if we can. After
  // warmstarting info is used up we won't need it again. If there will be any
  // warmstarting info needed regarding this node, that info is what we'll
  // get at the end of processing the node. So delete the current ws info.
  if (p.node->warmstart) {
    CoinWarmStart* ws = p.node->warmstart->convert_to_CoinWarmStart();
    p.lp_solver->setWarmStart(ws);
    delete ws;
    delete p.node->warmstart;
    p.node->warmstart = 0;
  }
}
