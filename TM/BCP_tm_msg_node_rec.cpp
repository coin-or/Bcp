// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdlib>
#include <algorithm>

#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_branch.hpp"
#include "BCP_enum_branch.hpp"
#include "BCP_message.hpp"
#include "BCP_temporary.hpp"
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

static int
BCP_tm_unpack_node_description(BCP_tm_prob& p, BCP_buffer& buf)
{
   // the first thing is the index of the node
   int index;
   buf.unpack(index);
   // get a pointer to this node
   BCP_tm_node* node = p.search_tree[index];

   // XXX
   if (node != p.active_nodes[p.slaves.lp->index_of_proc(node->lp)]) {
     throw BCP_fatal_error("\
BCP_tm_unpack_node_description: received node is different from processed.\n");
   }

   // get the quality and new lb for this node
   buf.unpack(node->_quality).unpack(node->_true_lower_bound);

   // wipe out any previous description of this node and create a new one if
   // the description is sent over
   delete node->_desc;   node->_desc = 0;
   bool desc_sent = false;
   buf.unpack(desc_sent);

   if (desc_sent) {
      BCP_node_change* desc = new BCP_node_change;
      node->_desc = desc;

      // unpack core_change
      if (p.core->varnum() + p.core->cutnum() > 0)
	 desc->core_change.unpack(buf);

      // get the variables and cuts. unpack takes care of checking
      // explicitness, and returns how many algo objects are there in ..._set
      // that do not yet have internal index.
      p.unpack_var_set_change(desc->var_change);
      p.unpack_cut_set_change(desc->cut_change);

      // pricing status
      desc->indexed_pricing.unpack(buf);

      // warmstart info
      bool has_data;
      buf.unpack(has_data);
      if (has_data)
	 desc->warmstart = p.user->unpack_warmstart(buf);
   }

   p.active_nodes[p.slaves.lp->index_of_proc(node->lp)] = 0;

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

   const double topq = p.candidates.top()->quality();

   if (quality <= topq)
      return BCP_TestBeforeDive;

   if (topq > 0) {
      if (quality / topq < ratio) return BCP_TestBeforeDive;
   } else if (topq == 0) {
      if (quality < ratio)        return BCP_TestBeforeDive;
   } else {
      if (quality < 0 &&
	  topq / quality < ratio) return BCP_TestBeforeDive;
   }

   return BCP_DoNotDive;
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

   BCP_temp_vec<double> tmp_lpobj;
   BCP_vec<double>& lpobj = tmp_lpobj.vec();
   BCP_temp_vec<double> tmp_qualities;
   BCP_vec<double>& qualities = tmp_qualities.vec();

   buf.unpack(dive).unpack(action).unpack(qualities).unpack(lpobj);
   BCP_internal_brobj* brobj = new BCP_internal_brobj;

   brobj->unpack(buf);

   // generate the children
   const int bvarnum = p.core->varnum();
   const int bcutnum = p.core->cutnum();
   node->child_num(brobj->child_num());
   int keep = -1;
   BCP_tm_node* child = 0;
   BCP_node_change* desc;
   BCP_node_change* nodedesc = node->_desc;
   int i;

   // fix the number of leaves assigned to the CP/VP
   if (node->cp) {
      BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	 BCP_tm_identify_process(p.leaves_per_cp, node->cp);
      if (proc == p.leaves_per_cp.end()) {
	 node->cp = 0; 
      } else {
	 proc->second += node->child_num() - 1;
      }
   }
   if (node->vp) {
      BCP_vec< std::pair<BCP_proc_id*, int> >::iterator proc =
	 BCP_tm_identify_process(p.leaves_per_vp, node->vp);
      if (proc == p.leaves_per_vp.end()) {
	 node->vp = 0; 
      } else {
	 proc->second += node->child_num() - 1;
      }
   }

   for (i = 0; i < brobj->child_num(); ++i){
      desc = new BCP_node_change;
      BCP_tm_create_core_change(desc, bvarnum, bcutnum,	brobj, i);
      BCP_tm_create_var_change(desc, nodedesc, bvarnum, brobj, i);
      BCP_tm_create_cut_change(desc, nodedesc, bcutnum, brobj, i);
      desc->indexed_pricing.set_status(nodedesc->indexed_pricing.get_status());
      if (nodedesc->indexed_pricing.get_status() & BCP_PriceIndexedVars)
         // in this case set storage the WrtParent (and there's no change)
         desc->indexed_pricing.empty(BCP_Storage_WrtParent);
      if (nodedesc->warmstart)
         // If the parent has warmstart info then 
         desc->warmstart = nodedesc->warmstart->empty_wrt_this();

      child = new BCP_tm_node(node->level() + 1, desc);
      p.search_tree.insert(child); // this sets _index
      child->_quality = qualities[i];
      child->_true_lower_bound =
	 p.param(BCP_tm_par::LpValueIsTrueLowerBound) ?
	 lpobj[i] : node->true_lower_bound();
      child->_parent = node;
      child->_birth_index = node->child_num();
      node->new_child(child);
      // _children  initialized to be empty -- OK
      switch (action[i]){
       case BCP_ReturnChild:
	 p.candidates.insert(child);
	 child->status = BCP_CandidateNode;
	 break;
       case BCP_KeepChild:
	 child->status = BCP_CandidateNode; // be conservative
	 keep = i;
	 break;
       case BCP_FathomChild:
	 child->status = BCP_PrunedNode_Discarded;
	 break;
      }
      // inherit var/cut pools
      child->vp = node->vp ? node->vp->clone() : 0;
      child->cp = node->cp ? node->cp->clone() : 0;
      // lp, cg, vg  initialized to 0 -- OK, none assigned yet
   }

   // check the one that's proposed to be kept if there's one
   if (keep >= 0){
      child = node->child(keep);
      if (dive == BCP_DoDive || dive == BCP_TestBeforeDive){
	 // we've got to answer
	 buf.clear();
	 if (dive == BCP_TestBeforeDive)
	    dive = BCP_tm_shall_we_dive(p, child->quality());
	 buf.pack(dive);
	 if (dive != BCP_DoNotDive){
	    child->status = BCP_ActiveNode;
	    // if diving then send the new index and var/cut_names
	    buf.pack(child->index());
	 } else {
	    p.candidates.insert(child);
	 }
	 p.msg_env->send(node->lp, BCP_Msg_DivingInfo, buf);
      }else{
	 p.candidates.insert(child);
      }
   }else{
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
      if (! node->lp) {
	throw BCP_fatal_error("\
BCP_tm_unpack_branching_info: the (old) node has no LP associated with!\n");
      }
      child->lp = node->lp;
      child->cg = node->cg;
      child->vg = node->vg;
      p.active_nodes[p.slaves.lp->index_of_proc(node->lp)] = child;
      node->lp = node->cg = node->vg = 0;
   }

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
   BCP_tm_unpack_branching_info(p, buf, p.search_tree[index]);
   // BCP_tm_list_candidates(p);
}

//#############################################################################

BCP_tm_node* BCP_tm_unpack_node_no_branching_info(BCP_tm_prob& p,
						  BCP_buffer& buf)
{
   const int index = BCP_tm_unpack_node_description(p, buf);

   // Mark the lp/cg/vg processes of the node as free
   BCP_tm_node* node = p.search_tree[index];
   BCP_tm_free_procs_of_node(p, node);

   return node;
}
