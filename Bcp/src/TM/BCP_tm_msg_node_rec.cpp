// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdlib>
#include <algorithm>

#include "CoinTime.hpp"

#include "BCP_os.hpp"
#include "BCP_USER.hpp"
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_branch.hpp"
#include "BCP_enum_branch.hpp"
#include "BCP_message.hpp"
#include "BCP_vector.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_tm_functions.hpp"

static int
BCP_tm_unpack_node_description(BCP_tm_prob& p, BCP_buffer& buf);
static void
BCP_tm_create_core_change(BCP_node_change* desc,
			  const int bvarnum, const int bcutnum,
			  const BCP_internal_brobj* brobj, const int childind);
static void
BCP_tm_create_var_change(BCP_node_change* desc,
			 const BCP_node_change* parentdesc, const int bvarnum,
			 BCP_internal_brobj* brobj, const int childind);
static void
BCP_tm_create_cut_change(BCP_node_change* desc,
			 const BCP_node_change* parentdesc, const int bcutnum,
			 BCP_internal_brobj* brobj, const int childind);
static void
BCP_tm_unpack_branching_info(BCP_tm_prob& p, BCP_buffer& buf,
			     BCP_tm_node* node);
static inline BCP_diving_status
BCP_tm_shall_we_dive(BCP_tm_prob& p, const double quality);

//#############################################################################

static void
BCP_tm_print_info_line(BCP_tm_prob& p, BCP_tm_node& node)
{
    const int freq = p.param(BCP_tm_par::TmVerb_SingleLineInfoFrequency);
    if (freq == 0)
	return;

    static int lines = 0;

    if ((lines % 40) == 0) {
	printf("\n");
	printf("BCP: ");
	printf(" Nodes  ");
	printf(" Proc'd ");
	printf("  BestUB   ");
	printf(" LowestQ   ");
	/*
	printf("AboveUB ");
	printf("BelowUB ");
	*/
	printf("\n");
    }
    const int processed = p.search_tree.processed();
    if ((processed % freq) == 0 || processed == 1) {
	++lines;
	printf("BCP: ");                                           // 5
	printf("%7i ", (int)p.search_tree.size());                 // 8
	printf("%7i ", processed);                                 // 8
	printf("%10g ", p.ub());                                   // 11
	if (p.candidate_list.empty()) {
	    printf("%10g ", 0.0);                                  // 11
	} else {
	    printf("%10g ", p.candidate_list.bestQuality());       // 11
	}
	/*
	int quality_above_UB;
	int quality_below_UB;
	p.candidates.compare_to_UB(quality_above_UB, quality_below_UB);
	printf("%7i ", quality_above_UB);                          // 8
	printf("%7i ", quality_below_UB);                          // 8
	*/
	printf("\n");
    }
}

//#############################################################################

void BCP_print_memusage(BCP_tm_prob& p)
{
#if 0
    static int cnt = 0;
    ++cnt;
    if ((cnt % 100) == 0) {
        long usedheap = BCP_used_heap();
	if (usedheap > 0) {
	    printf("    mallinfo used: %li\n", usedheap);
	}
    }
#endif
}

//#############################################################################

