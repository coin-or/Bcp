// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include "BCP_error.hpp"
#include "BCP_node_change.hpp"
#include "BCP_enum_tm.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_functions.hpp"

static inline BCP_node_start_result
BCP_tm_start_one_node(BCP_tm_prob& p);

//#############################################################################

BCP_vec< std::pair<BCP_proc_id*,int> >::iterator
BCP_tm_identify_process(BCP_vec< std::pair<BCP_proc_id*,int> >& proclist,
			BCP_proc_id* proc)
{
   BCP_vec< std::pair<BCP_proc_id*,int> >::iterator proci = proclist.begin();
   BCP_vec< std::pair<BCP_proc_id*,int> >::iterator lastproci = proclist.end();
   while (proci != lastproci) {
      if (proci->first->is_same_process(proc))
	 break;
      ++proci;
   }
   return proci;
}

//#############################################################################

bool
BCP_tm_assign_processes(BCP_tm_prob& p, BCP_tm_node* node)
{
   BCP_proc_id* lp = 0;
   BCP_proc_id* cg = 0;
   BCP_proc_id* vg = 0;
   BCP_proc_id* cp = 0;
   BCP_proc_id* vp = 0;
   bool so_far_so_good = true;

   if (so_far_so_good) {
      lp = p.slaves.lp->get_free_proc();
      if (lp == 0)
	 return false;
      if (! p.msg_env->alive(lp)) {
	 BCP_tm_remove_lp(p, p.slaves.lp->index_of_proc(lp));
	 so_far_so_good = false;
      }
   }

   if (so_far_so_good && p.slaves.cg) {
      cg = p.slaves.cg->get_free_proc();
      if (cg == 0)
	 return false;
      if (! p.msg_env->alive(cg)) {
	 BCP_tm_remove_cg(p, p.slaves.cg->index_of_proc(cg));
	 so_far_so_good = false;
      }
   }

   if (so_far_so_good && p.slaves.vg) {
      vg = p.slaves.vg->get_free_proc();
      if (vg == 0)
	 return false;
      if (! p.msg_env->alive(vg)) {
	 BCP_tm_remove_vg(p, p.slaves.vg->index_of_proc(vg));
	 so_far_so_good = false;
      }
   }

   if (so_far_so_good && p.slaves.cp) {
      while (true) {
	 cp = p.slaves.cp->get_free_proc();
	 if (cp == 0)
	    break;
	 if (p.msg_env->alive(cp))
	    break;
	 // *FIXME*
	 throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
      }
      if (cp == 0) {
	 // if there is no free CP, just keep the old one
	 if (node->cp && ! p.msg_env->alive(node->cp)) {
	    // *FIXME*
	    throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
	 }
      } else {
	 if (node->cp && ! p.msg_env->alive(node->cp)) {
	    // *FIXME*
	    throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
	 }
      } 
   }

   if (so_far_so_good && p.slaves.vp) {
      while (true) {
	 vp = p.slaves.vp->get_free_proc();
	 if (vp == 0)
	    break;
	 if (p.msg_env->alive(vp))
	    break;
	 // *FIXME*
	 throw BCP_fatal_error("TM: A VP has died. Aborting...\n");
      }
      if (vp == 0) {
	 // if there is no free VP, just keep the old one
	 if (node->vp && ! p.msg_env->alive(node->vp)) {
	    // *FIXME*
	    throw BCP_fatal_error("TM: A VP has died. Aborting...\n");
	 }
      } else {
	 if (node->vp && ! p.msg_env->alive(node->vp)) {
	    // *FIXME*
	    throw BCP_fatal_error("TM: A VP has died. Aborting...\n");
	 }
      } 
   }

   if (! so_far_so_good)
      return BCP_tm_assign_processes(p, node);

   node->lp = lp;
   node->cg = cg;
   node->vg = vg;
   if (cp) {
      // *LATER* : copy the old CP over to the free one and let node have
      // this new CP.
      node->cp = cp;
   }
   if (vp) {
      // *LATER* : copy the old VP over to the free one and let node have
      // this new VP.
      node->vp = vp;
   }

   return true;
}

//#############################################################################

