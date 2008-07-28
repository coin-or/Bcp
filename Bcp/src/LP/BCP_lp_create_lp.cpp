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
  p.user->load_problem(*p.lp_solver, p.core, p.node->vars, p.node->cuts);
  BCP_var_set& vars = p.node->vars;
  BCP_cut_set& cuts = p.node->cuts;
  const int varnum = vars.size();
  const int cutnum = cuts.size();

  // Now fix the bounds
  const int num = std::max<int>(varnum, cutnum);
  BCP_vec<int> ind;
  ind.reserve(num);
  int i = -1;
  while (++i < num) {
    ind.unchecked_push_back(i);
  }

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
  if (p.local_cut_pool) {
    p.local_cut_pool->rows_are_valid(false);
  }
  if (p.local_var_pool) {
    p.local_var_pool->cols_are_valid(false);
  }

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
