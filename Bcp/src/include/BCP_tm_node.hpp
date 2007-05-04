// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_NODE
#define _BCP_TM_NODE

//#include <cfloat>

#include <map>

#include "CoinSearchTree.hpp"

#include "BCP_math.hpp"
#include "BCP_vector.hpp"

#include "BCP_message_tag.hpp"
#include "BCP_obj_change.hpp"

/** Node status in the Tree Manager. */

enum BCP_tm_node_status{
    /** */
    BCP_DefaultNode,
    /** */
    BCP_ProcessedNode,
    /** */
    BCP_ActiveNode,
    /** */
    BCP_PrunedNode_OverUB,
    /** */
    BCP_PrunedNode_Infeas,
    /** */
    BCP_PrunedNode_Discarded,
    /** */
    BCP_CandidateNode,
    /** */
    BCP_NextPhaseNode_OverUB,
    /** */
    BCP_NextPhaseNode_Infeas
};

//#############################################################################

class BCP_node_change;
class BCP_user_data;

class BCP_tm_node;
class BCP_tm_prob;

//#############################################################################

struct BCP_tm_node_data {
    BCP_node_change* _desc;
    BCP_user_data*   _user;
};

//=============================================================================

class BCP_tm_node : public CoinTreeNode {
private:
    /**@name Disabled methods */
    /*@{*/
    /** The copy constructor is declared but not defined to disable it. */
    BCP_tm_node(const BCP_tm_node&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_tm_node& operator=(const BCP_tm_node&);
    /*@}*/

    // NOTE: deleting a tree_node deletes the whole subtree below!
public:
    /**@name Data members */
    // *FIXME* break into groups 
    /*@{*/
    /** */
    BCP_tm_node_status status;
    /** */
    int _index;
    /** */
    BCP_tm_node* _parent;

    int _locally_stored:1;
    
    /** */
    int _birth_index:31;
    /** */
    BCP_vec<BCP_tm_node*> _children;
    /** */
    int lp, cg, cp, vg, vp;
    /** */
    int _processed_leaf_num;
    /** */
    int _pruned_leaf_num;
    /** */
    int _tobepriced_leaf_num;
    /** */
    int _leaf_num;

    int _core_storage:4;
    int _var_storage:4;
    int _cut_storage:4;
    int _ws_storage:4;
    union {
	BCP_tm_node_data _data;
	int _data_location;
    };
	
    /*@}*/

public:
    /**@name Constructors and destructor */
    /*@{*/
    /** */
    BCP_tm_node(int level, BCP_node_change* desc) :
	CoinTreeNode(level),
	status(BCP_DefaultNode),
	_index(0),
	_parent(0),
	_locally_stored(1),
	_birth_index(-1),
	_children(),
	lp(-1), cg(-1), cp(-1), vg(-1), vp(-1),
	_processed_leaf_num(0),
	_pruned_leaf_num(0),
	_tobepriced_leaf_num(0),
	_leaf_num(0),
	_core_storage(-1),
	_var_storage(-1),
	_cut_storage(-1),
	_ws_storage(-1)
    {
	_data._desc = desc;
	_data._user = NULL;
    }

    /** */
    BCP_tm_node(int level, BCP_node_change* desc,
		BCP_tm_node* parent, int index) :
	CoinTreeNode(level),
	status(BCP_DefaultNode),
	_index(0),
	_parent(0),
	_locally_stored(1),
	_birth_index(-1),
	_children(),
	lp(-1), cg(-1), cp(-1), vg(-1), vp(-1),
	_processed_leaf_num(0),
	_pruned_leaf_num(0),
	_tobepriced_leaf_num(0),
	_leaf_num(0)
    {
	_data._desc = desc;
	_data._user = NULL;
    }

    /** */
    ~BCP_tm_node();
    /*@}*/

    /**@name Query methods */
    /*@{*/
    /** */
    inline int index() const { return _index; }
    /** */
    inline int child_num() const { return _children.size(); }
    /** */
    inline int birth_index() const { return _birth_index; }

    /** */
    //    inline BCP_user_data* user_data() { return _data._user; }
    /** */
    inline BCP_tm_node* child(int ind) { return _children[ind]; }
    /** */
    inline BCP_tm_node* parent() { return _parent; }

    /** */
    //    inline const BCP_user_data* user_data() const { return _data._user; }
    /** */
    inline const BCP_tm_node* child(int ind) const { return _children[ind]; }
    /** */
    inline const BCP_tm_node* parent() const { return _parent; }
    /*@}*/


    /**@name Modifying methods */
    /*@{*/
    /** */
    // Marking the descendants for deletion means that their _index fields are
    // set to -1. The reason is that some book-keeping must be one with the CP,
    // VP processes; with the next phase list, with the priority queue of the
    // current phase (and maybe sthing else?). So this function only marks, and
    // the data will be deleted later.
    int mark_descendants_for_deletion();
    /** */
    void remove_child(BCP_tm_node* node);
    /** */
    inline void reserve_child_num(int num) { _children.reserve(num); }
    /** */
    inline void new_child(BCP_tm_node* node) { _children.push_back(node); }
    /*@}*/
};

//#############################################################################

/** NO OLD DOC */

class BCP_tree {
private:
    /** */
    BCP_vec<BCP_tm_node*> _tree;
    /** */
    int maxdepth_;
    int processed_;

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_tree() : _tree(), maxdepth_(0), processed_(0) {}
    /** */
    ~BCP_tree() {
	for (int i = _tree.size() - 1; i >= 0; --i) {
	    delete _tree[i];
	}
    }
    /*@}*/

