// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_math.hpp"
#include "BCP_USER.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_tm_node.hpp"
#include "BCP_node_change.hpp"

//#############################################################################

BCP_tm_node::~BCP_tm_node() {
    delete _user_data;
    delete _desc;
    delete lp; delete cg; delete vg; delete cp; delete vp;
}

//#############################################################################

int
BCP_tm_node::mark_descendants_for_deletion() {
    int del_num = child_num();
    if (del_num > 0) {
	BCP_vec<BCP_tm_node*>::iterator child = _children.begin();
	BCP_vec<BCP_tm_node*>::const_iterator lastchild = _children.end();
	while (child != lastchild) {
	    del_num += (*child)->mark_descendants_for_deletion();
	    (*child)->_index = -1;
	    ++child;
	}
	_children.clear();
    }
    return del_num;
}

//#############################################################################

void
BCP_tm_node::remove_child(BCP_tm_node* node)
{
    const int num_children = child_num();
    int i;
    for (i = num_children - 1; i >= 0; --i) {
	if (_children[i] == node)
	    break;
    }
    if (i < 0) {
	throw BCP_fatal_error("BCP_tm_node::remove_child : \
Trying to remove nonexistent child.\n");
    }
    _children[i] = _children[num_children-1];
    _children.pop_back();
}

//#############################################################################
// This member function counts for every serch tree node the number of
// descendants and that how many of them are pruned already / currently being
// processed. (This includes the node itself.)

void
BCP_tree::enumerate_leaves(BCP_tm_node* node, const double obj_limit)
{
    if (node->child_num() == 0) {
	const BCP_tm_node_status st = node->status;
	node->_leaf_num = 1;
	node->_processed_leaf_num = st == BCP_ActiveNode ? 1 : 0;
	const bool is_pruned = ( st == BCP_PrunedNode_OverUB ||
				 st == BCP_PrunedNode_Infeas ||
				 st == BCP_PrunedNode_Discarded );
	node->_pruned_leaf_num = is_pruned ? 1 : 0;
	const bool is_next_phase = ( st == BCP_NextPhaseNode_OverUB ||
				     st == BCP_NextPhaseNode_Infeas );
	node->_tobepriced_leaf_num =
	    ( ((st & (BCP_ProcessedNode|BCP_ActiveNode|BCP_CandidateNode)) &&
	       node->true_lower_bound() > obj_limit) ||
	      is_next_phase ) ? 1 : 0;
    } else {
	node->_leaf_num = 0;
	node->_processed_leaf_num = 0;
	node->_pruned_leaf_num = 0;
	node->_tobepriced_leaf_num = 0;
	BCP_vec<BCP_tm_node*>::iterator child;
	BCP_vec<BCP_tm_node*>::const_iterator lastchild =
	    node->_children.end();
	for (child = node->_children.begin(); child != lastchild; ++child) {
	    this->enumerate_leaves(*child, obj_limit);
	    node->_processed_leaf_num += (*child)->_processed_leaf_num;
	    node->_pruned_leaf_num += (*child)->_pruned_leaf_num;
	    node->_tobepriced_leaf_num += (*child)->_tobepriced_leaf_num;
	    node->_leaf_num += (*child)->_leaf_num;
	}
    }
}

//#############################################################################
// Find the best lower bound
double
BCP_tree::true_lower_bound(const BCP_tm_node* node) const
{
    double worstlb = BCP_DBL_MAX;
    if (node->child_num() == 0) {
	const BCP_tm_node_status st = node->status;
	if (st == BCP_ActiveNode || st == BCP_CandidateNode)
	    worstlb = node->true_lower_bound();
    } else {
	BCP_vec<BCP_tm_node*>::const_iterator child;
	BCP_vec<BCP_tm_node*>::const_iterator lastchild = node->_children.end();
	for (child = node->_children.begin(); child != lastchild; ++child) {
	    const double childlb = true_lower_bound(*child);
	    if (childlb < worstlb)
		worstlb = childlb;
	}
    }
    return worstlb;
}

//#############################################################################

void
BCP_node_queue::pop()
{
    BCP_tm_node* node = _pq.back();
    _pq.pop_back();

    const int size = _pq.size();
    if (size > 0) {
	int pos = 1;
	int ch;
	for (ch = 2; ch < size - 1; pos = ch, ch *= 2) {
	    if (_p.user->compare_tree_nodes(_pq[ch+1], _pq[ch]))
		++ch;
	    if (_p.user->compare_tree_nodes(node, _pq[ch]))
		break;
	    _pq[pos] = _pq[ch];
	}
	if (ch == size - 1) {
	    if (_p.user->compare_tree_nodes(_pq[ch], node)) {
		_pq[pos] = _pq[ch];
		pos = ch;
	    }
	}
	_pq[pos] = node;
    }
}

//=============================================================================

void
BCP_node_queue::insert(BCP_tm_node* node)
{
    int pos = _pq.size();
    int ch;
    _pq.push_back(node);
    for (ch = pos/2; ch != 0; pos = ch, ch /= 2) {
	if (_p.user->compare_tree_nodes(_pq[ch], node))
	    break;
	_pq[pos] = _pq[ch];
    }
    _pq[pos] = node;
}

//=============================================================================

void
BCP_node_queue::compare_to_UB(int& quality_above_UB, int& quality_below_UB)
{
    quality_above_UB = quality_below_UB = 0;
    const int size = _pq.size();
    if (! _p.has_ub()) {
	quality_below_UB = size;
	return;
    }
    const double threshold = _p.ub() - _p.granularity();
      
    for (int i = 1; i < size; ++i) {
	if (_pq[i]->quality() <= threshold) {
	    ++quality_below_UB;
	} else {
	    ++quality_above_UB;
	}
    }
}

//#############################################################################