static inline BCP_node_start_result
BCP_tm_start_one_node(BCP_tm_prob& p)
{
   BCP_tm_node* next_node;
   BCP_node_change* desc;

   while (true){
      if (p.candidates.empty())
	 return BCP_NodeStart_NoNode;
      next_node = p.candidates.top();
      p.candidates.pop();
      desc = next_node->_desc;

      // if no UB yet or lb is lower than UB then go ahead
      if (! p.has_ub())
	 break;

      bool process_this = true;

      if (next_node->true_lower_bound() > p.ub() - p.granularity())
	 process_this = false;
      if (next_node->true_lower_bound() >
	  p.ub() - p.param(BCP_tm_par::TerminationGap_Absolute))
	 process_this = false;
      if (next_node->true_lower_bound() >
	  p.ub() * (1 - p.param(BCP_tm_par::TerminationGap_Relative)))
	 process_this = false;

      if (process_this)
	 break;

      // ok, so we do have an UB and lb is "higher" than the UB.
      if (desc->indexed_pricing.get_status() == BCP_PriceNothing ||
	  p.current_phase_colgen == BCP_DoNotGenerateColumns_Fathom) {
	 // nothing is left to price or in this phase we just fathom the
	 // over-the-bound nodes. in either case this node can be pruned right
	 // here.
	 // *FIXME-NOW* : adjust the number of nodes assigned to the cp and vp
	 // of this node 
	 next_node->status = BCP_PrunedNode;
	 if (p.param(BCP_tm_par::TmVerb_PrunedNodeInfo))
	    printf("TM: Pruning NODE %i LEVEL %i instead of sending it.\n",
		   next_node->index(), next_node->level());
	 continue;
      }
      if (p.current_phase_colgen == BCP_DoNotGenerateColumns_Send) {
	 // the node would be sent back from the LP right away. save the
	 // trouble and don't even send it out
	 p.next_phase_nodes.push_back(next_node);
	 next_node->status = BCP_NextPhaseNode;
	 if (p.param(BCP_tm_par::TmVerb_PrunedNodeInfo))
	    printf("\
TM: Moving NODE %i LEVEL %i into the next phase list \n\
    instead of sending it.\n",
		   next_node->index(), next_node->level());
	 continue;
      } else { // must be BCP_GenerateColumns
	 // all right, we want to send it out anyway for pricing
	 break;
      }
   }

   // assign processes to the node and send it off
   if (! BCP_tm_assign_processes(p, next_node)) {
      // couldn't find free processes
      p.candidates.insert(next_node);
      return BCP_NodeStart_Error;
   }

   p.active_nodes[p.slaves.lp->index_of_proc(next_node->lp)] = next_node;
   next_node->status = BCP_ActiveNode;
   BCP_tm_send_node(p, next_node, BCP_Msg_ActiveNodeData);

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
   dump_procinfo(p, "start_one_node()");
#endif
#endif

   return BCP_NodeStart_OK;
}

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
void dump_procinfo(BCP_tm_prob& p, const char* str)
{
   printf("TM: ***** dump_procinfo from %s *********\n", str);
   printf("TM: ********** Active nodes *********\n");
   for (int i = 0; i < p.slaves.lp->size(); ++i) {
      if (p.active_nodes[i])
	 printf("TM:     %i, %i, %i\n",
		i, p.active_nodes[i]->index(), p.active_nodes[i]->lp);
      else
	 printf("TM:     %i, %i, %i\n",
		i, -1, -1);
	 
   }
   printf("TM: ********** All nodes *********\n");
   for (int i = 0; i < p.search_tree.size(); ++i) {
      printf("TM:     %i, %i\n", i, p.search_tree[i]->lp);
   }
}
#endif
#endif

//#############################################################################

BCP_node_start_result BCP_tm_start_new_nodes(BCP_tm_prob& p)
{
   while (p.slaves.lp->free_num()){
      switch (BCP_tm_start_one_node(p)){
       case BCP_NodeStart_NoNode:
	 return BCP_NodeStart_NoNode;
       case BCP_NodeStart_Error:
	 if (p.slaves.lp->free_num() != 0) {
	    throw BCP_fatal_error("\
TM: couldn't start new node but there's a free LP ?!\n");
	 }
	 break;
       case BCP_NodeStart_OK:
	 break;
      }
   }
   return BCP_NodeStart_OK;
}

