// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "CoinTime.hpp"
#include "BCP_message.hpp"

#include "BCP_enum_branch.hpp"
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"

#include "BCP_tm.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_USER.hpp"
#include "BCP_message_tag.hpp"

//XXX
#include "BCP_tm_functions.hpp"

#ifndef BCP_DEBUG_PRINT
#define BCP_DEBUG_PRINT 0
#endif

//#############################################################################

std::map<int, BCP_tm_node_to_send*> BCP_tm_node_to_send::waiting;

//#############################################################################

BCP_tm_node_to_send::BCP_tm_node_to_send(BCP_tm_prob& prob,
					 const BCP_tm_node* node_to_send,
					 const BCP_message_tag tag) :
    p(prob),
    msgtag(tag),
    ID(node_to_send->_index),
    node(node_to_send),
    root_path(NULL),
    child_index(NULL),
    node_data_on_root_path(NULL),
    explicit_core_level(-1),
    explicit_var_level(-1),
    explicit_cut_level(-1),
    explicit_ws_level(-1),
    explicit_all_level(-1),
    missing_desc_num(-1),
    missing_var_num(-1),
    missing_cut_num(-1)
{
    level = node->getDepth();
    const BCP_tm_node* n = node;

    root_path = new const BCP_tm_node*[level + 1];
    child_index = new int[level];;
    node_data_on_root_path = new BCP_tm_node_data[level+1];
    
    // the path to the root
    int i = level;
    while (explicit_core_level < 0 || explicit_var_level < 0 ||
	   explicit_cut_level < 0 || explicit_ws_level < 0) {
	assert(i >= 0);
	root_path[i] = n;
	// some stuff is on other processes
	if (n->_locally_stored) {
	    node_data_on_root_path[i] = n->_data;
	} else {
	    node_data_on_root_path[i]._desc = NULL;
	    node_data_on_root_path[i]._user = NULL;
	}
	  
	if (n->_core_storage!=BCP_Storage_WrtParent && explicit_core_level<0) {
	    explicit_core_level = i;
	    explicit_all_level = i;
	}
	if (n->_var_storage!=BCP_Storage_WrtParent && explicit_var_level<0) {
	    explicit_var_level = i;
	    explicit_all_level = i;
	}
	if (n->_cut_storage!=BCP_Storage_WrtParent && explicit_cut_level<0) {
	    explicit_cut_level = i;
	    explicit_all_level = i;
	}
	if (n->_ws_storage!=BCP_Storage_WrtParent && explicit_ws_level<0) {
	    explicit_ws_level = i;
	    explicit_all_level = i;
	}
	--i;
	n = n->parent();
    }
    for (i = explicit_all_level + 1; i <= level; i++) {
	child_index[i-1] = root_path[i]->birth_index();
    }
    BCP_tm_node_to_send::waiting[ID] = this;
}

//#############################################################################

BCP_tm_node_to_send::~BCP_tm_node_to_send()
{
    delete[] root_path;
    delete[] child_index;
    delete[] node_data_on_root_path;
}

//#############################################################################

bool
BCP_tm_node_to_send::receive_node_desc(BCP_buffer& buf)
{
    const bool def = p.param(BCP_tm_par::ReportWhenDefaultIsExecuted);
    int cnt, lclLevel, index;
    buf.unpack(cnt);
    missing_desc_num -= cnt;
    bool has_user_data = false;
    while (--cnt >= 0) {
	buf.unpack(lclLevel).unpack(index);
	assert(root_path[lclLevel]->_index == index);
	node_data_on_root_path[lclLevel]._desc =
	    new BCP_node_change(p.packer, def, buf);
	buf.unpack(has_user_data);
	node_data_on_root_path[lclLevel]._user =
	  has_user_data ? p.packer->unpack_user_data(buf) : 0;
    }
    assert(missing_desc_num >= 0);
    if (missing_desc_num == 0) {
	return send();
    }
    return false;
}

//#############################################################################

bool
BCP_tm_node_to_send::receive_vars(BCP_buffer& buf)
{
    int cnt, pos, index;
    buf.unpack(cnt);
    missing_var_num -= cnt;
    while (--cnt >= 0) {
	buf.unpack(pos).unpack(index);
	vars[pos] = p.packer->unpack_var_algo(buf);
	assert(var_set._new_objs[pos] == vars[pos]->bcpind());
    }
    assert(missing_var_num >= 0);
    if (missing_var_num == 0 && missing_cut_num == 0) {
	return send();
    }
    return false;
}

//#############################################################################

bool
BCP_tm_node_to_send::receive_cuts(BCP_buffer& buf)
{
    int cnt, pos, index;
    buf.unpack(cnt);
    missing_cut_num -= cnt;
    while (--cnt >= 0) {
	buf.unpack(pos).unpack(index);
	cuts[pos] = p.packer->unpack_cut_algo(buf);
	assert(cut_set._new_objs[pos] == cuts[pos]->bcpind());
    }
    assert(missing_cut_num >= 0);
    if (missing_var_num == 0 && missing_cut_num == 0) {
	return send();
    }
    return false;
}