    /**@name Query methods */
    /*@{*/
    /** */
    inline BCP_vec<BCP_tm_node*>::iterator begin() { return _tree.begin(); }
    /** */
    inline BCP_vec<BCP_tm_node*>::iterator end()   { return _tree.end(); }

    /** */
    inline BCP_tm_node* root() { return _tree.front(); }
    /** */
    inline BCP_tm_node* operator[](int index) { return _tree[index]; }
    /** */
    inline size_t size() const { return _tree.size(); }
    /** */
    inline int maxdepth() const { return maxdepth_; }
    /** */
    inline int processed() const { return processed_; }
    inline void increase_processed() { ++processed_; }
    /*@}*/

    /**@name Modifying methods */
    /*@{*/
    /** Return the worst true lower bound in the search tree */
    double true_lower_bound(const BCP_tm_node* node) const;
    /** */
    void enumerate_leaves(BCP_tm_node* node, const double obj_limit);
    /** */
    inline void insert(BCP_tm_node* node) {
	node->_index = _tree.size();
	_tree.push_back(node);
	if (node->getDepth() > maxdepth_)
	    maxdepth_ = node->getDepth();
    }
    inline void remove(int index) {
	_tree[index] = 0;
    }
    /*@}*/
};

//#############################################################################
class BCP_tm_node_to_send;

class BCP_tm_node_to_send
{
public:
    static std::map<int, BCP_tm_node_to_send*> waiting;

private:
    BCP_tm_prob& p;

    /** the message tag to be used when finally the node is sent off */
    BCP_message_tag msgtag;

    /** An identifier of this object. It is the index of the node we want to
	send out */
    const int ID;

    const BCP_tm_node* node;
    /** the path to the root. The root is root_path[0], the node itself is in
	root_path[level] */
    const BCP_tm_node** root_path;
    /** at each level the index of the child in the parent's list */
    int* child_index;
    /** the node data on each level (well, up to the point where we have
	encountered an explicit description for all kind of data) */
    BCP_tm_node_data* node_data_on_root_path;

    /** the level of node to be sent */
    int level;

    /** where the various pieces start to be explicit (or wrt. root/core) */
    int explicit_core_level;
    int explicit_var_level;
    int explicit_cut_level;
    int explicit_ws_level;
    int explicit_all_level;

    /** -1/nonneg unset/value : how many desc is missing */
    int missing_desc_num;
    /** -1/nonneg unset/value : how many var is missing */
    int missing_var_num;
    /** -1/nonneg unset/value : how many cut is missing */
    int missing_cut_num;

    /** The explicit description of the vars/cuts of the parent as a
	BCP_obj_set_change and the vars/cuts themselves (the change contains
	only indices!) */
    BCP_obj_set_change var_set;
    BCP_obj_set_change cut_set;
    /** The list of vars/cuts of the node when the changes of the node are
	applied to \c var_set and \c cut_set */
    BCP_vec<BCP_var*> vars;
    BCP_vec<BCP_cut*> cuts;

public:

    BCP_tm_node_to_send(BCP_tm_prob& p, const BCP_tm_node* node,
			const BCP_message_tag tag);

    /** return true or false depending on whether the node was really sent out
	or it's still waiting for some data */
    bool send();

    /** return true if has everything to send the thing off to the LP.
	Actually, it sends it off, so if this method returns true then
	then object can be deleted */
    bool receive_node_desc(BCP_buffer& buf);
    /** return true if has everything to send the thing off to the LP.
	Actually, it sends it off, so if this method returns true then
	then object can be deleted */
    bool receive_vars(BCP_buffer& buf);
    /** return true if has everything to send the thing off to the LP.
	Actually, it sends it off, so if this method returns true then
	then object can be deleted */
    bool receive_cuts(BCP_buffer& buf);
};

#endif