//#############################################################################

void BCP_tm_list_candidates(BCP_tm_prob& p)
{
   BCP_vec<BCP_tm_node*> cands;
   BCP_tm_node* can;
   while (! p.candidates.empty()) {
      can = p.candidates.top();
      cands.push_back(can);
      p.candidates.pop();
      printf("%5i", can->index());
   }
   printf("\n");
   for (size_t i = 0; i < cands.size(); ++i)
      p.candidates.insert(cands[i]);
}

//#############################################################################

void BCP_check_parameters(BCP_tm_prob& p)
{
   p.ub(p.param(BCP_tm_par::UpperBound));

   if (p.param(BCP_tm_par::PriceInRootBeforePhase2)) {
      if (! p.param(BCP_tm_par::IndexedVariablesAreGenerated)) {
	 throw BCP_fatal_error("\
TM: PriceInRootBeforePhase2 is set, but IndexedVariablesAreGenerated is not.\n\
    This makes no sense.\n\
    Aborting.\n\n");
      }
	 
      printf("\
TM: PriceInRootBeforePhase2 is set.\n\
    Column generation will be disabled in the first phase.\n\n");
      if (p.param(BCP_tm_par::AlgorithmicVariablesAreGenerated)) {
	 printf("\
TM: PriceInRootBeforePhase2\n\
            *AND*\n\
    AlgorithmicVariablesAreGenerated are both set.\n\
TM: The final solution may not be optimal.\n\n");
      }
   }
}

//#############################################################################
// A little bit of sanity check

void BCP_sanity_checks(BCP_tm_prob& p)
{
   BCP_node_change* root_desc = p.search_tree.root()->_desc;

   if (p.core->varnum() == 0 &&
       root_desc->var_change.added_num() == 0) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("There are no vars neither in core or in root!\n");
   }

   if (p.core->cutnum() == 0 &&
       root_desc->cut_change.added_num() == 0) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("There are no cuts neither in core or in root!\n");
   }

   if (! (root_desc->indexed_pricing.get_status() & BCP_PriceIndexedVars) &&
       p.param(BCP_tm_par::IndexedVariablesAreGenerated)) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceIndexedVars is not set in the root description, yet indexed\n\
variables are supposed to be generated.\n");
   }

   if ((root_desc->indexed_pricing.get_status() & BCP_PriceIndexedVars) &&
       ! p.param(BCP_tm_par::IndexedVariablesAreGenerated)) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceIndexedVars is set in the root description, yet indexed variables\n\
are not supposed to be generated.\n");
   }

   if (p.param(BCP_tm_par::PriceInRootBeforePhase2) &&
       ( (root_desc->var_change.deleted_num() != 0) ||
	 (root_desc->var_change.changed_num() != 0) ||
	 (root_desc->var_change.added_num() != 0) )) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceInRootBeforePhase2 is set and there is something in the var_change\n\
part of the root description. Currently BCP allows only core variables if\n\
BCP_PriceInRootBeforePhase2 is set.\n");
   }
   if (p.param(BCP_tm_par::PriceInRootBeforePhase2) &&
       ( (root_desc->cut_change.deleted_num() != 0) ||
	 (root_desc->cut_change.changed_num() != 0) ||
	 (root_desc->cut_change.added_num() != 0) )) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceInRootBeforePhase2 is set and there is something in the cut_change\n\
part of the root description. Currently BCP allows only core cuts if\n\
BCP_PriceInRootBeforePhase2 is set.\n");
   }

   if (p.param(BCP_tm_par::PriceInRootBeforePhase2) &&
       p.core->varnum() == 0) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceInRootBeforePhase2 is set but there are no variables in the core!\n");
   }
   if (p.param(BCP_tm_par::PriceInRootBeforePhase2) &&
       p.core->cutnum() == 0) {
      // *FIXME* : kill all the processes
      throw BCP_fatal_error("\
BCP_PriceInRootBeforePhase2 is set but there are no cuts in the core!\n");
   }
}