static int
BCP_tm_unpack_node_description(BCP_tm_prob& p, BCP_buffer& buf)
{
    // the first thing is the index of the node
    int index;
    buf.unpack(index);
    // get a pointer to this node
    BCP_tm_node* node = p.search_tree[index];
    p.search_tree.increase_processed();

    // XXX
    const int lp_id = node->lp;
    if (node != p.active_nodes[lp_id]) {
	throw BCP_fatal_error("\
BCP_tm_unpack_node_description: received node is different from processed.\n");
    }

    // get the quality and new lb for this node
    double q, tlb;
    buf.unpack(q).unpack(tlb);
    node->setQuality(q);
    node->setTrueLB(tlb);

    // wipe out any previous description of this node and create a new one if
    // the description is sent over
    if (node->_locally_stored) {
	node->_data._desc = NULL;
	node->_data._user = NULL;
    } else {
	BCP_buffer b;
	BCP_vec<int> indices(1, index);
	b.pack(indices);
	p.msg_env->send(node->_data_location, BCP_Msg_NodeListDelete, b);
	--BCP_tm_node::num_remote_nodes;
	++BCP_tm_node::num_local_nodes;
	node->_locally_stored = true;
    }

    bool desc_sent = false;
    buf.unpack(desc_sent);

    if (desc_sent) {
	BCP_node_change* desc = new BCP_node_change;

	// unpack core_change
	if (p.core->varnum() + p.core->cutnum() > 0)
	    desc->core_change.unpack(buf);

	int cnt;
	// get the variables. first unpack the new vars. those with negative
	// bcpind has not yet been sent to the TM from this LP process, but
	// they may have been sent here by another (if CP is used). those with
	// positive bcpind have already been sent to the TM, receiving such
	// var again is an error.
	buf.unpack(cnt);
	while (--cnt >= 0) {
	    p.unpack_var();
	}
	// Now unpack the change data.
	desc->var_change.unpack(buf);

	// same for cuts
	buf.unpack(cnt);
	while (--cnt >= 0) {
	    p.unpack_cut();
	}
	desc->cut_change.unpack(buf);
	    
	// warmstart info
	bool has_data;
	switch (p.param(BCP_tm_par::WarmstartInfo)) {
	case BCP_WarmstartNone:
	  break;
	case BCP_WarmstartRoot:
	  // nothing needs to be done even in this case as the WS has been
	  // sent in a separate message
	  break;
	case BCP_WarmstartParent:
	  buf.unpack(has_data);
	  if (has_data) {
	    const bool def = p.param(BCP_tm_par::ReportWhenDefaultIsExecuted);
	    desc->warmstart = p.packer->unpack_warmstart(buf, def);
	  }
	  break;
	}
	// user data
	buf.unpack(has_data);
	BCP_user_data* udata = has_data ? p.packer->unpack_user_data(buf) : 0;

	node->_data._desc = desc;
	node->_data._user = udata;
	node->_core_storage = desc->core_change.storage();
	node->_var_storage = desc->var_change.storage();
	node->_cut_storage = desc->cut_change.storage();
	node->_ws_storage =
	    desc->warmstart ? desc->warmstart->storage() : BCP_Storage_NoData;
    } else {
	node->_core_storage = BCP_Storage_NoData;
	node->_var_storage = BCP_Storage_NoData;
	node->_cut_storage = BCP_Storage_NoData;
	node->_ws_storage = BCP_Storage_NoData;
    }

    p.active_nodes.erase(lp_id);

    return index;
}

//#############################################################################

static inline BCP_diving_status
BCP_tm_shall_we_dive(BCP_tm_prob& p, const double quality)
{
    if (rand() < p.param(BCP_tm_par::UnconditionalDiveProbability) * RAND_MAX)
	return BCP_DoDive;

    const double ratio = p.has_ub() ?
	p.param(BCP_tm_par::QualityRatioToAllowDiving_HasUB) :
	p.param(BCP_tm_par::QualityRatioToAllowDiving_NoUB);

    if (ratio < 0)
	return BCP_DoNotDive;

    if (p.candidate_list.empty())
	return BCP_TestBeforeDive;

    const double topq = p.candidate_list.bestQuality();

    if (quality <= topq)
	return BCP_TestBeforeDive;

    if (topq > 0.05) {
	return (quality / topq < ratio) ? BCP_TestBeforeDive : BCP_DoNotDive;
    }
    if (topq >= -0.05) {
	return (quality < ratio) ? BCP_TestBeforeDive : BCP_DoNotDive;
    }
    /* must be topq < -0.05 */
    return (quality < 0 && topq / quality < ratio) ?
	    BCP_TestBeforeDive : BCP_DoNotDive;
}

//#############################################################################

