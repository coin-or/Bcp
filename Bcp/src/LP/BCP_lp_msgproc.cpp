// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <functional>

#include "BCP_message.hpp"
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

void
BCP_lp_prob::process_message()
{
   BCP_cut* cut;
   BCP_var* var;
   const int cpid = node->cp;
   const int vpid = node->vp;

   int node_index;
   int node_itcnt;

   switch (msg_buf.msgtag()){
    case BCP_Msg_InitialUserInfo:
      throw BCP_fatal_error("\
LP: BCP_Msg_InitialUserInfo arrived in BCP_lp_prob::process_message().\n");

    case BCP_Msg_CutIndexSet:
      msg_buf.unpack(next_cut_index).unpack(last_cut_index);
      break;

    case BCP_Msg_VarIndexSet:
      msg_buf.unpack(next_var_index).unpack(last_var_index);
      break;

    case BCP_Msg_CutDescription:
      cut = unpack_cut();
      if (param(BCP_lp_par::CompareNewCutsToOldOnes)){
	 // check if we already have this cut in the local cut pool
	 BCP_lp_cut_pool::iterator oldcut = local_cut_pool->begin();
	 BCP_lp_cut_pool::iterator lastoldcut = local_cut_pool->end();
	 while (oldcut != lastoldcut){
	    switch (user->compare_cuts((*oldcut)->cut(), cut)){
	     case BCP_FirstObjIsBetter:
	     case BCP_ObjsAreSame:
	       delete cut;   cut = 0;
	       msg_buf.clear();
	       return;
	     case BCP_SecondObjIsBetter:
	     case BCP_DifferentObjs:
	       ++oldcut;
	       break;
	    }
	 }
      }

      if (no_more_cuts_cnt >= 0){ // we are waiting for cuts
	 const bool from_pool = (cpid == msg_buf.sender());
	 BCP_lp_cut_pool& cp = *local_cut_pool;
	 const int old_cp_size = cp.size();
	 BCP_vec<BCP_row*> rows;
	 rows.reserve(1);
	 BCP_vec<BCP_cut*> cuts(1, cut);
	 user->cuts_to_rows(node->vars, cuts, rows, *lp_result,
			    cpid == msg_buf.sender() ?
			    BCP_Object_FromPool : BCP_Object_FromGenerator,
			    true);
	 const int cutnum = cuts.size();
	 for (int i = 0; i < cutnum; ++i) {
	    cut = cuts[i];
	    if (! from_pool) 
		cut->set_bcpind(-BCP_lp_next_cut_index(*this));
	    cut->dont_send_to_pool(cpid == -1 || from_pool);
	    cp.push_back(new BCP_lp_waiting_row(cut, rows[i]));
	 }
	 // compute the violation(s)
	 cp.compute_violations(*lp_result, cp.entry(old_cp_size), cp.end());
      }else{ // the cut arrived while we are waiting for new LP
	 local_cut_pool->push_back(new BCP_lp_waiting_row(cut));
      }
      break;

    case BCP_Msg_NoMoreCuts:
      // no more cuts can be generated for the current LP solution and hence
      // calculation can resume
      double cutgen_time;
      msg_buf.unpack(node_index).unpack(node_itcnt).unpack(cutgen_time);
      stat.time_cut_generation += cutgen_time;
      if (no_more_cuts_cnt >= 0 &&
	  node->index == node_index && node->iteration_count == node_itcnt)
	 no_more_cuts_cnt--;
      break;

    case BCP_Msg_VarDescription:
      var = unpack_var();
      if (param(BCP_lp_par::CompareNewVarsToOldOnes)){
	 // check if we already have this var in the local var pool
	 BCP_lp_var_pool::iterator oldvar = local_var_pool->begin();
	 BCP_lp_var_pool::iterator lastoldvar = local_var_pool->end();
	 while (oldvar != lastoldvar){
	    switch (user->compare_vars((*oldvar)->var(), var)){
	     case BCP_FirstObjIsBetter:
	     case BCP_ObjsAreSame:
	       delete var;   var = 0;
	       msg_buf.clear();
	       return;
	     case BCP_SecondObjIsBetter:
	     case BCP_DifferentObjs:
	       ++oldvar;
	       break;
	    }
	 }
      }

      if (no_more_vars_cnt >= 0){ // we are waiting for vars
	 const bool from_pool = (vpid == msg_buf.sender());
	 BCP_lp_var_pool& vp = *local_var_pool;
	 const int old_vp_size = vp.size();
	 BCP_vec<BCP_col*> cols;
	 cols.reserve(1);
	 BCP_vec<BCP_var*> vars(1, var);
	 user->vars_to_cols(node->cuts, vars, cols, *lp_result,
			    vpid == msg_buf.sender() ?
			    BCP_Object_FromPool : BCP_Object_FromGenerator,
			    true);
	 const int varnum = vars.size();
	 for (int i = 0; i < varnum; ++i) {
	    var = vars[i];
	    if (! from_pool) 
		var->set_bcpind(-BCP_lp_next_var_index(*this));
	    var->dont_send_to_pool(vpid == -1 || from_pool);
	    vp.push_back(new BCP_lp_waiting_col(var, cols[i]));
	 }
	 // compute the reduced_cost(s)
	 vp.compute_red_costs(*lp_result, vp.entry(old_vp_size), vp.end());
      }else{ // the var arrived while we are waiting for new LP
	 local_var_pool->push_back(new BCP_lp_waiting_col(var));
      }
      break;

    case BCP_Msg_NoMoreVars:
      // no more vars can be generated for the current LP solution and hence
      // calculation can resume
      double vargen_time;
      msg_buf.unpack(node_index).unpack(node_itcnt).unpack(vargen_time);
      stat.time_var_generation += vargen_time;
      if (no_more_vars_cnt >= 0 &&
	  node->index == node_index && node->iteration_count == node_itcnt)
	 no_more_vars_cnt--;
      break;

    case BCP_Msg_UpperBound:
      BCP_lp_process_ub_message(*this, msg_buf);
      break;

#if 0
    case BCP_Msg_RootToPrice:
      BCP_lp_unpack_active_node(*this, msg_buf);
      // load the lp formulation into the lp solver
      BCP_lp_create_lp(p);
      BCP_lp_repricing(p);
      lp_solver->unload_lp();
      break;
#endif

    case BCP_Msg_ActiveNodeData:
      BCP_lp_unpack_active_node(*this, msg_buf);
      // load the lp formulation into the lp solver
      lp_solver = master_lp->clone();
      if (node->colgen != BCP_GenerateColumns) {
	  // FIXME: If we had a flag in the node that indicates not to
	  // generate cols in it and in its descendants then the dual obj
	  // limit could still be set...
	  lp_solver->setDblParam(OsiDualObjectiveLimit, ub() - granularity());
      }
      BCP_lp_create_lp(*this);
      BCP_lp_main_loop(*this);
      delete lp_solver;
      lp_solver = NULL;
      break;

    case BCP_Msg_DivingInfo:
      BCP_lp_unpack_diving_info(*this, msg_buf);
      break;

    case BCP_Msg_NextPhaseStarts:
      msg_buf.clear();
      // First send back timing data for the previous phase
      stat.pack(msg_buf);
      msg_env->send(get_parent() /*ree_manager*/,
		    BCP_Msg_LpStatistics, msg_buf);
      phase++;
      break;

    case BCP_Msg_FinishedBCP:
      // No need to clean up anything since the destructor of 'p' will do that.
      // However, send back the statistics.
      stat.pack(msg_buf);
      msg_env->send(get_parent() /*ree_manager*/,
		    BCP_Msg_LpStatistics, msg_buf);
      return;

//     case BCP_Msg_UserMessageToLp:
//       user->unpack_user_message(*this, msg_buf);
//       msg_buf.clear();
//       break;

    default:
      printf("Unknown message type arrived to LP: %i\n", msg_buf.msgtag());
      break;
   }

   msg_buf.clear();
}

