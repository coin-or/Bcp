// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include "BCP_tm.hpp"
#include "BCP_tm_functions.hpp"

static int BCP_tm_trim_tree(BCP_tm_prob& p, BCP_tm_node* node,
			    const bool between_phases);

//#############################################################################
// For now we'll trim the tree only between phases when nothing is processed.
// Reason: It's kinda ugly to cut off a subtree when several of its nodes
// might be processed. Nevertheless, this should be done sooner or later.
//
// *THINK*
//#############################################################################

void BCP_tm_trim_tree_wrapper(BCP_tm_prob& p, const bool between_phases)
{
   BCP_tree& tree = p.search_tree;
   BCP_tm_node* root = tree.root();
   tree.enumerate_leaves(root, p.ub() - p.granularity());
   const int trimmed =
      BCP_tm_trim_tree(p, root, true /* called between phases */);
   if (p.param(BCP_tm_par::TmVerb_TrimmedNum)) {
      printf("\
TM: before starting the new phase, \n\
    %i nodes were trimmed from the tree.\n", trimmed);
   }

   BCP_vec<BCP_tm_node*>::iterator nodep;
   BCP_vec<BCP_tm_node*>::const_iterator lastnodep;
   BCP_tm_node * node;

   // clean up the next phase nodes
   nodep = p.next_phase_nodes.begin();
   lastnodep = p.next_phase_nodes.end();
   while (nodep != lastnodep) {
      if ((*nodep)->index() == -1) {
	 *nodep = *--lastnodep;
	 p.next_phase_nodes.pop_back();
      } else {
	 ++nodep;
      }
   }

   // clean up the list of candidates
   if (! p.candidates.empty()) {
      BCP_vec<BCP_tm_node*> cands;
      while (! p.candidates.empty()) {
	 node = p.candidates.top();
	 if (node->index() != -1)
	    cands.push_back(node);
	 p.candidates.pop();
      }
      lastnodep = cands.end();
      for (nodep = cands.begin(); nodep != lastnodep; ++nodep)
	 p.candidates.insert(*nodep);
   }

   // clean up the tree
   nodep = p.search_tree.begin();
   lastnodep = p.search_tree.end();
   while (nodep != lastnodep) {
      node = *nodep;
      if (node->index() == -1) {
	 // The search tree node is in a subtree that is trimmed
#ifdef BCP_DEBUG
	 if (node->lp != 0 || node->cg != 0 || node->vg != 0)
	    throw BCP_fatal_error("\
TM: At least on of lp/cg/vg of a trimmed node is non-0.\n");
#endif
	 if (node->cp && node->child_num() == 0) {
	    BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	       BCP_tm_identify_process(p.leaves_per_cp, node->cp);
#ifdef BCP_DEBUG
	    if (proc == p.leaves_per_cp.end())
	       throw BCP_fatal_error("\
TM: non-existing CP is assigned to a leaf.\n");
#endif
	    --proc->second;
	 }
	 if (node->vp && node->child_num() == 0) {
	    BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	       BCP_tm_identify_process(p.leaves_per_vp, node->vp);
#ifdef BCP_DEBUG
	    if (proc == p.leaves_per_vp.end())
	       throw BCP_fatal_error("\
TM: non-existing VP is assigned to a leaf.\n");
#endif
	    --proc->second;
	 }
	 delete node;
	 *nodep = 0;
      }
      ++nodep;
   }

   // scan through the CP and VP list to mark the free ones
   BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc;
   BCP_vec< std::pair<BCP_proc_id*, int> >::iterator lastproc;

   lastproc = p.leaves_per_cp.end();
   for (proc = p.leaves_per_cp.begin(); proc != lastproc; ++proc)
      if (proc->second == 0)
	 p.slaves.cp->set_proc_free(proc->first->clone());

   lastproc = p.leaves_per_vp.end();
   for (proc = p.leaves_per_vp.begin(); proc != lastproc; ++proc)
      if (proc->second == 0)
	 p.slaves.vp->set_proc_free(proc->first->clone());
}

//#############################################################################
// We will trim very conservatively

int BCP_tm_trim_tree(BCP_tm_prob& p, BCP_tm_node* node,
		     const bool between_phases)
{
   bool trim = true;
   int trimmed = 0;

   if (node->_processed_leaf_num > 0) {
      // if there is a node currently processed in the subtree then don't trim
      // here
      trim = false;
   } else if (node->child_num() == 0) {
      // if this is a leaf then there is no point in trimming
      trim = false;
   } else if (node->_leaf_num - node->_pruned_leaf_num <= 2) {
      // if there are no more than 2 nodes further down that have to be taken
      // care of then don't trim
      trim = false;
   } else if (node->true_lower_bound() < p.ub() - p.granularity()) {
      // don't trim if the gap at this node is not below the granularity
      trim = false;
   }

   if (trim) {
      trimmed = node->mark_descendants_for_deletion();
      if (node->cp) {
	 BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	    BCP_tm_identify_process(p.leaves_per_cp, node->cp);
#ifdef BCP_DEBUG
	 if (proc == p.leaves_per_cp.end())
	    throw BCP_fatal_error("\
TM: An internal node is assigned to a non-existing CP.\n");
#endif
	 ++proc->second;
      }
      if (node->vp) {
	 BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	    BCP_tm_identify_process(p.leaves_per_vp, node->vp);
#ifdef BCP_DEBUG
	 if (proc == p.leaves_per_vp.end())
	    throw BCP_fatal_error("\
TM: An internal node is assigned to a non-existing VP.\n");
#endif
	 ++proc->second;
      }
   } else {
      // try to trim at the children
      BCP_vec<BCP_tm_node*>::iterator child;
      BCP_vec<BCP_tm_node*>::const_iterator lastchild = node->_children.end();
      for (child = node->_children.begin(); child != lastchild; ++child)
	 trimmed += BCP_tm_trim_tree(p, *child, between_phases);
   }

   return trimmed;
}

//#############################################################################