static void
BCP_tm_create_core_change(BCP_node_change* desc,
			  const int bvarnum, const int bcutnum,
			  const BCP_internal_brobj* brobj, const int childind)
{
    if (bvarnum + bcutnum == 0) {
	desc->core_change._storage = BCP_Storage_NoData;
	return;
    }

    desc->core_change._storage = BCP_Storage_WrtParent;
    if (bvarnum > 0 && brobj->affected_varnum() > 0) {
	// First count how many of them are affecting core vars
	int affected_core = 0;
	BCP_vec<int>::const_iterator posi = brobj->var_positions().begin();
	BCP_vec<int>::const_iterator lastposi = brobj->var_positions().end();
	while (posi != lastposi) {
	    if (*posi < bvarnum)
		++affected_core;
	    ++posi;
	}
      
	if (affected_core > 0) {
	    BCP_vec<int>& var_pos = desc->core_change.var_pos;
	    BCP_vec<BCP_obj_change>& var_ch = desc->core_change.var_ch;
	    var_pos.reserve(affected_core);
	    var_ch.reserve(affected_core);
	    posi = brobj->var_positions().begin();
	    BCP_vec<double>::const_iterator boundsi =
		brobj->var_bounds_child(childind);
	    while (posi != lastposi) {
		if (*posi < bvarnum) {
		    var_pos.unchecked_push_back(*posi);
		    const double lb = *boundsi;
		    const double ub = *(boundsi+1);
		    var_ch.unchecked_push_back(BCP_obj_change(lb, ub,
							      BCP_ObjNotRemovable));
		}
		++posi;
		boundsi += 2;
	    }
	}
    }

    if (bcutnum > 0 && brobj->affected_cutnum() > 0) {
	// First count how many of them are affecting core cuts
	int affected_core = 0;
	BCP_vec<int>::const_iterator posi = brobj->cut_positions().begin();
	BCP_vec<int>::const_iterator lastposi = brobj->cut_positions().end();
	while (posi != lastposi) {
	    if (*posi < bcutnum)
		++affected_core;
	    ++posi;
	}

	if (affected_core > 0) {
	    BCP_vec<int>& cut_pos = desc->core_change.cut_pos;
	    BCP_vec<BCP_obj_change>& cut_ch = desc->core_change.cut_ch;
	    cut_pos.reserve(affected_core);
	    cut_ch.reserve(affected_core);
	    posi = brobj->cut_positions().begin();
	    BCP_vec<double>::const_iterator boundsi =
		brobj->cut_bounds_child(childind);
	    while (posi != lastposi) {
		if (*posi < bcutnum) {
		    cut_pos.unchecked_push_back(*posi);
		    const double lb = *boundsi;
		    const double ub = *(boundsi+1);
		    cut_ch.unchecked_push_back(BCP_obj_change(lb, ub,
							      BCP_ObjNotRemovable));
		}
		++posi;
		boundsi += 2;
	    }
	}
    }
}

//#############################################################################

static void
BCP_tm_create_var_change(BCP_node_change* desc,
			 const BCP_node_change* parentdesc, const int bvarnum,
			 BCP_internal_brobj* brobj, const int childind)
{
    // check first how many added var has changed in brobj
    int affected_added = 0;
    BCP_vec<int>::const_iterator posi = brobj->var_positions().begin();
    BCP_vec<int>::const_iterator lastposi = brobj->var_positions().end();
    while (posi != lastposi) {
	if (*posi >= bvarnum)
	    ++affected_added;
	++posi;
    }

    if (affected_added == 0) {
	if (parentdesc->var_change.storage() == BCP_Storage_Explicit &&
	    parentdesc->var_change.added_num() == 0) {
	    desc->var_change._storage = BCP_Storage_Explicit;
	} else {
	    desc->var_change._storage = BCP_Storage_WrtParent;
	}
	return;
    }

    desc->var_change._storage = BCP_Storage_WrtParent;
    BCP_vec<int>& dc_pos = desc->var_change._del_change_pos;
    BCP_vec<BCP_obj_change>& ch = desc->var_change._change;
    dc_pos.reserve(affected_added);
    ch.reserve(affected_added);
    posi = brobj->var_positions().begin();
    BCP_vec<double>::const_iterator boundsi = brobj->var_bounds_child(childind);
    while (posi != lastposi) {
	if (*posi >= bvarnum) {
	    dc_pos.unchecked_push_back(*posi - bvarnum);
	    const double lb = *boundsi;
	    const double ub = *(boundsi+1);
	    ch.unchecked_push_back(BCP_obj_change(lb, ub, BCP_ObjNotRemovable));
	}
	++posi;
	boundsi += 2;
    }
}

//#############################################################################

static void
BCP_tm_create_cut_change(BCP_node_change* desc,
			 const BCP_node_change* parentdesc, const int bcutnum,
			 BCP_internal_brobj* brobj, const int childind)
{
    // check first how many added cut has changed in brobj
    int affected_added = 0;
    BCP_vec<int>::const_iterator posi = brobj->cut_positions().begin();
    BCP_vec<int>::const_iterator lastposi = brobj->cut_positions().end();
    while (posi != lastposi) {
	if (*posi >= bcutnum)
	    ++affected_added;
	++posi;
    }

    if (affected_added == 0) {
	if (parentdesc->cut_change.storage() == BCP_Storage_Explicit &&
	    parentdesc->cut_change.added_num() == 0) {
	    desc->cut_change._storage = BCP_Storage_Explicit;
	} else {
	    desc->cut_change._storage = BCP_Storage_WrtParent;
	}
	return;
    }

    desc->cut_change._storage = BCP_Storage_WrtParent;
    BCP_vec<int>& dc_pos = desc->cut_change._del_change_pos;
    BCP_vec<BCP_obj_change>& ch = desc->cut_change._change;
    dc_pos.reserve(affected_added);
    ch.reserve(affected_added);
    posi = brobj->cut_positions().begin();
    BCP_vec<double>::const_iterator boundsi = brobj->cut_bounds_child(childind);
    while (posi != lastposi) {
	if (*posi >= bcutnum) {
	    dc_pos.unchecked_push_back(*posi - bcutnum);
	    const double lb = *boundsi;
	    const double ub = *(boundsi+1);
	    ch.unchecked_push_back(BCP_obj_change(lb, ub, BCP_ObjNotRemovable));
	}
	++posi;
	boundsi += 2;
    }
}

