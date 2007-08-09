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
BCP_lp_unpack_parent(BCP_lp_prob& p, BCP_buffer& buf, BCP_lp_node& node,
		     BCP_var_set_change& parent_added_vars,
		     BCP_cut_set_change& parent_added_cuts);
static inline void
BCP_lp_create_node(BCP_lp_prob& p, BCP_node_change& node_change,
		   BCP_var_set_change& parent_added_vars,
		   BCP_cut_set_change& parent_added_cuts);
static inline void
BCP_lp_create_core(BCP_lp_prob& p, BCP_node_change& node_change);
static inline void
BCP_lp_set_core(BCP_lp_prob& p, BCP_lp_node& node, BCP_problem_core_change& core);
static inline void
BCP_lp_modify_core(BCP_lp_node& node, BCP_problem_core_change& change);
static inline void
BCP_lp_create_added_vars(BCP_lp_prob& p, BCP_node_change& node_change,
			 BCP_var_set_change& parent_added_vars);
static inline void
BCP_lp_create_added_cuts(BCP_lp_prob& p, BCP_node_change& node_change,
			 BCP_cut_set_change& parent_added_cuts);
static inline void
BCP_lp_create_warmstart(BCP_lp_prob& p, BCP_node_change& node_change);

static inline void
BCP_lp_unpack_procid(BCP_message_environment* msg_env,
		     BCP_buffer& buf, BCP_proc_id*& pid);
//-----------------------------------------------------------------------------

