// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <functional>

#include "BCP_message.hpp"
#include "BCP_temporary.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_functions.hpp"

//#############################################################################

void BCP_lp_check_ub(BCP_lp_prob& p)
{
   while (true){
      // call receive with 0 timeout => nonblocking
      p.msg_buf.clear();
      p.msg_env->receive(BCP_AnyProcess, BCP_Msg_UpperBound, p.msg_buf, 0);
      if (p.msg_buf.msgtag() == BCP_Msg_NoMessage)
	 break;
      BCP_lp_process_ub_message(p, p.msg_buf);
   }
}

//#############################################################################

void BCP_lp_process_message(BCP_lp_prob& p, BCP_buffer& buf)
{
   BCP_cut* cut;
   BCP_var* var;
   const BCP_proc_id * cpid = p.node->cp;
   const BCP_proc_id * vpid = p.node->vp;
   bool dont_send_to_pool;

   int node_index;
   int node_itcnt;

   switch (buf.msgtag()){
    case BCP_Msg_InitialUserInfo:
      throw BCP_fatal_error("LP: BCP_Msg_InitialUserInfo arrived in BCP_lp_process_message().\n");

    case BCP_Msg_CutIndexSet:
      buf.unpack(p.next_cut_index).unpack(p.last_cut_index);
      break;

    case BCP_Msg_VarIndexSet:
      buf.unpack(p.next_var_index).unpack(p.last_var_index);
      break;

    case BCP_Msg_CutDescription:
      cut = p.unpack_cut();
      if (p.param(BCP_lp_par::CompareNewCutsToOldOnes)){
	 // check if we already have this cut in the local cut pool
	 BCP_lp_cut_pool::iterator oldcut = p.local_cut_pool->begin();
	 BCP_lp_cut_pool::iterator lastoldcut = p.local_cut_pool->end();
	 while (oldcut != lastoldcut){
	    switch (p.user->compare_cuts((*oldcut)->cut(), cut)){
	     case BCP_FirstObjIsBetter:
	     case BCP_ObjsAreSame:
	       delete cut;   cut = 0;
	       buf.clear();
	       return;
	     case BCP_SecondObjIsBetter:
	     case BCP_DifferentObjs:
	       ++oldcut;
	       break;
	    }
	 }
      }

      dont_send_to_pool =
	 (! cpid || (cpid && cpid->is_same_process(buf.sender())));
      if (p.no_more_cuts_cnt >= 0){ // we are waiting for cuts
	 BCP_lp_cut_pool& cp = *p.local_cut_pool;
	 const int old_cp_size = cp.size();
	 BCP_vec<BCP_row*> rows;
	 rows.reserve(1);
	 BCP_vec<BCP_cut*> cuts(1, cut);
	 p.user->cuts_to_rows(p.node->vars, cuts, rows, *p.lp_result,
			      cpid && cpid->is_same_process(buf.sender()) ?
			      BCP_Object_FromPool : BCP_Object_FromGenerator,
			      true);
	 const int cutnum = cuts.size();
	 for (int i = 0; i < cutnum; ++i) {
	    cut = cuts[i];
	    cut->set_bcpind(-BCP_lp_next_cut_index(p));
	    cut->dont_send_to_pool(dont_send_to_pool);
	    cp.push_back(new BCP_lp_waiting_row(cut, rows[i]));
	 }
	 // compute the violation(s)
	 cp.compute_violations(*p.lp_result, cp.entry(old_cp_size), cp.end());
      }else{ // the cut arrived while we are waiting for new LP
	 p.local_cut_pool->push_back(new BCP_lp_waiting_row(cut));
      }
      break;

    case BCP_Msg_NoMoreCuts:
      // no more cuts can be generated for the current LP solution and hence
      // calculation can resume
      double cutgen_time;
      buf.unpack(node_index).unpack(node_itcnt).unpack(cutgen_time);
      p.stat.time_cut_generation += cutgen_time;
      if (p.no_more_cuts_cnt >= 0 &&
	  p.node->index == node_index && p.node->iteration_count == node_itcnt)
	 p.no_more_cuts_cnt--;
      break;

    case BCP_Msg_VarDescription:
      var = p.unpack_var();
      if (p.param(BCP_lp_par::CompareNewVarsToOldOnes)){
	 // check if we already have this var in the local var pool
	 BCP_lp_var_pool::iterator oldvar = p.local_var_pool->begin();
	 BCP_lp_var_pool::iterator lastoldvar = p.local_var_pool->end();
	 while (oldvar != lastoldvar){
	    switch (p.user->compare_vars((*oldvar)->var(), var)){
	     case BCP_FirstObjIsBetter:
	     case BCP_ObjsAreSame:
	       delete var;   var = 0;
	       buf.clear();
	       return;
	     case BCP_SecondObjIsBetter:
	     case BCP_DifferentObjs:
	       ++oldvar;
	       break;
	    }
	 }
      }

      dont_send_to_pool =
	 (! vpid || (vpid && vpid->is_same_process(buf.sender())));
      if (p.no_more_vars_cnt >= 0){ // we are waiting for vars
	 BCP_lp_var_pool& vp = *p.local_var_pool;
	 const int old_vp_size = vp.size();
	 BCP_vec<BCP_col*> cols;
	 cols.reserve(1);
	 BCP_vec<BCP_var*> vars(1, var);
	 p.user->vars_to_cols(p.node->cuts, vars, cols, *p.lp_result,
			      vpid && vpid->is_same_process(buf.sender()) ?
			      BCP_Object_FromPool : BCP_Object_FromGenerator,
			      true);
	 const int varnum = vars.size();
	 for (int i = 0; i < varnum; ++i) {
	    var = vars[i];
	    var->set_bcpind(-BCP_lp_next_var_index(p));
	    var->dont_send_to_pool(dont_send_to_pool);
	    vp.push_back(new BCP_lp_waiting_col(var, cols[i]));
	 }
	 // compute the reduced_cost(s)
	 vp.compute_red_costs(*p.lp_result, vp.entry(old_vp_size), vp.end());
      }else{ // the var arrived while we are waiting for new LP
	 p.local_var_pool->push_back(new BCP_lp_waiting_col(var));
      }
      break;

    case BCP_Msg_NoMoreVars:
      // no more vars can be generated for the current LP solution and hence
      // calculation can resume
      double vargen_time;
      buf.unpack(node_index).unpack(node_itcnt).unpack(vargen_time);
      p.stat.time_var_generation += vargen_time;
      if (p.no_more_vars_cnt >= 0 &&
	  p.node->index == node_index && p.node->iteration_count == node_itcnt)
	 p.no_more_vars_cnt--;
      break;

    case BCP_Msg_UpperBound:
      BCP_lp_process_ub_message(p, buf);
      break;

#if 0
    case BCP_Msg_RootToPrice:
      BCP_lp_unpack_active_node(p, buf);
      // load the lp formulation into the lp solver
      BCP_lp_create_lp(p);
      BCP_lp_repricing(p);
      p.lp_solver->unload_lp();
      break;
#endif

    case BCP_Msg_ActiveNodeData:
      BCP_lp_unpack_active_node(p, buf);
      // load the lp formulation into the lp solver
      p.lp_solver = p.master_lp->clone();
      if (! p.param(BCP_lp_par::SolveLpToOptimality))
	 p.lp_solver->setDblParam(OsiDualObjectiveLimit,
				  p.ub() - p.granularity());
      BCP_lp_create_lp(p);
      BCP_lp_main_loop(p);
      delete p.lp_solver;
      p.lp_solver = NULL;
      break;

    case BCP_Msg_DivingInfo:
      BCP_lp_unpack_diving_info(p, buf);
      break;

    case BCP_Msg_NextPhaseStarts:
      buf.clear();
      // First send back timing data for the previous phase
      p.stat.pack(buf);
      p.msg_env->send(p.tree_manager, BCP_Msg_LpStatistics, buf);
      p.phase++;
      break;

    case BCP_Msg_FinishedBCP:
      // No need to clean up anything since the destructor of 'p' will do that.
      // However, send back the statistics.
      p.stat.pack(buf);
      p.msg_env->send(p.tree_manager, BCP_Msg_LpStatistics, buf);
      return;

//     case BCP_Msg_UserMessageToLp:
//       p.user->unpack_user_message(p, buf);
//       buf.clear();
//       break;

    default:
      printf("Unknown message type arrived to LP: %i\n", buf.msgtag());
      break;
   }

   buf.clear();
}