//#############################################################################

static void
BCP_tm_unpack_branching_info(BCP_tm_prob& p, BCP_buffer& buf,
			     BCP_tm_node* node)
{
    BCP_diving_status dive; // the old diving status

    BCP_vec<BCP_child_action> action;
    BCP_vec<BCP_user_data*> user_data;
    BCP_vec<double> true_lb;
    BCP_vec<double> qualities;

    buf.unpack(dive).unpack(action).unpack(qualities).unpack(true_lb);

    const int child_num = action.size();
    user_data.insert(user_data.end(), child_num, 0);
    bool has_user_data = false;
    for (int i = 0; i < child_num; ++i) {
	buf.unpack(has_user_data);
	if (has_user_data) {
	    user_data[i] = p.packer->unpack_user_data(buf);
	}
    }

    BCP_internal_brobj* brobj = new BCP_internal_brobj;
    brobj->unpack(buf);

    // generate the children
    const int bvarnum = p.core->varnum();
    const int bcutnum = p.core->cutnum();
    node->reserve_child_num(brobj->child_num());
    int keep = -1;
    BCP_tm_node* child = 0;
    BCP_node_change* desc;
    // nodedesc exists, because when we unpack the barnching info we just
    // received back the description of the node
    const BCP_node_change* nodedesc = node->_data._desc.GetRawPtr();
    int i;

    // fix the number of leaves assigned to the CP/VP
    if (node->cp != -1) {
	BCP_vec< std::pair<int, int> >::iterator proc =
	    BCP_tm_identify_process(p.leaves_per_cp, node->cp);
	if (proc == p.leaves_per_cp.end()) {
	    node->cp = -1; 
	} else {
	    proc->second += node->child_num() - 1;
	}
    }
    if (node->vp != -1) {
	BCP_vec< std::pair<int, int> >::iterator proc =
	    BCP_tm_identify_process(p.leaves_per_vp, node->vp);
	if (proc == p.leaves_per_vp.end()) {
	    node->vp = -1; 
	} else {
	    proc->second += node->child_num() - 1;
	}
    }

    CoinTreeNode** children = new CoinTreeNode*[child_num];
    int numChildrenAdded = 0;
    const int depth = node->getDepth() + 1;
    for (i = 0; i < child_num; ++i){
	desc = new BCP_node_change;
	BCP_tm_create_core_change(desc, bvarnum, bcutnum, brobj, i);
	BCP_tm_create_var_change(desc, nodedesc, bvarnum, brobj, i);
	BCP_tm_create_cut_change(desc, nodedesc, bcutnum, brobj, i);
	if (nodedesc->warmstart)
	    // If the parent has warmstart info then 
	    desc->warmstart = nodedesc->warmstart->empty_wrt_this();

	child = new BCP_tm_node(node->getDepth() + 1, desc);
	child->_core_storage = desc->core_change.storage();
	child->_var_storage = desc->var_change.storage();
	child->_cut_storage = desc->cut_change.storage();
	child->_ws_storage =
	    desc->warmstart ? desc->warmstart->storage() : BCP_Storage_NoData;

	p.search_tree.insert(child); // this sets _index
	child->_data._user = user_data[i];
	child->_parent = node;
	child->_birth_index = node->child_num();
	/* Fill out the fields in CoinTreeNode */
	child->setDepth(depth);
	child->setQuality(qualities[i]);
	child->setTrueLB(true_lb[i]);
	if (i > 0 && depth <= 63) {
	  child->setPreferred(node->getPreferred() | (1 << (63-depth)));
	} else {
	  child->setPreferred(node->getPreferred());
	}
	//	printf("index: %i, depth: %i, preferred: %li\n",
	//	       child->_index, depth, child->getPreferred());
	/* Add the child to the list of children in the parent */
	node->new_child(child);
	// _children  initialized to be empty -- OK
	switch (action[i]){
	case BCP_ReturnChild:
	    children[numChildrenAdded++] = child;
	    child->status = BCP_CandidateNode;
	    break;
	case BCP_KeepChild:
	    children[numChildrenAdded++] = child;
	    child->status = BCP_CandidateNode; // be conservative
	    keep = i;
	    break;
	case BCP_FathomChild:
	    child->status = BCP_PrunedNode_Discarded;
	    break;
	}
	// inherit var/cut pools
	child->vp = node->vp;
	child->cp = node->cp;
	// lp, cg, vg  initialized to -1 -- OK, none assigned yet
// #define DEBUG_PRINT
#ifdef DEBUG_PRINT
	printf("TM:    sibling[%i]: %i, quality: %lf, tlb: %lf, pref: %llX\n",
	       i, child->_index, child->getQuality(),
	       child->getTrueLB(), child->getPreferred());
#endif
    }

    if (numChildrenAdded > 0) {
      CoinTreeSiblings siblings(numChildrenAdded, children);

      BCP_print_memusage(p);
      p.need_a_TS = ! BCP_tm_is_data_balanced(p);

      // check the one that's proposed to be kept if there's one
      if (keep >= 0) {
	child = node->child(keep);
	if (dive == BCP_DoDive || dive == BCP_TestBeforeDive){
	  // we've got to answer
	  buf.clear();
	  if (p.need_a_TS) {
	    /* Force no diving if we need a TS process */
	    dive = BCP_DoNotDive;
	  } else {
	    if (dive == BCP_TestBeforeDive)
	      dive = BCP_tm_shall_we_dive(p, child->getQuality());
	  }
	  buf.pack(dive);
	  if (dive != BCP_DoNotDive){
	    p.user->display_node_information(p.search_tree, *child);
	    child->status = BCP_ActiveNode;
	    // if diving then send the new index and var/cut_names
	    buf.pack(child->index());
	    siblings.advanceNode();
	  }
	  p.candidate_list.push(siblings);
	  p.user->change_candidate_heap(p.candidate_list, false);
	  p.msg_env->send(node->lp, BCP_Msg_DivingInfo, buf);
	} else {
	  p.candidate_list.push(siblings);
	  p.user->change_candidate_heap(p.candidate_list, false);
	}
      } else {
	p.candidate_list.push(siblings);
	p.user->change_candidate_heap(p.candidate_list, false);
	dive = BCP_DoNotDive;
      }

      if (dive == BCP_DoNotDive){
	// lp,cg,vg becomes free (zeroes out node->{lp,cg,vg})
	BCP_tm_free_procs_of_node(p, node);
      } else {
	// if diving then the child takes over the parent's lp,cg,vg
	// XXX
	if (child != node->child(keep)) {
	  throw BCP_fatal_error("\
BCP_tm_unpack_branching_info: the value of child is messed up!\n");
	}
	if (node->lp == -1) {
	  throw BCP_fatal_error("\
BCP_tm_unpack_branching_info: the (old) node has no LP associated with!\n");
	}
	child->lp = node->lp;
	child->cg = node->cg;
	child->vg = node->vg;
	p.active_nodes[node->lp] = child;
	node->lp = node->cg = node->vg = -1;
      }
    }
    delete[] children;

    // and the node is done
    node->status = BCP_ProcessedNode;

    delete brobj;

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
    dump_procinfo(p, "BCP_tm_unpack_branching_info()");
#endif
#endif
}

//#############################################################################

void BCP_tm_unpack_node_with_branching_info(BCP_tm_prob& p, BCP_buffer& buf)
{
    const int index = BCP_tm_unpack_node_description(p, buf);
    /* In the middle of BCP_tm_unpack_branching_info we check for data
       balancing just before we (may) allow diving.*/
    BCP_tm_unpack_branching_info(p, buf, p.search_tree[index]);
    BCP_tm_print_info_line(p, *p.search_tree[index]);
    // BCP_tm_list_candidates(p);

    p.need_a_TS = BCP_tm_balance_data(p);
}

//#############################################################################

BCP_tm_node* BCP_tm_unpack_node_no_branching_info(BCP_tm_prob& p,
						  BCP_buffer& buf)
{
    const int index = BCP_tm_unpack_node_description(p, buf);

    BCP_print_memusage(p);
    p.need_a_TS = ! BCP_tm_is_data_balanced(p);

    // Mark the lp/cg/vg processes of the node as free
    BCP_tm_node* node = p.search_tree[index];
    BCP_tm_print_info_line(p, *node);
    BCP_tm_free_procs_of_node(p, node);

    p.need_a_TS = BCP_tm_balance_data(p);

    return node;
}
