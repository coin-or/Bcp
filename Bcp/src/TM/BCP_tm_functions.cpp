// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include "BCP_os.hpp"
#include "BCP_error.hpp"
#include "BCP_node_change.hpp"
#include "BCP_enum_tm.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_functions.hpp"

static inline BCP_node_start_result BCP_tm_start_one_node(BCP_tm_prob& p);

//#############################################################################

BCP_vec< std::pair<int,int> >::iterator
BCP_tm_identify_process(BCP_vec< std::pair<int,int> >& proclist, int proc)
{
    BCP_vec< std::pair<int,int> >::iterator proci = proclist.begin();
    BCP_vec< std::pair<int,int> >::iterator lastproci = proclist.end();
    while (proci != lastproci) {
	if (proci->first == proc)
	    break;
	++proci;
    }
    return proci;
}

//#############################################################################

bool
BCP_tm_assign_processes(BCP_tm_prob& p, BCP_tm_node* node)
{
    int lp = -1;
    int cg = -1;
    int vg = -1;
    int cp = -1;
    int vp = -1;
    bool so_far_so_good = true;

    if (so_far_so_good) {
	lp = p.slaves.lp->get_free_proc();
	if (lp == -1)
	    return false;
	if (! p.msg_env->alive(lp)) {
	    BCP_tm_remove_lp(p, lp);
	    so_far_so_good = false;
	}
    }

#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
    if (so_far_so_good && p.slaves.cg) {
	cg = p.slaves.cg->get_free_proc();
	if (cg == -1)
	    return false;
	if (! p.msg_env->alive(cg)) {
	    BCP_tm_remove_cg(p, p.slaves.cg->index_of_proc(cg));
	    so_far_so_good = false;
	}
    }

    if (so_far_so_good && p.slaves.vg) {
	vg = p.slaves.vg->get_free_proc();
	if (vg == -1)
	    return false;
	if (! p.msg_env->alive(vg)) {
	    BCP_tm_remove_vg(p, p.slaves.vg->index_of_proc(vg));
	    so_far_so_good = false;
	}
    }
#endif

    if (so_far_so_good && p.slaves.cp) {
	while (true) {
	    cp = p.slaves.cp->get_free_proc();
	    if (cp == -1)
		break;
	    if (p.msg_env->alive(cp))
		break;
	    // *FIXME*
	    throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
	}
	if (cp == -1) {
	    // if there is no free CP, just keep the old one
	    if (node->cp != -1 && ! p.msg_env->alive(node->cp)) {
		// *FIXME*
		throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
	    }
	} else {
	    if (node->cp != -1 && ! p.msg_env->alive(node->cp)) {
		// *FIXME*
		throw BCP_fatal_error("TM: A CP has died. Aborting...\n");
	    }
	} 
    }

    if (so_far_so_good && p.slaves.vp) {
	while (true) {
	    vp = p.slaves.vp->get_free_proc();
	    if (vp == -1)
		break;
	    if (p.msg_env->alive(vp))
		break;
	    // *FIXME*
	    throw BCP_fatal_error("TM: A VP has died. Aborting...\n");
	}
	if (vp == -1) {
	    // if there is no free VP, just keep the old one
	    if (node->vp != -1 && ! p.msg_env->alive(node->vp)) {
		// *FIXME*
		throw BCP_fatal_error("TM: A VP has died. Aborting...\n");
	    }
	} else {
	    if (node->vp != -1 && ! p.msg_env->alive(node->vp)) {
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
    if (cp != -1) {
	// *LATER* : copy the old CP over to the free one and let node have
	// this new CP.
	node->cp = cp;
    }
    if (vp != -1) {
	// *LATER* : copy the old VP over to the free one and let node have
	// this new VP.
	node->vp = vp;
    }

    return true;
}

//#############################################################################

static void BCP_tm_free_nodes(BCP_tm_prob& p)
{
    for (int i = p.nodes_to_free.size() - 1; i >= 0; --i) {
	BCP_tm_modify_pool_counters(p, p.nodes_to_free[i]);
	BCP_tm_remove_explored(p, p.nodes_to_free[i]);
    }
    p.nodes_to_free.clear();
}

//#############################################################################

static inline BCP_node_start_result
BCP_tm_start_one_node(BCP_tm_prob& p)
{
    BCP_tm_node* next_node;

    while (true){
	if (p.candidate_list.empty()) {
	    BCP_tm_free_nodes(p);
	    return BCP_NodeStart_NoNode;
	}
	next_node = dynamic_cast<BCP_tm_node*>(p.candidate_list.top());
	p.candidate_list.pop();

	// if no UB yet or lb is lower than UB then go ahead
	if (! p.has_ub())
	    break;

	bool process_this = true;

	if (next_node->getTrueLB() > p.ub() - p.granularity())
	    process_this = false;
	if (next_node->getTrueLB() >
	    p.ub() - p.param(BCP_tm_par::TerminationGap_Absolute))
	    process_this = false;
	if (next_node->getTrueLB() >
	    p.ub() * (1 - p.param(BCP_tm_par::TerminationGap_Relative)))
	    process_this = false;

	if (process_this)
	    break;

	// ok, so we do have an UB and true lb is "higher" than the UB.
	if (p.current_phase_colgen == BCP_DoNotGenerateColumns_Fathom) {
	    // nothing is left to price or in this phase we just fathom the
	    // over-the-bound nodes. in either case this node can be pruned
	    // right here.
	    next_node->status = BCP_PrunedNode_OverUB;
	    if (p.param(BCP_tm_par::TmVerb_PrunedNodeInfo))
		printf("TM: Pruning NODE %i LEVEL %i instead of sending it.\n",
		       next_node->index(), next_node->getDepth());
	    p.nodes_to_free.push_back(next_node);
	    BCP_print_memusage(p);
	    continue;
	}
	if (p.current_phase_colgen == BCP_DoNotGenerateColumns_Send) {
	    // the node would be sent back from the LP right away. save the
	    // trouble and don't even send it out
	    p.next_phase_nodes.push_back(next_node);
	    next_node->status = BCP_NextPhaseNode_OverUB;
	    if (p.param(BCP_tm_par::TmVerb_PrunedNodeInfo))
		printf("\
TM: Moving NODE %i LEVEL %i into the next phase list \n\
    instead of sending it.\n",
		       next_node->index(), next_node->getDepth());
	    continue;
	} else { // must be BCP_GenerateColumns
	    // all right, we want to send it out anyway for pricing
	    break;
	}
    }

    // assign processes to the node and send it off
    if (! BCP_tm_assign_processes(p, next_node)) {
	// couldn't find free processes
	p.candidate_list.push(next_node, false);
	BCP_tm_free_nodes(p);
	return BCP_NodeStart_Error;
    }

    p.active_nodes[next_node->lp] = next_node;
    next_node->status = BCP_ActiveNode;
    if (p.param(BCP_tm_par::MessagePassingIsSerial)) {
	BCP_tm_free_nodes(p);
    }
    BCP_tm_node_to_send* node_to_send =
	new BCP_tm_node_to_send(p, next_node, BCP_Msg_ActiveNodeData);
    if (node_to_send->send()) {
	delete node_to_send;
    }

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
    BCP_tm_free_nodes(p);
    return BCP_NodeStart_OK;
}

//#############################################################################

void BCP_tm_list_candidates(BCP_tm_prob& p)
{
    /* FIXME: must walk through the siblings... */
#if 0
    CoinSearchTreeBase& candidates = *p.candidate_list.getTree();
    const int n = candidates.size();
    const std::vector<CoinTreeNode*>& nodes = candidates.getNodes();
    for (int i = 0; i < n; ++i) {
	printf("%5i", dynamic_cast<BCP_tm_node*>(nodes[i])->index());
    }
    printf("\n");
#endif
}

//#############################################################################

void BCP_check_parameters(BCP_tm_prob& p)
{
    p.ub(p.param(BCP_tm_par::UpperBound));

    if (p.par.entry(BCP_tm_par::VerbosityShutUp)) {
	int i;
	BCP_parameter_set<BCP_tm_par>& tmpar = p.par;
	BCP_parameter_set<BCP_lp_par>& lppar = p.slave_pars.lp;
	BCP_parameter_set<BCP_cg_par>& cgpar = p.slave_pars.cg;
	BCP_parameter_set<BCP_vg_par>& vgpar = p.slave_pars.vg;
	char treestat = tmpar.entry(BCP_tm_par::TmVerb_FinalStatistics);
	char bestsol  = tmpar.entry(BCP_tm_par::TmVerb_BestFeasibleSolution);
	for (i = BCP_tm_par::TmVerb_First+1; i < BCP_tm_par::TmVerb_Last; ++i){
	    if (tmpar.entry(static_cast<BCP_tm_par::chr_params>(i)) == 2) {
		tmpar.set_entry(static_cast<BCP_tm_par::chr_params>(i), true);
	    } else {
		tmpar.set_entry(static_cast<BCP_tm_par::chr_params>(i), false);
	    }
	}
	for (i = BCP_lp_par::LpVerb_First+1; i < BCP_lp_par::LpVerb_Last; ++i){
	    if (lppar.entry(static_cast<BCP_lp_par::chr_params>(i)) == 2) {
		lppar.set_entry(static_cast<BCP_lp_par::chr_params>(i), true);
	    } else {
		lppar.set_entry(static_cast<BCP_lp_par::chr_params>(i), false);
	    }
	}
	for (i = BCP_cg_par::CgVerb_First+1; i < BCP_cg_par::CgVerb_Last; ++i){
	    if (cgpar.entry(static_cast<BCP_cg_par::chr_params>(i)) == 2) {
		cgpar.set_entry(static_cast<BCP_cg_par::chr_params>(i), true);
	    } else {
		cgpar.set_entry(static_cast<BCP_cg_par::chr_params>(i), false);
	    }
	}
	/*
	  for (i = BCP_vg_par::VgVerb_First+1; i < BCP_vg_par::VgVerb_Last; ++i){
	  vgpar.set_entry(static_cast<BCP_vg_par::chr_params>(i), false);
	  }
	*/
	tmpar.set_entry(BCP_tm_par::TmVerb_FinalStatistics, treestat);
	tmpar.set_entry(BCP_tm_par::TmVerb_BestFeasibleSolution, bestsol);
	if (tmpar.entry(BCP_tm_par::ReportWhenDefaultIsExecuted) == 2) {
	    tmpar.set_entry(BCP_tm_par::ReportWhenDefaultIsExecuted, true);
	} else {
	    tmpar.set_entry(BCP_tm_par::ReportWhenDefaultIsExecuted, false);
	}
	if (lppar.entry(BCP_lp_par::ReportWhenDefaultIsExecuted) == 2) {
	    lppar.set_entry(BCP_lp_par::ReportWhenDefaultIsExecuted, true);
	} else {
	    lppar.set_entry(BCP_lp_par::ReportWhenDefaultIsExecuted, false);
	}
	if (cgpar.entry(BCP_cg_par::ReportWhenDefaultIsExecuted) == 2) {
	    cgpar.set_entry(BCP_cg_par::ReportWhenDefaultIsExecuted, true);
	} else {
	    cgpar.set_entry(BCP_cg_par::ReportWhenDefaultIsExecuted, false);
	}
	if (vgpar.entry(BCP_vg_par::ReportWhenDefaultIsExecuted) == 2) {
	    vgpar.set_entry(BCP_vg_par::ReportWhenDefaultIsExecuted, true);
	} else {
	    vgpar.set_entry(BCP_vg_par::ReportWhenDefaultIsExecuted, false);
	}
    }
    if (p.param(BCP_tm_par::MaxHeapSize) == 0) {
        long fm = BCP_free_mem();
	fm = fm == -1 ? 192 * (1<<20) /* 192M */ : fm;
	p.par.set_entry(BCP_tm_par::MaxHeapSize, fm);
	p.slave_pars.ts.set_entry(BCP_ts_par::MaxHeapSize, fm);
    }
}

//#############################################################################
// A little bit of sanity check

void BCP_sanity_checks(BCP_tm_prob& p)
{
}

