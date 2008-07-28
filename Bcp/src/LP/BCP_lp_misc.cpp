// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_warmstart.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_solution.hpp"

//#############################################################################

void BCP_lp_process_result(BCP_lp_prob& p, const BCP_lp_result& lpres)
{
  p.user_has_lp_result_processing = true;
  p.user->process_lp_result(lpres, p.node->vars, p.node->cuts,
			    p.node->true_lower_bound,
			    p.new_true_lower_bound, p.sol,
			    p.new_cuts, p.new_rows, p.new_vars, p.new_cols);
}

//#############################################################################

void BCP_lp_purge_slack_pool(BCP_lp_prob& p)
{
  BCP_vec<int> purge;
  p.user->purge_slack_pool(p.slack_pool, purge);
  if (purge.size() > 0)
    purge_ptr_vector_by_index(p.slack_pool, purge.begin(), purge.end());
}

//#############################################################################

void BCP_lp_test_feasibility(BCP_lp_prob& p, const BCP_lp_result& lpres)
{
  BCP_solution* sol = p.user_has_lp_result_processing ? p.sol :
    p.user->test_feasibility(lpres, p.node->vars, p.node->cuts);
  p.sol = NULL;

  if (sol) {
    // send the feas sol to the tree manager (that routine also displays)
    p.user->send_feasible_solution(sol);
    delete sol;
  }
}

//#############################################################################

double BCP_lp_compute_lower_bound(BCP_lp_prob& p, const BCP_lp_result& lpres)
{
  return p.user_has_lp_result_processing ? p.new_true_lower_bound :
    p.user->compute_lower_bound(p.node->true_lower_bound,
				lpres, p.node->vars, p.node->cuts);
}

//#############################################################################

void BCP_lp_clean_up_node(BCP_lp_prob& p)
{
  p.node->clean();
  BCP_vec<BCP_var*>& vars = p.node->vars;
  purge_ptr_vector(vars, vars.entry(p.core->varnum()), vars.end());
  BCP_vec<BCP_cut*>& cuts = p.node->cuts;
  purge_ptr_vector(cuts, cuts.entry(p.core->cutnum()), cuts.end());
  p.parent->clean();
  // Also, the local pools might contain only locally valid
  // cuts/vars, so get rid of them
  purge_ptr_vector(*p.local_cut_pool);
  purge_ptr_vector(*p.local_var_pool);
}

//#############################################################################

BCP_message_tag BCP_lp_pack_for_cg(BCP_lp_prob& p)
{
  BCP_buffer& buf = p.msg_buf;
  buf.clear();
  buf.pack(p.node->level).pack(p.node->index).pack(p.node->iteration_count);

  buf.set_msgtag(BCP_Msg_ForCG_User);

  p.user->pack_primal_solution(buf, *p.lp_result,
			       p.node->vars, p.node->cuts);

  return buf.msgtag();
}

//#############################################################################

BCP_message_tag BCP_lp_pack_for_vg(BCP_lp_prob& p)
{
  BCP_buffer& buf = p.msg_buf;
  buf.clear();
  buf.pack(p.node->level).pack(p.node->index).pack(p.node->iteration_count);

  buf.set_msgtag(BCP_Msg_ForVG_User);

  p.user->pack_dual_solution(buf, *p.lp_result,
			     p.node->vars, p.node->cuts);

  return buf.msgtag();
}

//#############################################################################

