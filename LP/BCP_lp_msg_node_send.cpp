// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <functional>

#include "BCP_message.hpp"
#include "BCP_temporary.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_branch.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_functions.hpp"

#include "BCP_USER.hpp"

//#############################################################################
static inline void BCP_lp_pack_core(BCP_lp_prob& p);
static inline void BCP_lp_pack_noncore_vars(BCP_lp_prob& p,
					    BCP_vec<int>& deleted_pos);
static inline void BCP_lp_pack_noncore_cuts(BCP_lp_prob& p,
					    BCP_vec<int>& deleted_pos);
static inline void BCP_lp_pack_indexed_pricing(BCP_lp_prob& p);
static inline void BCP_lp_pack_warmstart(BCP_lp_prob& p,
					 BCP_vec<int>& del_vars,
					 BCP_vec<int>& del_cuts);

//-----------------------------------------------------------------------------

static inline int BCP_lp_pack_branching_info(BCP_lp_prob& p,
					     BCP_presolved_lp_brobj* brobj);

//#############################################################################

static inline void
BCP_lp_pack_core(BCP_lp_prob& p)
{
   if (p.core->varnum() + p.core->cutnum() > 0){
      BCP_problem_core_change exp_bc(p.core->varnum(), p.node->vars,
				 p.core->cutnum(), p.node->cuts);
      switch (p.node->tm_storage.core_change){
       case BCP_Storage_WrtCore:
	 {
	    BCP_problem_core_change wrtcore_bc(BCP_Storage_WrtCore,
					   *p.core_as_change, exp_bc);
	    wrtcore_bc.pack(p.msg_buf);
	 }
	 break;

       case BCP_Storage_Explicit:
	 exp_bc.pack(p.msg_buf);
	 break;

       case BCP_Storage_WrtParent:
	 {
	    BCP_problem_core_change wrtparent_bc(BCP_Storage_WrtParent,
					     p.parent->core_as_change, exp_bc);
	    BCP_problem_core_change wrtcore_bc(BCP_Storage_WrtCore,
					   *p.core_as_change, exp_bc);
	    if (exp_bc.pack_size() <= wrtparent_bc.pack_size()){
	       if (exp_bc.pack_size() <= wrtcore_bc.pack_size()){
		  exp_bc.pack(p.msg_buf);
	       }else{
		  wrtcore_bc.pack(p.msg_buf);
	       }
	    }else{
	       if (wrtparent_bc.pack_size() < wrtcore_bc.pack_size()){
		  wrtparent_bc.pack(p.msg_buf);
	       }else{
		  wrtcore_bc.pack(p.msg_buf);
	       }
	    }
	 }
	 break;

       default: // including BCP_Storage_NoData
	 throw BCP_fatal_error("BCP_lp_pack_core() : Bad storage.\n");
      }
      // look ahead (the current node may become the parent) and replace the
      // parent's core to be exp_bc
      p.parent->core_as_change.swap(exp_bc);
   }
}

//#############################################################################

static inline void
BCP_lp_pack_noncore_vars(BCP_lp_prob& p, BCP_vec<int>& deleted_pos)
{
   // create the BCP_var_set_change describing the current node.
   // must create a WrtParent listing as well as an Explicit listing since the
   // set pf deleted vars will be needed when the warmstart info is put
   // together.
   deleted_pos.clear();
   BCP_var_set_change desc(p.node->vars.entry(p.core->varnum()),
			   p.node->vars.end());

   if (p.node->tm_storage.var_change == BCP_Storage_WrtParent) {
      BCP_var_set_change change(p.node->vars.entry(p.core->varnum()),
				p.node->vars.end(),
				p.parent->added_vars_index,
				p.parent->added_vars_desc);

      deleted_pos.append(change._del_change_pos.begin(),
			 change._del_change_pos.entry(change.deleted_num()));

      // if the TM storage is WrtParent then pack the shorter
      if (desc.pack_size() > change.pack_size())
	 desc.swap(change);
   }

   // Now desc holds the right description (Explicit or WrtParent, who knows?)
   // Pack it.
   p.pack_var_set_change(desc);
}

//#############################################################################

