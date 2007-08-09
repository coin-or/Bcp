// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include "CoinTime.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_enum.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_node.hpp"

int BCP_lp_generate_vars(BCP_lp_prob& p,
			 bool cutset_changed, const bool from_repricing)
{
   double time0 = CoinCpuTime();

   BCP_lp_result& lpres = *p.lp_result;
   BCP_lp_var_pool& vp = *p.local_var_pool;
   int prev_size = vp.size();

   if (prev_size > 0 && ! vp.cols_are_valid()){
      // we must regenerate the cols from the variables
      // first delete the old cols then expand the vars again

      // these will hold vars and cols expanded from the vars
      BCP_vec<BCP_var*> vars;
      BCP_vec<BCP_col*> cols;

      vars.reserve(prev_size);
      int i;
      for (i = 0; i < prev_size; ++i) {
	 vp[i]->delete_col();
	 vars.unchecked_push_back(vp[i]->var());
      }
      // now expand
      cols.reserve(prev_size);
      p.user->vars_to_cols(p.node->cuts, vars, cols,
			   lpres, BCP_Object_Leftover, false);
      for (i = 0; i < prev_size; ++i) {
	 vp[i]->set_col(cols[i]);
      }
      cols.clear();
   }
   vp.cols_are_valid(true);

   if (p.param(BCP_lp_par::LpVerb_ReportLocalVarPoolSize))
      printf("LP:   Number of leftover vars: %i\n", prev_size);

   // Generate vars within the LP process
   BCP_vec<BCP_var*> new_vars;
   BCP_vec<BCP_col*> new_cols;
   BCP_price_vars(p, false /* not from fathom */, new_vars, new_cols);
   if (new_vars.size() > 0) {
      const int new_size = new_vars.size();
      vp.reserve(vp.size() + new_size);
      for (int i = 0; i < new_size; ++i) {
	 new_vars[i]->set_bcpind(-BCP_lp_next_var_index(p));
	 vp.unchecked_push_back(new BCP_lp_waiting_col(new_vars[i],
						       new_cols[i]));
      }
      new_cols.clear();
      new_vars.clear();
      if (p.param(BCP_lp_par::LpVerb_ReportLocalVarPoolSize))
	 printf("LP:   Number of vars generated in the LP process: %i\n",
		new_size);
      prev_size = vp.size();
   }

   // Compute the reduced cost for everything in the local var pool and throw
   // out the ones with positive reduced cost
   if (prev_size > 0) {
      vp.compute_red_costs(lpres, vp.begin(), vp.end());
      //      char dumpname[200];
      //sprintf(dumpname, "reducedcosts-%i-%i",
      //      p.node->index, p.node->iteration_count);
      //FILE* dumpfile = fopen(dumpname, "w");
      //for (int i = 0; i < prev_size; ++i) {
      //	 fprintf(dumpfile, "%.6f\n", vp[i]->red_cost());
      //}
      //fclose(dumpfile);
      double detol = 0.0;
      p.lp_solver->getDblParam(OsiDualTolerance, detol);
      const int cnt = vp.remove_positives(detol);
      if (p.param(BCP_lp_par::LpVerb_ReportLocalVarPoolSize))
	 printf("LP:   Positive rc (hence removed): %i\n", cnt);
      prev_size = vp.size();
   }

   if (p.param(BCP_lp_par::MessagePassingIsSerial)) {
      // If the message passing environment is not really parallel (i.e., while
      // the VG/VP are working the LP stops and also the LP must immediately
      // process any vars sent back then this is the place to send the lp
      // solution to the VG/VP.
      // send the current solution to VG, and also to VP if we are either
      //  - at the beginning of a chain (but not in the root in the
      //    first phase)
      //  - or this is the var_pool_check_freq-th iteration.
      if (p.node->vg || p.node->vp) {
	 const BCP_message_tag msgtag = BCP_lp_pack_for_vg(p);
	 if (p.node->vg) {
	    ++p.no_more_vars_cnt;
	    p.msg_env->send(p.node->vg, msgtag, p.msg_buf);
	 }
	 if (p.node->vp) {
	    if (! (p.node->iteration_count %
		   p.param(BCP_lp_par::VarPoolCheckFrequency))
		|| cutset_changed) {
	       ++p.no_more_vars_cnt;
	       p.msg_env->send(p.node->vp, msgtag, p.msg_buf);
	    }
	 }
      }
   }

   if (p.no_more_vars_cnt > 0){
      // Receive vars if we have sent out the lp solution somewhere.
      // set the timeout (all the times are in microseconds).
      double first_var_time_out = cutset_changed ?
	 p.param(BCP_lp_par::FirstLP_FirstVarTimeout) :
	 p.param(BCP_lp_par::LaterLP_FirstVarTimeout);
      double all_vars_time_out = cutset_changed ?
	 p.param(BCP_lp_par::FirstLP_AllVarsTimeout) :
	 p.param(BCP_lp_par::LaterLP_AllVarsTimeout);
      double tout = vp.size() == 0 ? first_var_time_out : all_vars_time_out;
      double tin = CoinCpuTime();

      while(true){
	 p.msg_buf.clear();
	 p.msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage,
			    p.msg_buf, tout);
	 if (p.msg_buf.msgtag() == BCP_Msg_NoMessage){
	    // check that everyone is still alive
	    if (! p.msg_env->alive(p.get_parent() /*tree_manager*/))
	       throw BCP_fatal_error("LP:   The TM has died -- LP exiting\n");
	    if (p.node->cg && ! p.msg_env->alive(p.node->cg))
	       throw BCP_fatal_error("LP:   The CG has died -- LP exiting\n");
	    if (p.node->cp && ! p.msg_env->alive(p.node->cp))
	       throw BCP_fatal_error("LP:   The CP has died -- LP exiting\n");
	    if (p.node->vg && ! p.msg_env->alive(p.node->vg))
	       throw BCP_fatal_error("LP:   The VG has died -- LP exiting\n");
	    if (p.node->vp && ! p.msg_env->alive(p.node->vp))
	       throw BCP_fatal_error("LP:   The VP has died -- LP exiting\n");
	    // now the message queue is empty and received_message has
	    // returned, i.e., we have waited enough
	    if (p.param(BCP_lp_par::LpVerb_ReportVarGenTimeout))
	       printf("LP:   Receive vars timed out after %f secs\n",
		      (prev_size != static_cast<int>(vp.size())?
		       all_vars_time_out : first_var_time_out));
	    break;
	 }
	 p.process_message();
	 // break out if no more vars can come
	 if (p.no_more_vars_cnt == 0)
	    break;

	 // reset the timeout
	 tout = vp.size() == 0 ? first_var_time_out : all_vars_time_out;
	 if (tout >= 0){
	    // with this tout we'll read out the rest of the message queue
	    // even if var generation times out.
	    tout = std::max<double>(0.0, tout - (CoinCpuTime() - tin));
	 }
      }
   }
   // reset no_more_vars_cnt to 0
   p.no_more_vars_cnt = 0;

   if (p.param(BCP_lp_par::LpVerb_ReportLocalVarPoolSize)) {
      printf("LP:   Number of vars received from VG: %i\n",
	     static_cast<int>(vp.size() - prev_size));
      printf("LP:   Total number of vars in local pool: %i\n",
	     static_cast<int>(vp.size()));
   }

   if (vp.size() > 0) {
      const int oldsize = vp.size();
      double detol = 0.0;
      p.lp_solver->getDblParam(OsiDualTolerance, detol);
      const int cnt = vp.remove_positives(detol);
      if (cnt > 0) {
	 printf("\
LP: *WARNING*: There are vars with positive red cost in the local VP\n\
               at the end of var generation.\n\
               Discarding %i variables out of %i.\n", cnt, oldsize);
      }
   }

   p.stat.time_var_generation += CoinCpuTime() - time0;

   return vp.size();
}