//#############################################################################

int
BCP_lp_next_var_index(BCP_lp_prob& p)
{
   if (p.next_var_index == p.last_var_index) {
      BCP_buffer& buf = p.msg_buf;
      // get new set of indices
      buf.clear();
      p.msg_env->send(p.get_parent() /*ree_manager*/,
		      BCP_Msg_RequestVarIndexSet);
      // In a single process environment the new index range has already
      // been received (and unpacked), thus we've got to receive it only if
      // the range still has length 0.
      if (p.next_var_index == p.last_var_index) {
	 p.msg_env->receive(p.get_parent() /*ree_manager*/,
			    BCP_Msg_VarIndexSet, buf, -1);
	 p.process_message();
      }
   }
   const int tmp = p.next_var_index++;
   return tmp;
}

//#############################################################################

int
BCP_lp_next_cut_index(BCP_lp_prob& p)
{
   if (p.next_cut_index == p.last_cut_index) {
      BCP_buffer& buf = p.msg_buf;
      // get new set of indices
      buf.clear();
      p.msg_env->send(p.get_parent() /*ree_manager*/,
		      BCP_Msg_RequestCutIndexSet);
      // In a single process environment the new index range has already
      // been received (and unpacked), thus we've got to receive it only if
      // the range still has length 0.
      if (p.next_cut_index == p.last_cut_index) {
	 p.msg_env->receive(p.get_parent() /*ree_manager*/,
			    BCP_Msg_CutIndexSet, buf, -1);
	 p.process_message();
      }
   }
   const int tmp = p.next_cut_index++;
   return tmp;
}

//#############################################################################

void BCP_lp_process_ub_message(BCP_lp_prob& p, BCP_buffer& buf)
{
    double new_ub;
    buf.unpack(new_ub);
    if (p.ub(new_ub) && p.lp_solver && p.node &&
	p.node->colgen != BCP_GenerateColumns) {
	// FIXME: If we had a flag in the node that indicates not to
	// generate cols in it and in its descendants then the dual obj
	// limit could still be set...
	p.lp_solver->setDblParam(OsiDualObjectiveLimit,new_ub-p.granularity());
    }
}

//#############################################################################

void BCP_lp_send_cuts_to_cp(BCP_lp_prob& p, const int eff_cnt_limit)
{
  if (p.node->cp != -1) // go back if no cut pool exists
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
	p.pack_cut(*cut);
      cut->dont_send_to_pool(true);
    }

    if (p.node->cp != -1) {
      p.msg_env->send(p.node->cp, BCP_Msg_CutsToCutPool, buf);
      if (p.param(BCP_lp_par::LpVerb_CutsToCutPoolCount))
	printf("LP:   %i cuts sent to cutpool\n", cnt);
    }
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