//#############################################################################

BCP_IndexType
BCP_lp_next_var_index(BCP_lp_prob& p)
{
   if (p.next_var_index == p.last_var_index) {
      BCP_buffer& buf = p.msg_buf;
      // get new set of indices
      buf.clear();
      p.msg_env->send(p.tree_manager, BCP_Msg_RequestVarIndexSet);
      // In a single process environment the new index range has already
      // been received (and unpacked), thus we've got to receive it only if
      // the range still has length 0.
      if (p.next_var_index == p.last_var_index) {
	 p.msg_env->receive(p.tree_manager, BCP_Msg_VarIndexSet, buf, -1);
	 BCP_lp_process_message(p, buf);
      }
   }
   const BCP_IndexType tmp = p.next_var_index++;
   return tmp;
}

//#############################################################################

BCP_IndexType
BCP_lp_next_cut_index(BCP_lp_prob& p)
{
   if (p.next_cut_index == p.last_cut_index) {
      BCP_buffer& buf = p.msg_buf;
      // get new set of indices
      buf.clear();
      p.msg_env->send(p.tree_manager, BCP_Msg_RequestCutIndexSet);
      // In a single process environment the new index range has already
      // been received (and unpacked), thus we've got to receive it only if
      // the range still has length 0.
      if (p.next_cut_index == p.last_cut_index) {
	 p.msg_env->receive(p.tree_manager, BCP_Msg_CutIndexSet, buf, -1);
	 BCP_lp_process_message(p, buf);
      }
   }
   const BCP_IndexType tmp = p.next_cut_index++;
   return tmp;
}

