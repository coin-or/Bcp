// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <functional>
#include "BCP_message.hpp"
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_functions.hpp"

static inline void
BCP_lp_unpack_parent(BCP_lp_prob& p, BCP_buffer& buf, BCP_lp_node& node);

static inline void
BCP_lp_create_node(BCP_lp_prob& p, BCP_node_change& node_change);

static inline void
BCP_lp_create_core(BCP_lp_prob& p, BCP_node_change& node_change);
static inline void
BCP_lp_set_core(BCP_lp_prob& p, BCP_lp_node& node, BCP_problem_core_change& core);
static inline void
BCP_lp_modify_core(BCP_lp_node& node, BCP_problem_core_change& change);

static inline void
BCP_lp_create_added_vars(BCP_lp_prob& p, BCP_node_change& node_change);
static inline void
BCP_lp_create_added_cuts(BCP_lp_prob& p, BCP_node_change& node_change);

static inline void
BCP_lp_create_warmstart(BCP_lp_prob& p, BCP_node_change& node_change);

//-----------------------------------------------------------------------------

void BCP_lp_unpack_parent(BCP_lp_prob& p, BCP_buffer& buf, BCP_lp_node& node)
{
    buf.unpack(p.parent->index);
    if (node.tm_storage.core_change == BCP_Storage_WrtParent){
	p.parent->core_as_change.unpack(buf);  // BCP_problem_core_change
	// make sure the parent's storage is Explicit (convert if it is
	// WrtCore now). This way later we can test for WrtParent easily.
	p.parent->core_as_change.ensure_explicit(*p.core_as_change);
    }

    int i;

    assert(node.vars.size() == p.core->vars.size());
    if (node.tm_storage.var_change == BCP_Storage_WrtParent){
	// these are the vars present in the parent
	p.parent->var_set.unpack(buf);
#ifdef PARANOID
	if (p.parent->var_set.storage() != BCP_Storage_Explicit)
	    throw BCP_fatal_error("BCP_lp_unpack_parent(): oops 1\n");
#endif
	const int num = p.parent->var_set.added_num();
	node.vars.reserve(num);
	for (i = 0; i < num; ++i) {
	    node.vars.unchecked_push_back(p.packer->unpack_var_algo(buf));
	}
    } else {
	assert(p.parent->var_set._change.empty());
	assert(p.parent->var_set._new_objs.empty());
    }

    assert(node.cuts.size() == p.core->cuts.size());
    if (node.tm_storage.cut_change == BCP_Storage_WrtParent){
	// these are the cuts present in the parent
	p.parent->cut_set.unpack(buf);
#ifdef PARANOID
	if (p.parent->cut_set.storage() != BCP_Storage_Explicit)
	    throw BCP_fatal_error("BCP_lp_unpack_parent(): oops 1\n");
#endif
	const int num = p.parent->cut_set.added_num();
	node.cuts.reserve(num);
	for (i = 0; i < num; ++i) {
	    node.cuts.unchecked_push_back(p.packer->unpack_cut_algo(buf));
	}
    } else {
	assert(p.parent->cut_set._change.empty());
	assert(p.parent->cut_set._new_objs.empty());
    }

    if (node.tm_storage.warmstart == BCP_Storage_WrtParent) {
	const bool def = p.param(BCP_lp_par::ReportWhenDefaultIsExecuted);
	p.parent->warmstart = p.packer->unpack_warmstart(buf, def);
    }
}

//-----------------------------------------------------------------------------
// The following two functions are needed below in BCP_lp_create_core()

void BCP_lp_modify_core(BCP_lp_node& node, BCP_problem_core_change& change)
{
    if (change.varnum() > 0)
	node.vars.set_lb_ub_st(change.var_pos.begin(), change.var_ch);
    if (change.cutnum() > 0)
	node.cuts.set_lb_ub_st(change.cut_pos.begin(), change.cut_ch);
}