static inline void
BCP_lp_pack_noncore_cuts(BCP_lp_prob& p, BCP_vec<int>& deleted_pos)
{
   // create the BCP_cut_set_change describing the current node.
   // must create a WrtParent listing as well as an Explicit listing since the
   // set pf deleted cuts will be needed when the warmstart info is put
   // together.
   deleted_pos.clear();
   BCP_cut_set_change desc(p.node->cuts.entry(p.core->cutnum()),
			   p.node->cuts.end());
   
   if (p.node->tm_storage.cut_change == BCP_Storage_WrtParent) {
      BCP_cut_set_change change(p.node->cuts.entry(p.core->cutnum()),
				p.node->cuts.end(),
				p.parent->added_cuts_index,
				p.parent->added_cuts_desc);

      deleted_pos.append(change._del_change_pos.begin(),
			 change._del_change_pos.entry(change.deleted_num()));

      // if the TM storage is WrtParent then pack the shorter
      if (desc.pack_size() > change.pack_size())
	 desc.swap(change);
   }

   // Now desc holds the right description (Explicit or WrtParent, who knows?)
   // Pack it.
   p.pack_cut_set_change(desc);
}

//#############################################################################

static inline void
BCP_lp_pack_indexed_pricing(BCP_lp_prob& p)
{
   BCP_indexed_pricing_list& indexed_pricing = p.node->indexed_pricing;
   if (p.node->tm_storage.indexed_pricing != BCP_Storage_WrtParent) {
      // if ! BCP_PriceIndexedVars then storage will be NoData, so OK.
      indexed_pricing.pack(p.msg_buf);
   } else {
      BCP_indexed_pricing_list* pricing_change =
	 indexed_pricing.as_change(p.parent->indexed_pricing);
      if (pricing_change->pack_size() < indexed_pricing.pack_size())
	 pricing_change->pack(p.msg_buf);
      else
	 indexed_pricing.pack(p.msg_buf);
      delete pricing_change;   pricing_change = 0;
   }
}

//#############################################################################

static inline void
BCP_lp_pack_warmstart(BCP_lp_prob& p,
		      BCP_vec<int>& del_vars, BCP_vec<int>& del_cuts)
{
   bool has_data = p.node->warmstart != 0;
   p.msg_buf.pack(has_data);

   if (has_data) {
      if (p.node->tm_storage.warmstart != BCP_Storage_WrtParent) {
	p.user->pack_warmstart(p.node->warmstart, p.msg_buf);
      } else {
	double petol = 0.0;
	double detol = 0.0;
	p.lp_solver->getDblParam(OsiPrimalTolerance, petol);
	p.lp_solver->getDblParam(OsiDualTolerance, detol);
	// this return an explicit storage if that's shorter!
	BCP_warmstart* ws_change =
	  p.node->warmstart->as_change(p.parent->warmstart,
				       del_vars, del_cuts, petol, detol);
	p.user->pack_warmstart(ws_change, p.msg_buf);
	delete ws_change;
	ws_change = 0;
      }
   }      
}

//#############################################################################

static inline int
BCP_lp_pack_branching_info(BCP_lp_prob& p, BCP_presolved_lp_brobj* lp_brobj)
{
   const int child_num = lp_brobj->candidate()->child_num;

   // collect the lower bounds on the children
   BCP_temp_vec<double> tmp_lpobj(child_num);
   BCP_vec<double>& lpobj = tmp_lpobj.vec();
   for (int i = 0; i < child_num; ++i) {
      lpobj.unchecked_push_back(lp_brobj->lpres(i).objval());
   }

   // The qualities are the same (for now) as the lpobjs
   BCP_temp_vec<double> tmp_qualities(lpobj);
   BCP_vec<double>& qualities = tmp_qualities.vec();

   const BCP_vec<BCP_child_action>& action = lp_brobj->action();

   // now pack all those stuff
   BCP_buffer& buf = p.msg_buf;
   buf.pack(p.node->dive).pack(action).pack(qualities).pack(lpobj);
   BCP_internal_brobj int_brobj(*lp_brobj->candidate());
   int_brobj.pack(buf);

   int keep = -1;
   if (p.node->dive != BCP_DoNotDive){
      for (int i = child_num - 1; i >= 0; --i)
	 if (action[i] == BCP_KeepChild)
	    if (keep == -1)
	       keep = i;
	    else
	       throw BCP_fatal_error("LP : Can't keep more than one child!\n");
   }
   return keep;
}

