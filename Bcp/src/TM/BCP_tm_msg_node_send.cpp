// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_message.hpp"

#include "BCP_enum_branch.hpp"
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"

#include "BCP_tm.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_USER.hpp"

//XXX
#include "BCP_tm_functions.hpp"

static inline void
BCP_tm_pack_core_objects(BCP_tm_prob& p, const BCP_tm_node* node,
			 const BCP_tm_node* const * root_path,
			 const int* const child_index);
static inline void
BCP_tm_pack_added_vars(BCP_tm_prob& p, const BCP_tm_node* node,
		       const BCP_tm_node* const * root_path,
		       const int* const child_index);
static inline void
BCP_tm_pack_added_cuts(BCP_tm_prob& p, const BCP_tm_node* node,
		       const BCP_tm_node* const * root_path,
		       const int* const child_index);
static inline void
BCP_tm_pack_warmstart(BCP_tm_prob& p, const BCP_tm_node* node,
		      const BCP_tm_node* const * root_path);
static inline void
BCP_tm_pack_parent(BCP_tm_prob& p, const BCP_tm_node* node);

static inline void
BCP_tm_pack_procid(BCP_message_environment* msg_env,
		   BCP_buffer& buf, const BCP_proc_id* pid);

//#############################################################################

static inline void
BCP_tm_pack_core_objects(BCP_tm_prob& p, const BCP_tm_node* node,
			 const BCP_tm_node* const * root_path,
			 const int* const child_index)
{
   // Go up in the tree to find where do we store the core objects
   // explicitely or wrt core for the first time. (root at the latest)
   BCP_tm_node* n = const_cast<BCP_tm_node*>(node);
   for ( ;
	 n->_desc->core_change.storage() == BCP_Storage_WrtParent;
	 n = n->parent());

   BCP_problem_core_change core;
   const int level = node->level();
   int i;
   for (i = n->level(); i < level; ++i)
      core.update(*p.core_as_change, root_path[i]->_desc->core_change);
   core.make_wrtcore_if_shorter(*p.core_as_change);
   core.pack(p.msg_buf);
}

//-----------------------------------------------------------------------------

static inline void
BCP_tm_pack_added_vars(BCP_tm_prob& p, const BCP_tm_node* node,
		       const BCP_tm_node* const * root_path,
		       const int* const child_index)
{
   // Go up in the tree to find where do we store the added variables
   // explicitely for the first time. (root at the latest) At the same time,
   // compute an upper bound on the number of variables.
   BCP_tm_node* n = const_cast<BCP_tm_node*>(node);
   int vars_num = 0; // upper bound on the explicit size (there might be
		     // deleted vars, too...
   for ( ; n->_desc->var_change.storage() == BCP_Storage_WrtParent;
	vars_num += n->_desc->var_change.added_num(), n = n->parent());
   vars_num += n->_desc->var_change.added_num();

   // var_set will hold the set of variables as an explicit var_set_change
   // the reason for this is that we'll have only pointers to the vars, not
   // copies of them, so we want to store the current lb/ub/st in a vector of
   // obj_changes. But then it's easier to use a var_set_change object.
   BCP_var_set_change var_set;
   const int level = node->level();
   for (int i = n->level(); i < level; ++i) {
      var_set.update(root_path[i]->_desc->var_change);
   }
   p.pack_var_set_change(var_set);
}

//-----------------------------------------------------------------------------

static inline void
BCP_tm_pack_added_cuts(BCP_tm_prob& p, const BCP_tm_node* node,
		       const BCP_tm_node* const * root_path,
		       const int* const child_index)
{
   // Go up in the tree to find where do we store the added cuts explicitely
   // for the first time. (root at the latest) At the same time, compute an
   // upper bound on the number of cuts.
   BCP_tm_node* n = const_cast<BCP_tm_node*>(node);
   int cuts_num = 0;
   for ( ; n->_desc->cut_change.storage() == BCP_Storage_WrtParent;
	cuts_num += n->_desc->cut_change.added_num(), n = n->parent());
   cuts_num += n->_desc->cut_change.added_num(); // the explicit size

   // cut_set will hold the set of cuts as an explicit cut_set_change
   // the reason for this is that we'll have only pointers to the cuts, not
   // copies of them, so we want to store the current lb/ub/st in a vector of
   // obj_changes. But then it's easier to use a cut_set_change object.
   BCP_cut_set_change cut_set;
   const int level = node->level();
   for (int i = n->level(); i < level; ++i) {
      cut_set.update(root_path[i]->_desc->cut_change);
   }
   p.pack_cut_set_change(cut_set);
}

//-----------------------------------------------------------------------------

