// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_NODE
#define _BCP_TM_NODE

//#include <cfloat>

#include "BCP_math.hpp"
#include "BCP_vector.hpp"


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

class BCP_proc_id;
class BCP_node_change;
class BCP_user_data;

class BCP_tm_node;
class BCP_tm_prob;

//#############################################################################

/** LITTLE OLD DESC */

class BCP_tm_node {
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
    int _level;
    /** */
    double _quality;
    /** */
    double _true_lower_bound;
    /** */
    BCP_node_change* _desc;
    /** */
    BCP_tm_node* _parent;
    /** */
    BCP_user_data* _user_data;
    /** */
    int _birth_index;
    /** */
    BCP_vec<BCP_tm_node*> _children;
    /** */
    BCP_proc_id* lp;
    /** */
    BCP_proc_id* cg;
    /** */
    BCP_proc_id* cp;
    /** */
    BCP_proc_id* vg;
    /** */
    BCP_proc_id* vp;
    /** */
    int _processed_leaf_num;
    /** */
    int _pruned_leaf_num;
    /** */
    int _tobepriced_leaf_num;
    /** */
    int _leaf_num;
    /*@}*/

public:
    /**@name Constructors and destructor */
    /*@{*/
    /** */
    BCP_tm_node(int level, BCP_node_change* desc) :
	status(BCP_DefaultNode),
	_index(0),
	_level(level),
	_quality(-BCP_DBL_MAX),
	_true_lower_bound(-BCP_DBL_MAX),
	_desc(desc),
	_parent(0),
	_user_data(0),
	_birth_index(-1),
	_children(),
	lp(0), cg(0), cp(0), vg(0), vp(0),
	_processed_leaf_num(0),
	_pruned_leaf_num(0),
	_tobepriced_leaf_num(0),
	_leaf_num(0)
    {}

    /** */
    BCP_tm_node(int level, BCP_node_change* desc,
		BCP_tm_node* parent, int index) :
	status(BCP_DefaultNode),
	_index(0),
	_level(level),
	_quality(-BCP_DBL_MAX),
	_true_lower_bound(-BCP_DBL_MAX),
	_desc(desc),
	_parent(0),
	_user_data(0),
	_birth_index(-1),
	_children(),
	lp(0), cg(0), cp(0), vg(0), vp(0),
	_processed_leaf_num(0),
	_pruned_leaf_num(0),
	_tobepriced_leaf_num(0),
	_leaf_num(0)
    {}

    /** */
    ~BCP_tm_node();
    /*@}*/

    /**@name Query methods */
    /*@{*/
    /** */
    inline int index() const { return _index; }
    /** */
    inline int level() const { return _level; }
    /** */
    inline int child_num() const { return _children.size(); }
    /** */
    inline double quality() const { return _quality; }
    /** */
    inline double true_lower_bound() const { return _true_lower_bound; }
    /** */
    inline int birth_index() const { return _birth_index; }

    /** */
    inline BCP_user_data* user_data() { return _user_data; }
    /** */
    inline BCP_tm_node* child(int ind) { return _children[ind]; }
    /** */
    inline BCP_tm_node* parent() { return _parent; }

    /** */
    inline const BCP_user_data* user_data() const { return _user_data; }
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
	if (node->_level > maxdepth_)
	    maxdepth_ = node->_level;
    }
    inline void remove(int index) {
	_tree[index] = 0;
    }
    /*@}*/
};

//#############################################################################

class BCP_node_queue {
private:
    BCP_node_queue();
    BCP_node_queue(const BCP_node_queue&);
    BCP_node_queue& operator=(const BCP_node_queue&);

private:
    /**@name Private data members */
    /*@{*/
    /** A reference to the problem structure so that we can invoke the method
	comparing two search tree nodes. */
    BCP_tm_prob& _p;
    /** The tree nodes in the priority queue.
	Note that the 0-th entry is not used in the tree (the loops are much
	nicer that way) and is always a NULL pointer.
    */
    BCP_vec<BCP_tm_node*> _pq;
    /*@}*/
  
public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_node_queue(BCP_tm_prob& p) : _p(p), _pq() { _pq.push_back(NULL); }
    /** */
    ~BCP_node_queue() {}
    /*@}*/

    /** Return whether the queue is empty or not */
    inline bool empty() const { return _pq.size() == 1; }

    /** Return the top member of the queue */
    BCP_tm_node* top() const { return _pq[1]; }

    /** Delete the top member of the queue. */
    void pop();

    /** Insert a new node into the queue. */
    void insert(BCP_tm_node* node);

    /** Find out how many candidates are below/above the current best upper
	bound (including the granularity!). */
    void compare_to_UB(int& quality_above_UB, int& quality_below_UB);
};

//#############################################################################

#endif