void BCP_lp_set_core(BCP_lp_prob& p, BCP_lp_node& node,
		     BCP_problem_core_change& core)
{
    switch (core.storage()) {
    case BCP_Storage_WrtCore:
	if (p.core->varnum() > 0)
	    // this call sets the changes on the core
	    node.vars.set_lb_ub_st(p.core_as_change->var_ch);
	if (p.core->cutnum() > 0)
	    // this call sets the changes on the core
	    node.cuts.set_lb_ub_st(p.core_as_change->cut_ch);
	BCP_lp_modify_core(node, core);
	break;
      
    case BCP_Storage_Explicit:
	if (core.varnum() > 0)
	    // this call sets the changes on the core
	    node.vars.set_lb_ub_st(core.var_ch);
	if (core.cutnum() > 0)
	    // this call sets the changes on the core
	    node.cuts.set_lb_ub_st(core.cut_ch);
	break;
      
    default:
	// impossible in this function
	throw BCP_fatal_error("BCP_lp_set_core: Impossible storage_type.\n");
    }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_core(BCP_lp_prob& p, BCP_node_change& node_change)
{
    switch (p.node->tm_storage.core_change){
    case BCP_Storage_WrtCore:
	//in this case it'll copy the orig core first then update it
    case BCP_Storage_Explicit:
	// in this case it'll apply the core in node_change directly
	BCP_lp_set_core(p, *p.node, node_change.core_change);
	break;

    case BCP_Storage_WrtParent:
	// copy the parent lb/ub/status then apply the changes in node_change
	BCP_lp_set_core(p, *p.node, p.parent->core_as_change);
	BCP_lp_modify_core(*p.node, node_change.core_change);
	break;

    case BCP_Storage_NoData:
	// there are no core objects
	break;

    default:
	// impossible
	throw BCP_fatal_error("BCP_lp_create_core: Bad storage.\n");
    }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_added_vars(BCP_lp_prob& p, BCP_node_change& node_change)
    // When this function is called, the core vars are already listed in
    // p.node->vars, be careful not to destroy them
{
    switch (p.node->tm_storage.var_change) {
    case BCP_Storage_WrtParent:
    case BCP_Storage_Explicit:
	break;
    case BCP_Storage_NoData:
	// there are no added vars
	return;
    case BCP_Storage_WrtCore:
    default:
	// impossible
	throw BCP_fatal_error("BCP_lp_create_added_vars: Bad storage.\n");
    }
    // WrtParent || Explicit
    // It happens so that in case of explicit storage parent.var_set is empty,
    // so this is still OK!!
    BCP_obj_set_change var_set = p.parent->var_set;
    var_set.update(node_change.var_change);
    for (int i = var_set._change.size() - 1; i >= 0; --i) {
	p.node->vars[i]->change_lb_ub_st(var_set._change[i]);
    }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_added_cuts(BCP_lp_prob& p, BCP_node_change& node_change)
{
    switch (p.node->tm_storage.cut_change) {
    case BCP_Storage_WrtParent:
    case BCP_Storage_Explicit:
	break;
    case BCP_Storage_NoData:
	// there are no added cuts
	return;
    case BCP_Storage_WrtCore:
    default:
	// impossible
	throw BCP_fatal_error("BCP_lp_create_added_cuts: Bad storage.\n");
    }
    // WrtParent || Explicit
    // It happens so that in case of explicit storage parent.cut_set is empty,
    // so this is still OK!!
    BCP_obj_set_change cut_set = p.parent->cut_set;
    cut_set.update(node_change.cut_change);
    for (int i = cut_set._change.size() - 1; i >= 0; --i) {
	p.node->cuts[i]->change_lb_ub_st(cut_set._change[i]);
    }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_warmstart(BCP_lp_prob& p, BCP_node_change& node_change)
{
    switch (p.node->tm_storage.warmstart) {
    case BCP_Storage_WrtParent:
	p.node->warmstart = p.parent->warmstart->clone();
	p.node->warmstart->update(node_change.warmstart);
	break;

    case BCP_Storage_Explicit:
	p.node->warmstart = node_change.warmstart;
	node_change.warmstart = 0;
	break;

    case BCP_Storage_NoData:   // there's no warmstart info
	break;

    case BCP_Storage_WrtCore:
    default:
	// impossible
	throw BCP_fatal_error("BCP_lp_create_warmstart: Bad storage.\n");
    }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_node(BCP_lp_prob& p, BCP_node_change& node_change)
{
    // we got to put together p.node from parent and node_change
    p.node->iteration_count = 0;

    BCP_lp_create_core(p, node_change);
    BCP_lp_create_added_vars(p, node_change);
    BCP_lp_create_added_cuts(p, node_change);
    BCP_lp_create_warmstart(p, node_change);
}

//#############################################################################

void BCP_lp_unpack_active_node(BCP_lp_prob& p, BCP_buffer& buf)
{
    const bool def = p.param(BCP_lp_par::ReportWhenDefaultIsExecuted);

    if (p.parent->warmstart != 0 || p.node->warmstart != 0) {
	throw BCP_fatal_error("\
BCP_lp_unpack_active_node: parent's or node's warmstart is non-0.\n");
    }

    BCP_lp_node& node = *p.node;
    // unpack a few essential data
    buf.unpack(node.colgen).unpack(node.index).unpack(node.level)
	.unpack(node.quality).unpack(node.true_lower_bound)
	.unpack(node.dive);

    // unpack process information
    buf.unpack(node.cg);
    buf.unpack(node.cp);
    buf.unpack(node.vg);
    buf.unpack(node.vp);

    // unpack how the various pieces are stored in node
    buf.unpack(node.tm_storage.core_change)
	.unpack(node.tm_storage.var_change)
	.unpack(node.tm_storage.cut_change)
	.unpack(node.tm_storage.warmstart);

    if (node.level > 0)
	BCP_lp_unpack_parent(p, buf, node);

    BCP_node_change node_change;
    node_change.unpack(p.packer, def, buf);

    // Now unpack the full list of vars/cuts of the node
    int i, cnt;
    buf.unpack(cnt);
    assert(node.vars.size() == p.core->vars.size());
    node.vars.reserve(cnt);
    for (i = 0; i < cnt; ++i) {
	node.vars.unchecked_push_back(p.packer->unpack_var_algo(buf));
    }
    buf.unpack(cnt);
    assert(node.cuts.size() == p.core->cuts.size());
    node.cuts.reserve(cnt);
    for (i = 0; i < cnt; ++i) {
	node.cuts.unchecked_push_back(p.packer->unpack_cut_algo(buf));
    }

    // Create the active node from the parent and from the last changes
    BCP_lp_create_node(p, node_change);

    // Delete the old user data
    delete p.node->user_data;
    bool has_data;
    buf.unpack(has_data);
    p.node->user_data = has_data ? p.packer->unpack_user_data(buf) : 0;
}