void BCP_lp_unpack_parent(BCP_lp_prob& p, BCP_buffer& buf, BCP_lp_node& node,
			  BCP_var_set_change& parent_added_vars,
			  BCP_cut_set_change& parent_added_cuts)
{
   if (node.tm_storage.core_change == BCP_Storage_WrtParent){
      p.parent->core_as_change.unpack(buf);  // BCP_problem_core_change
      // make sure the parent's storage is Explicit (convert if it is WrtCore
      // now). This way later we can test for WrtParent easily.
      p.parent->core_as_change.ensure_explicit(*p.core_as_change);
   }

   int i;

   if (node.tm_storage.var_change == BCP_Storage_WrtParent){
      // these are the vars present in the parent
      p.unpack_var_set_change(parent_added_vars);
#ifdef PARANOID
      if (parent_added_vars.storage() != BCP_Storage_Explicit)
	 throw BCP_fatal_error("BCP_lp_unpack_parent(): oops 1\n");
#endif
      const int pvarnum = parent_added_vars._change.size();
      p.parent->added_vars_desc = parent_added_vars._change;
      BCP_vec<int>& index = p.parent->added_vars_index;
      index.reserve(pvarnum);
      for (i = 0; i < pvarnum; ++i)
	 index.unchecked_push_back(parent_added_vars._new_vars[i]->bcpind());
   }

   if (node.tm_storage.cut_change == BCP_Storage_WrtParent){
      p.unpack_cut_set_change(parent_added_cuts);
#ifdef PARANOID
      if (parent_added_cuts.storage() != BCP_Storage_Explicit)
	 throw BCP_fatal_error("BCP_lp_unpack_parent(): oops 2\n");
#endif
      const int pcutnum = parent_added_cuts._change.size();
      p.parent->added_cuts_desc = parent_added_cuts._change;
      BCP_vec<int>& index = p.parent->added_cuts_index;
      index.reserve(pcutnum);
      for (i = 0; i < pcutnum; ++i)
	 index.unchecked_push_back(parent_added_cuts._new_cuts[i]->bcpind());
   }

   if (node.tm_storage.indexed_pricing == BCP_Storage_WrtParent)
      p.parent->indexed_pricing.unpack(buf);

   if (node.tm_storage.warmstart == BCP_Storage_WrtParent) {
      p.parent->warmstart = p.user->unpack_warmstart(buf);
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

void BCP_lp_create_added_vars(BCP_lp_prob& p, BCP_node_change& node_change,
			      BCP_var_set_change& parent_added_vars)
   // When this function is called, the core vars are already listed in
   // p.node->vars, be careful not to destroy them
{
   switch (p.node->tm_storage.var_change){
    case BCP_Storage_WrtParent:
    case BCP_Storage_Explicit:
      {
	 // modify the set of added vars in the parent then add the result to
	 // p.node->vars
	 // It happens so that in case of explicit storage parent_added_vars is
	 // an empty vector, so this is still OK!!
	 parent_added_vars.update(node_change.var_change);
	 const BCP_vec<BCP_obj_change>& added_var_changes =
	    parent_added_vars._change;
	 BCP_vec<BCP_var*>& new_vars = parent_added_vars._new_vars;
	 for (int i = added_var_changes.size() - 1; i >= 0; --i)
	    new_vars[i]->change_lb_ub_st(added_var_changes[i]);
	 p.node->vars.append(parent_added_vars._new_vars);
	 // set the set of added vars to that in node_change.var_change
      }
      break;

    case BCP_Storage_NoData:
      // there are no added vars
      break;

    case BCP_Storage_WrtCore:
    default:
      // impossible
      throw BCP_fatal_error("BCP_lp_create_added_vars: Bad storage.\n");
   }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_added_cuts(BCP_lp_prob& p, BCP_node_change& node_change,
			      BCP_cut_set_change& parent_added_cuts)
{
   switch (p.node->tm_storage.cut_change){
    case BCP_Storage_WrtParent:
    case BCP_Storage_Explicit:
      // comment like above
      {
	 // modify the set of added vars in the parent then add the result to
	 // p.node->vars
	 // It happens so that in case of explicit storage parent_added_vars is
	 // an empty vector, so this is still OK!!
	 parent_added_cuts.update(node_change.cut_change);
	 const BCP_vec<BCP_obj_change>& added_cut_changes =
	    parent_added_cuts._change;
	 BCP_vec<BCP_cut*>& new_cuts = parent_added_cuts._new_cuts;
	 for (int i = added_cut_changes.size() - 1; i >= 0; --i)
	    new_cuts[i]->change_lb_ub_st(added_cut_changes[i]);
	 p.node->cuts.append(parent_added_cuts._new_cuts);
	 // set the set of added cuts to that in node_change.cut_change
      }
      break;

    case BCP_Storage_NoData:
      // there are no added cuts
      break;

    case BCP_Storage_WrtCore:
    default:
      // impossible
      throw BCP_fatal_error("BCP_lp_create_added_cuts: Bad storage.\n");
   }
}

//-----------------------------------------------------------------------------

void BCP_lp_create_indexed_pricing(BCP_lp_prob& p,
				   BCP_node_change& node_change)
{
   switch (p.node->tm_storage.indexed_pricing){
    case BCP_Storage_WrtParent:
      p.node->indexed_pricing = p.parent->indexed_pricing;
      p.node->indexed_pricing.update(node_change.indexed_pricing);
      break;

    case BCP_Storage_Explicit:
      p.node->indexed_pricing.swap(node_change.indexed_pricing);
      break;

    case BCP_Storage_NoData:   // there's no indexed to price
      break;

    case BCP_Storage_WrtCore:
    default:
      // impossible
      throw BCP_fatal_error("BCP_lp_create_indexed_pricing: Bad storage.\n");
   }
   
}

//-----------------------------------------------------------------------------

void BCP_lp_create_warmstart(BCP_lp_prob& p, BCP_node_change& node_change)
{
   switch (p.node->tm_storage.warmstart){
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

void BCP_lp_create_node(BCP_lp_prob& p, BCP_node_change& node_change,
			BCP_var_set_change& parent_added_vars,
			BCP_cut_set_change& parent_added_cuts)
{
   // we got to put together p.node from parent and node_change
   p.node->iteration_count = 0;

   BCP_lp_create_core(p, node_change);
   BCP_lp_create_added_vars(p, node_change, parent_added_vars);
   BCP_lp_create_added_cuts(p, node_change, parent_added_cuts);
   BCP_lp_create_indexed_pricing(p, node_change);
   BCP_lp_create_warmstart(p, node_change);
}

//#############################################################################

static inline void
BCP_lp_unpack_procid(BCP_message_environment* msg_env,
		     BCP_buffer& buf, BCP_proc_id*& pid)
{
   bool has_id;
   buf.unpack(has_id);
   pid = has_id ? msg_env->unpack_proc_id(buf) : 0;
}

//#############################################################################

void BCP_lp_unpack_active_node(BCP_lp_prob& p, BCP_buffer& buf)
{
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
   BCP_lp_unpack_procid(p.msg_env, buf, node.cg);
   BCP_lp_unpack_procid(p.msg_env, buf, node.cp);
   BCP_lp_unpack_procid(p.msg_env, buf, node.vg);
   BCP_lp_unpack_procid(p.msg_env, buf, node.vp);

   // unpack how the various pieces are stored in node
   buf.unpack(node.tm_storage.core_change)
      .unpack(node.tm_storage.var_change)
      .unpack(node.tm_storage.cut_change)
      .unpack(node.tm_storage.indexed_pricing)
      .unpack(node.tm_storage.warmstart);

   BCP_var_set_change parent_added_vars;
   BCP_cut_set_change parent_added_cuts;
   if (node.level > 0)
      BCP_lp_unpack_parent(p, buf, node, parent_added_vars, parent_added_cuts);

   BCP_node_change node_change;
   node_change.core_change.unpack(buf);
   p.unpack_var_set_change(node_change.var_change);
   p.unpack_cut_set_change(node_change.cut_change);
   node_change.indexed_pricing.unpack(buf);

   bool has_data;
   buf.unpack(has_data);
   node_change.warmstart = has_data ? p.user->unpack_warmstart(buf) : 0;

   // Create the active node from the parent and from the last changes
   BCP_lp_create_node(p, node_change, parent_added_vars, parent_added_cuts);

   // Delete the old user data
   delete p.node->user_data;

   buf.unpack(has_data);
   p.node->user_data = has_data ? p.user->unpack_user_data(buf) : 0;
}