//#############################################################################

bool
BCP_tm_node_to_send::send()
{
    int i;

    if (missing_desc_num < 0) {
	missing_desc_num = 0;
	// collect what needs od be asked for
	std::map< int, BCP_vec<int> > tms_nodelevel;
	for (i = explicit_all_level; i <= level; i++) {
	    if (node_data_on_root_path[i]._desc.IsNull()) {
		tms_nodelevel[root_path[i]->_data_location].push_back(i);
		++missing_desc_num;
	    }
	}
	BCP_buffer& buf = p.msg_buf;
	std::map< int, BCP_vec<int> >::const_iterator tms;
	BCP_vec<int> indices;
	for (tms = tms_nodelevel.begin(); tms != tms_nodelevel.end(); ++tms) {
	    buf.clear();
	    buf.pack(ID);
	    const BCP_vec<int>& nodelevel = tms->second;
	    indices.clear();
	    const int size = nodelevel.size();
	    indices.reserve(size);
	    for (i = 0; i < size; ++i) {
		indices.unchecked_push_back(root_path[nodelevel[i]]->_index);
	    }
	    buf.pack(nodelevel);
	    buf.pack(indices);
	    p.msg_env->send(tms->first, BCP_Msg_NodeListRequest, buf);
	}
    }

    if (missing_desc_num > 0) {
	return false;
    }

#if 0
    // FIXME--DELETE (used to test Bonmin code)
    for (i = 0; i <= level; ++i) {
      assert(node_data_on_root_path[level]._desc.IsValid());
      assert(node_data_on_root_path[level]._user.IsValid());
    }
#endif

    // OK, we have all the descriptions. Now if we haven't done so yet (which
    // is indicated by missing_var_num (and missing_cut_num) being negative)
    // we need to create the explicit parent description and the current node
    // description (vars/cuts will be present only with their bcpind). Also,
    // we need to create the full list of variables for the node itself (and
    // not for the parent).
    if (missing_var_num < 0) {
	missing_var_num = 0;
	missing_cut_num = 0;
	std::map< int, BCP_vec<int> > tms_pos;
	std::map< int, BCP_vec<int> >::const_iterator tms;
	std::map<int, int>::iterator remote;
	std::map<int, Coin::SmartPtr<BCP_var> >::iterator localvar;
	std::map<int, Coin::SmartPtr<BCP_cut> >::iterator localcut;
	BCP_buffer& buf = p.msg_buf;
	BCP_vec<int> indices;

	if (node->_var_storage == BCP_Storage_WrtParent) {
	    for (i = explicit_var_level; i < level; ++i) {
		var_set.update(node_data_on_root_path[i]._desc->var_change);
	    }
	    // FIXME: can it be BCP_Storage_NoData ???
	    assert (var_set._storage == BCP_Storage_Explicit);
	}
	BCP_obj_set_change node_var_set = var_set;
	node_var_set.update(node_data_on_root_path[level]._desc->var_change);
	BCP_vec<int>& var_inds = node_var_set._new_objs;
	const int varnum = var_inds.size();
	vars.reserve(varnum);
	// check whether we have all the vars
	for (i = 0; i < varnum; ++i) {
	    remote = p.vars_remote.find(var_inds[i]);
	    if (remote != p.vars_remote.end()) {
		tms_pos[remote->second].push_back(i);
		vars.unchecked_push_back(NULL);
		++missing_var_num;
		continue;
	    }
	    localvar = p.vars_local.find(var_inds[i]);
	    if (localvar != p.vars_local.end()) {
		// FIXME: cloning could be avoided by using smart pointers
		vars.unchecked_push_back(localvar->second);
		continue;
	    }
	    throw BCP_fatal_error("\
TM: var in node description is neither local nor remote.\n");
	}
	for (tms = tms_pos.begin(); tms != tms_pos.end(); ++tms) {
	    buf.clear();
	    buf.pack(ID);
	    const BCP_vec<int>& pos = tms->second;
	    const int num = pos.size();
	    indices.clear();
	    indices.reserve(num);
	    for (i = 0; i < num; ++i) {
		indices.unchecked_push_back(var_inds[pos[i]]);
	    }
	    buf.pack(pos);
	    buf.pack(indices);
	    p.msg_env->send(tms->first, BCP_Msg_VarListRequest, buf);
	}

	if (node->_cut_storage == BCP_Storage_WrtParent) {
	    for (i = explicit_cut_level; i < level; ++i) {
		cut_set.update(node_data_on_root_path[i]._desc->cut_change);
	    }
	    // FIXME: can it be BCP_Storage_NoData ???
	    assert (cut_set._storage == BCP_Storage_Explicit);
	}
	BCP_obj_set_change node_cut_set = cut_set;
	node_cut_set.update(node_data_on_root_path[level]._desc->cut_change);
	BCP_vec<int>& cut_inds = node_cut_set._new_objs;
	const int cutnum = cut_inds.size();
	cuts.reserve(cutnum);
	// check whether we have all the cuts
	tms_pos.clear();
	for (i = 0; i < cutnum; ++i) {
	    remote =p.cuts_remote.find(cut_inds[i]);
	    if (remote != p.cuts_remote.end()) {
		tms_pos[remote->second].push_back(i);
		cuts.unchecked_push_back(NULL);
		++missing_cut_num;
		continue;
	    }
	    localcut = p.cuts_local.find(cut_inds[i]);
	    if (localcut != p.cuts_local.end()) {
		// FIXME: cloning could be avoided by using smart pointers
		cuts.unchecked_push_back(localcut->second);
		continue;
	    }
	    throw BCP_fatal_error("\
TM: cut in node description is neither local nor remote.\n");
	}
	for (tms = tms_pos.begin(); tms != tms_pos.end(); ++tms) {
	    buf.clear();
	    buf.pack(ID);
	    const BCP_vec<int>& pos = tms->second;
	    const int num = pos.size();
	    indices.clear();
	    indices.reserve(num);
	    for (i = 0; i < num; ++i) {
		indices.unchecked_push_back(cut_inds[pos[i]]);
	    }
	    buf.pack(pos);
	    buf.pack(indices);
	    p.msg_env->send(tms->first, BCP_Msg_CutListRequest, buf);
	}
    }

    if (missing_var_num > 0 || missing_cut_num > 0) {
	return false;
    }

    //=========================================================================
    // Great! Now we have everything. Start to pack it up.
    const bool def = p.param(BCP_tm_par::ReportWhenDefaultIsExecuted);
    p.user->display_node_information(p.search_tree, *node);

    BCP_diving_status dive =
	(rand() < p.param(BCP_tm_par::UnconditionalDiveProbability)*RAND_MAX) ?
	BCP_DoDive : BCP_TestBeforeDive;

    // start with book-keeping data
    BCP_buffer& buf = p.msg_buf;
    buf.clear();
    buf.pack(p.current_phase_colgen).pack(node->index()).pack(level).
	pack(node->getQuality()).pack(node->getTrueLB()).pack(dive);

    // pack the process information
    buf.pack(node->cg).pack(node->cp).pack(node->vg).pack(node->vp);

    // pack how things are stored in node. this will influence what do we pack
    // in the parent, too.
    buf.pack(node->_core_storage).
	pack(node->_var_storage).
	pack(node->_cut_storage).
	pack(node->_ws_storage);
    
    // Now pack the parent if there's one
    if (level > 0) {
	p.msg_buf.pack(node->parent()->index());
	// start with the core
	if (node->_core_storage == BCP_Storage_WrtParent) {
	    BCP_problem_core_change core;
	    for (i = explicit_core_level; i < level; ++i) {
		core.update(*p.core_as_change,
			    node_data_on_root_path[i]._desc->core_change);
	    }
	    core.make_wrtcore_if_shorter(*p.core_as_change);
	    core.pack(p.msg_buf);
	}

	// next the variabless
	if (node->_var_storage == BCP_Storage_WrtParent) {
	    var_set.pack(buf);
	}

	// now the cuts
	if (node->_cut_storage == BCP_Storage_WrtParent) {
	    cut_set.pack(buf);
	}

	// finally warmstart
	if (node->_ws_storage == BCP_Storage_WrtParent) {
	    BCP_warmstart* warmstart =
		node_data_on_root_path[explicit_ws_level]._desc->warmstart->clone();
	    for (i = explicit_ws_level + 1; i < level; ++i) {
		warmstart->update(node_data_on_root_path[i]._desc->warmstart);
	    }
	    p.packer->pack_warmstart(warmstart, p.msg_buf, def);
	    delete warmstart;
	}
    }

    // finally pack the changes in this node
    node_data_on_root_path[level]._desc->pack(p.packer, def, buf);
    
    // Now pack the full list of vars/cuts of the node
    int cnt = vars.size();
    buf.pack(cnt);
    for (i = 0; i < cnt; ++i) {
	p.pack_var(*vars[i]);
    }
    cnt = cuts.size();
    buf.pack(cnt);
    for (i = 0; i < cnt; ++i) {
	p.pack_cut(*cuts[i]);
    }

    const BCP_user_data* ud = node_data_on_root_path[level]._user.GetRawPtr();
    bool has_user_data = ud != 0;
    buf.pack(has_user_data);
    if (has_user_data) {
	p.packer->pack_user_data(ud, buf);
    }

#if (BCP_DEBUG_PRINT != 0)
    //    const char* compName = p.candidate_list.getTree()->compName();
    printf("TM %.3lf: Sending to proc %i  node: %i  quality: %lf  pref: %s\n",
	   CoinWallclockTime()-p.start_time,
	   node->lp, node->_index, node->getQuality(), 
	   node->getPreferred().str().c_str());

#endif

    p.msg_env->send(node->lp, msgtag, buf);
    if (node->_index == 0) {
      p.root_node_sent_ = CoinGetTimeOfDay();
    }

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
    dump_procinfo(p, "BCP_tm_send_node()");
#endif
#endif
    return true;
}