void
BCP_lp_prepare_for_new_node(BCP_lp_prob& p)
{
  int i;

  BCP_var_set& vars = p.node->vars;
  BCP_cut_set& cuts = p.node->cuts;

  BCP_vec<int> vcp;
  BCP_vec<double> vbd;
  BCP_vec<int> ccp;
  BCP_vec<double> cbd;

  const int varnum = vars.size();
  BCP_vec<BCP_obj_status> vstat;
  vstat.reserve(varnum);
  for (i = 0; i < varnum; ++i) {
    vstat.unchecked_push_back(vars[i]->status());
  }

  const int cutnum = cuts.size();
  BCP_vec<BCP_obj_status> cstat;
  cstat.reserve(cutnum);
  for (i = 0; i < cutnum; ++i) {
    cstat.unchecked_push_back(cuts[i]->status());
  }

  p.user->initialize_new_search_tree_node(vars, cuts, vstat, cstat,
					  vcp, vbd, ccp, cbd);

  if (2 * vcp.size() != vbd.size()) {
    throw BCP_fatal_error("new node init returned uneven var vectors\n");
  }
  if (2 * ccp.size() != cbd.size()) {
    throw BCP_fatal_error("new node init returned uneven cut vectors\n");
  }

  const double petol = p.lp_result->primalTolerance();

  OsiSolverInterface& lp = *p.lp_solver;

  const int var_change_num = vcp.size();
  if (var_change_num > 0) {
    BCP_vec<double>::const_iterator newbd = vbd.begin();
    // check that the new bounds are actually tighter than the old ones.
    // throw an exception if not.
    for (i = 0; i < var_change_num; ++i) {
      const double new_lb = *newbd;
      ++newbd;
      const double new_ub = *newbd;
      ++newbd;
      const int pos = vcp[i];
      if (vars[pos]->lb() > new_lb+petol || vars[pos]->ub() < new_ub-petol)
	throw BCP_fatal_error("new node init enlarged var feas region!\n");
    }
    lp.setColSetBounds(vcp.begin(), vcp.end(), vbd.begin());
    vars.set_lb_ub(vcp, vbd.begin());
  }

  const int cut_change_num = ccp.size();
  if (cut_change_num > 0) {
    BCP_vec<double>::const_iterator newbd = cbd.begin();
    // check that the new bounds are actually tighter than the old ones.
    // throw an exception if not.
    for (i = 0; i < cut_change_num; ++i) {
      const double new_lb = *newbd;
      ++newbd;
      const double new_ub = *newbd;
      ++newbd;
      const int pos = ccp[i];
      if (cuts[pos]->lb() > new_lb+petol || cuts[pos]->ub() < new_ub-petol)
	throw BCP_fatal_error("new node init enlarged cut feas region!\n");
    }
    lp.setRowSetBounds(ccp.begin(), ccp.end(), cbd.begin());
    cuts.set_lb_ub(ccp, cbd.begin());
  }

  if (lp.numberObjects() == 0) {
    if (!p.intAndSosObjects.empty()) {
      lp.addObjects(p.intAndSosObjects.size(), &p.intAndSosObjects[0]);
      const int numObj = lp.numberObjects();
      OsiObject** obj = lp.objects();
      for (int i = 0; i < numObj; ++i) {
	OsiSimpleInteger* io = dynamic_cast<OsiSimpleInteger*>(obj[i]);
	if (io) {
	  io->resetBounds(&lp);
	} else {
	  // The rest is OsiSOS where we don't need to do anything
	  break;
	}
      }
    } else {
      for (int i = 0; i < varnum; ++i) {
	if (vars[i]->var_type() != BCP_ContinuousVar) {
	  lp.setInteger(i);
	}
      }
      lp.findIntegersAndSOS(false);
    }
  }

  // If there is a root warmstart info then set it
  if (p.param(BCP_lp_par::WarmstartInfo) == BCP_WarmstartRoot &&
      p.warmstartRoot != NULL) {
    lp.setWarmStart(p.warmstartRoot);
  }
}

//#############################################################################

void
BCP_lp_add_cols_to_lp(const BCP_vec<BCP_col*>& cols, OsiSolverInterface* lp)
{
  const int colnum = cols.size();
  double * clb = new double[colnum];
  double * cub = new double[colnum];
  double * obj = new double[colnum];
  const CoinPackedVectorBase** vectors =
    new const CoinPackedVectorBase*[colnum];
  for (int i = 0; i < colnum; ++i) {
    const BCP_col * col = cols[i];
    vectors[i] = col;
    clb[i] = col->LowerBound();
    cub[i] = col->UpperBound();
    obj[i] = col->Objective();
  }
  lp->addCols(colnum, vectors, clb, cub, obj);
  delete[] vectors;
  delete[] obj;
  delete[] cub;
  delete[] clb;
}

//#############################################################################

void
BCP_lp_add_rows_to_lp(const BCP_vec<BCP_row*>& rows, OsiSolverInterface* lp)
{
  const int rownum = rows.size();
  double * rlb = new double[rownum];
  double * rub = new double[rownum];
  const CoinPackedVectorBase** vectors =
    new const CoinPackedVectorBase*[rownum];
  for (int i = 0; i < rownum; ++i) {
    const BCP_row * row = rows[i];
    vectors[i] = row;
    rlb[i] = row->LowerBound();
    rub[i] = row->UpperBound();
  }
  lp->addRows(rownum, vectors, rlb, rub);
  delete[] vectors;
  delete[] rub;
  delete[] rlb;
}