static inline void
BCP_tm_pack_warmstart(BCP_tm_prob& p, const BCP_tm_node* node,
		      const BCP_tm_node* const * root_path)
{
   // Go up in the tree to find where do we store the warmstart info
   // explicitely for the first time (root at the latest). 
   BCP_tm_node* n = const_cast<BCP_tm_node*>(node);
   for ( ;
	 n->_desc->warmstart->storage() == BCP_Storage_WrtParent;
	 n = n->parent());

   BCP_warmstart* warmstart = n->_desc->warmstart->clone();
   const int level = node->level();
   for (int i = n->level() + 1; i < level; ++i)
      warmstart->update(root_path[i]->_desc->warmstart);
   p.user->pack_warmstart(warmstart, p.msg_buf);
   delete warmstart;   warmstart = 0;
}

//-----------------------------------------------------------------------------

static inline void
BCP_tm_pack_parent(BCP_tm_prob& p, const BCP_tm_node* node)
{
   const int level = node->level();
   BCP_node_change* desc = node->_desc;
   BCP_tm_node* n = const_cast<BCP_tm_node*>(node);
   int i;

   int parentind = (node->parent() == NULL) ? -1 : node->parent()->_index ;
   p.msg_buf.pack(parentind);

   // the path to the root
   BCP_tm_node** root_path = new BCP_tm_node*[level + 1];
   for (i = level + 1; n; root_path[--i] = n, n = n->parent());

   // the child indices when tracing back from root to node
   int* child_index = new int[level];
   for (i = 1; i <= level; i++)
      child_index[i-1] = root_path[i]->birth_index();

   if (desc->core_change.storage() == BCP_Storage_WrtParent)
      BCP_tm_pack_core_objects(p, node, root_path, child_index);
   if (desc->var_change.storage() == BCP_Storage_WrtParent)
      BCP_tm_pack_added_vars(p, node, root_path, child_index);
   if (desc->cut_change.storage() == BCP_Storage_WrtParent)
      BCP_tm_pack_added_cuts(p, node, root_path, child_index);
   if (desc->warmstart && desc->warmstart->storage() == BCP_Storage_WrtParent)
      BCP_tm_pack_warmstart(p, node, root_path);

   delete[] child_index;
   delete[] root_path;
}

//#############################################################################

static inline void
BCP_tm_pack_procid(BCP_message_environment* msg_env,
		   BCP_buffer& buf, const BCP_proc_id* pid)
{
   bool has_id = pid != 0;
   buf.pack(has_id);
   if (has_id)
      msg_env->pack_proc_id(buf, pid);
}

//#############################################################################

//-----------------------------------------------------------------------------
// the data are store like this:
// exp, wrt, wrt, wrt, ...., wrt, wrt
// |-------- this is 'parent'-------|     : "P"
// pack_parent() packs only "P" and send_node() packs the last wrt.
// core objects are handled separately from the added ones, because they can
// only change (can't be deleted/added).

void BCP_tm_send_node(BCP_tm_prob& p, const BCP_tm_node* node,
		      const BCP_message_tag msgtag)
{
   p.user->display_node_information(p.search_tree, *node);
   BCP_buffer& buf = p.msg_buf;
   BCP_node_change* desc = node->_desc;
   BCP_diving_status dive =
      (rand() < p.param(BCP_tm_par::UnconditionalDiveProbability) * RAND_MAX) ?
      BCP_DoDive : BCP_TestBeforeDive;

   // start with book-keeping data
   buf.clear();
   buf.pack(p.current_phase_colgen).pack(node->index()).pack(node->level())
      .pack(node->quality()).pack(node->true_lower_bound())
      .pack(dive);

   // pack the process information
   BCP_tm_pack_procid(p.msg_env, buf, node->cg);
   BCP_tm_pack_procid(p.msg_env, buf, node->cp);
   BCP_tm_pack_procid(p.msg_env, buf, node->vg);
   BCP_tm_pack_procid(p.msg_env, buf, node->vp);

   // pack how things are stored in node. this will influence what do we pack
   // in the parent, too.
   BCP_storage_t ws_storage =
      desc->warmstart ? desc->warmstart->storage() : BCP_Storage_NoData;
   buf.pack(desc->core_change.storage())
      .pack(desc->var_change.storage())
      .pack(desc->cut_change.storage())
      .pack(ws_storage);

   // Now pack the parent if there's one
   if (node->level() > 0)
      BCP_tm_pack_parent(p, node);

   // finally pack the changes in this node
   desc->core_change.pack(buf);
   p.pack_var_set_change(desc->var_change);
   p.pack_cut_set_change(desc->cut_change);
   
   bool has_data = desc->warmstart != 0;
   buf.pack(has_data);
   if (has_data)
      p.user->pack_warmstart(desc->warmstart, buf);

   has_data = node->_user_data != 0;
   buf.pack(has_data);
   if (has_data)
      p.user->pack_user_data(node->_user_data, buf);

   p.msg_env->send(node->lp, msgtag, buf);

#ifdef BCP__DUMP_PROCINFO
#if (BCP__DUMP_PROCINFO == 1)
   dump_procinfo(p, "BCP_tm_send_node()");
#endif
#endif
}