//#############################################################################

// brobj is 0, msgtag is 'real' when invoked from fathom().
// brobj is 'real', msgtag is BCP_Msg_NoMessage when invoked from branch()

int BCP_lp_send_node_description(BCP_lp_prob& p,
				 BCP_presolved_lp_brobj* brobj,
				 BCP_message_tag msgtag)
{
   BCP_buffer& buf = p.msg_buf;
   BCP_lp_node& node = *p.node;

   // let's start with saying who this node is and what is the lb we got
   buf.clear();
   buf.pack(node.index).pack(node.quality).pack(node.true_lower_bound);

   // Send the node description only if this node is branched on (i.e., brobj
   // is non-null) or we got to send the description of fathomed nodes, too.
   if (brobj || p.param(BCP_lp_par::SendFathomedNodeDesc)) {
      // Pack the core (WrtCore, WrtParent or Explicit)
      BCP_lp_pack_core(p);  // BCP_problem_core_change
      // pack the indexed/algo var set change (or pack them explicitly)
      BCP_temp_vec<int> tmp_del_vars;
      BCP_vec<int>& del_vars = tmp_del_vars.vec();
      BCP_lp_pack_noncore_vars(p, del_vars);

      BCP_temp_vec<int> tmp_del_cuts;
      BCP_vec<int>& del_cuts = tmp_del_cuts.vec();
      BCP_lp_pack_noncore_cuts(p, del_cuts);

      // pack the pricing status
      BCP_lp_pack_indexed_pricing(p);

      // At this point there aren't supposed to be any ws info. It was deleted
      // when the lp formulation was created. Test this.
      if (p.node->warmstart) {
	 throw BCP_fatal_error("\
LP: there is ws info in BCP_lp_send_node_description()!\n");
      }
      // get and pack the warmstart info
      OsiWarmStart* ws = p.lp_solver->getWarmStart();
      p.node->warmstart = BCP_lp_convert_OsiWarmStart(p, ws);
      BCP_lp_pack_warmstart(p, del_vars, del_cuts);
   }

   int keep = -1;

   if (brobj) {
      // we came here from branch()
      // pack the branching info, 'keep' will tell whether we wish to dive
      keep = BCP_lp_pack_branching_info(p, brobj);
      // In a single process environment (message driven) the reaction in the
      // TM to the send below will reset p.node->dive. In a multi-process
      // environment p.node->dive will remain Unknown. This will help later
      // (20 lines below) to decide whether we have to get the diving info or
      // not.
      p.node->dive = BCP_UnknownDivingStatus;
      p.msg_env->send(p.tree_manager,
		      BCP_Msg_NodeDescriptionWithBranchingInfo, buf);
   }else{
      // we came from fathom()
      p.msg_env->send(p.tree_manager, msgtag, buf);
   }

   if (keep == -1){
      // we don't wan't to dive (or we came from fathom()),
      // don't wait for the names of the ones not having global internal index
      return -1;
   }

   // We did want to dive

   // In the single process environment the diving info already came back
   // (when the TM processes the branching info) and p.node->dive is set.
   // Otherwise we got to receive the diving info here.
   if (p.node->dive == BCP_UnknownDivingStatus) {
      // We got to receive the diving information by hand
      p.msg_buf.clear();
      p.msg_env->receive(p.tree_manager, BCP_Msg_DivingInfo, buf, -1);
      BCP_lp_unpack_diving_info(p, p.msg_buf);
   }

   // BCP_lp_unpack_diving_info() sets p.node->index to the new index if
   // diving is to be done, or to -1 if diving is not allowed.
   if (p.node->index == -1) {
      keep = -1;
      // At this point brobj cannot be empty.
      // We must reset the child to be kept, too.
      brobj->keep_no_child();
   }
   return keep;
}

//#############################################################################