//#############################################################################

void BCP_lp_process_ub_message(BCP_lp_prob& p, BCP_buffer& buf)
{
   double new_ub;
   buf.unpack(new_ub);
   if (! p.param(BCP_lp_par::SolveLpToOptimality) && p.ub(new_ub))
     p.lp_solver->setDblParam(OsiDualObjectiveLimit, new_ub - p.granularity());
}

//#############################################################################

void BCP_lp_send_cuts_to_cp(BCP_lp_prob& p, const int eff_cnt_limit)
{
  if (! p.node->cp) // go back if no cut pool exists
    return;

  BCP_cut_set& cuts = p.node->cuts;
  BCP_cut_set::iterator cuti = cuts.entry(p.core->cutnum());
  const BCP_cut_set::const_iterator lastcuti = cuts.end();
  BCP_cut* cut;
  int cnt;

  // First count how many to send
  for (cnt = 0; cuti != lastcuti; ++cuti) {
    cut = *cuti;
    if (cut->effective_count() >= eff_cnt_limit &&
	! cut->dont_send_to_pool())
      ++cnt;
  }

  if (cnt > 0){
    BCP_buffer& buf = p.msg_buf;
    buf.clear();
    buf.pack(cnt);
    // whatever is sent to the CP must have been generated at this level
    buf.pack(p.node->level);
    // pack the cuts
    cuti = cuts.entry(p.core->cutnum());
    for ( ; cuti != lastcuti; ++cuti) {
      cut = *cuti;
      if (cut->effective_count() >= eff_cnt_limit &&
	  ! cut->dont_send_to_pool())
	p.pack_cut(BCP_ProcessType_CP, *cut);
      cut->dont_send_to_pool(true);
    }

    p.msg_env->send(p.node->cp, BCP_Msg_CutsToCutPool, buf);
    if (p.param(BCP_lp_par::LpVerb_CutsToCutPoolCount))
      printf("LP:   %i cuts sent to cutpool\n", cnt);
  }
}

//#############################################################################

void BCP_lp_unpack_diving_info(BCP_lp_prob& p, BCP_buffer& buf)
{
   buf.unpack(p.node->dive); // what's the new diving status?
   if (p.node->dive != BCP_DoNotDive){
      // do dive
      buf.unpack(p.node->index);
      // finally, a little cleaning for the new node
      p.node->level++;
      p.node->iteration_count = 0;
   } else {
      p.node->index = -1;
   }
}
