// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <functional>

#include "BCP_message.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_branch.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_vector.hpp"

#include "BCP_USER.hpp"

//#############################################################################
static inline void BCP_lp_pack_core(BCP_lp_prob& p);
static inline void BCP_lp_pack_noncore_vars(BCP_lp_prob& p,
					    BCP_vec<int>& deleted_pos);
static inline void BCP_lp_pack_noncore_cuts(BCP_lp_prob& p,
					    BCP_vec<int>& deleted_pos);
static inline void BCP_lp_pack_warmstart(BCP_lp_prob& p,
					 BCP_vec<int>& del_vars,
					 BCP_vec<int>& del_cuts);
static inline void BCP_lp_pack_user_data(BCP_lp_prob& p);

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
    // No matter whether we'll use an explicit description or WrtParent, all
    // vars with negative bcpind will have to be sent to the TM. These vars
    // will be collected in this vector.
    BCP_vec<BCP_var*> vars_to_tm;
    
    // First create an explicit description
    BCP_obj_set_change expl;
    
    BCP_var** vars = &p.node->vars[p.core->varnum()];
    const int new_added_num = p.node->vars.size() - p.core->varnum();
    if (new_added_num > 0) {
	expl._new_objs.reserve(new_added_num);
	expl._change.reserve(new_added_num);
	for (int i = 0; i < new_added_num; ++i) {
	    BCP_var* v = vars[i];
	    const int bcpind = v->bcpind();
	    // while we make the explicit description make sure that all extra
	    // vars have positive bcpind. After all, they are being sent to
	    // the TM right now.
	    if (bcpind < 0) {
		vars_to_tm.push_back(v);
		expl._new_objs.unchecked_push_back(-bcpind);
	    } else {
		expl._new_objs.unchecked_push_back(bcpind);
	    }
	    expl._change.unchecked_push_back(BCP_obj_change(v->lb(), v->ub(),
							    v->status()));
	}
    }
    
    // Now create a WrtParent description and see which one is shorter. Also,
    // we'll need the list of deleted variable positions when we set of the
    // warmstart information to be sent.
    BCP_obj_set_change wrtp;
    wrtp._storage = BCP_Storage_WrtParent;

    const BCP_vec<int>& old_added_bcpind = p.parent->var_set._new_objs;
    const BCP_vec<BCP_obj_change>& old_added_desc = p.parent->var_set._change;
    const int old_added_num = old_added_bcpind.size();
    wrtp._del_change_pos.reserve(old_added_num);

    BCP_vec<int> chpos;
    chpos.reserve(new_added_num);

    int i, j;

    // first check how many entry has been deleted from oldvars
    for (i = 0, j = 0; i < new_added_num && j < old_added_num; ++j) {
	const BCP_var* const v = vars[i];
	const BCP_obj_change& old = old_added_desc[j];
	if (v->bcpind() == old_added_bcpind[j]) {
	    // added_bcpind ALWAYS has real indices, so this really separates
	    if (v->lb()!=old.lb || v->ub()!=old.ub || v->status()!=old.stat)
		chpos.unchecked_push_back(i);
	    ++i;
	} else {
	    wrtp._del_change_pos.unchecked_push_back(j);
	}
    }
    // append the remains of old_added to _del_change_pos
    for ( ; j < old_added_num; ++j) {
	wrtp._del_change_pos.unchecked_push_back(j);
    }
    // _deleted_num is the current length of _del_change_pos
    wrtp._deleted_num = wrtp._del_change_pos.size();

    // the rest are the set of really new vars, and also the position of those
    // vars must be appended to chpos.
    wrtp._new_objs.reserve(new_added_num - i);
    for ( ; i < new_added_num; ++i){
	const int bcpind = vars[i]->bcpind();
	wrtp._new_objs.unchecked_push_back(bcpind > 0 ? bcpind : -bcpind);
	chpos.unchecked_push_back(i);
    }
    // append chpos to _del_change_pos to get the final list
    wrtp._del_change_pos.append(chpos);
    
    // finally, create _change: just pick up things based on chpos
    const int chnum = chpos.size();
    wrtp._change.reserve(chnum);
    for (i = 0; i < chnum; ++i) {
	const BCP_var* const var = vars[chpos[i]];
	wrtp._change.unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
							var->status()));
    }
   
    deleted_pos.clear();
    deleted_pos.append(wrtp._del_change_pos.begin(),
		       wrtp._del_change_pos.entry(wrtp.deleted_num()));

    // Whether we'll pack the Explicit or the WrtParent description, the new
    // vars need to be packed. Pack them first, so by the time the positive
    // bcpind in expl (or wrtp) arrives to the TM, the var already exists in
    // the TM.
    int num = vars_to_tm.size();
    p.msg_buf.pack(num);
    for (i = 0; i < num; ++i) {
	assert(vars_to_tm[i]->bcpind() < 0);
	p.pack_var(*vars_to_tm[i]);
	vars_to_tm[i]->set_bcpind_flip();
    }

    // if the TM storage is WrtParent then pack the shorter
    // FIXME: why only if TM storage is WrtParent ???
    if ((p.node->tm_storage.var_change == BCP_Storage_WrtParent) &&
	(expl.pack_size() > wrtp.pack_size())) {
	wrtp.pack(p.msg_buf);
    } else {
	expl.pack(p.msg_buf);
    }
}

//#############################################################################

static inline void
BCP_lp_pack_noncore_cuts(BCP_lp_prob& p, BCP_vec<int>& deleted_pos)
{
    // No matter whether we'll use an explicit description or WrtParent, all
    // cuts with negative bcpind will have to be sent to the TM. These cuts
    // will be collected in this vector.
    BCP_vec<BCP_cut*> cuts_to_tm;
    
    // First create an explicit description
    BCP_obj_set_change expl;
    
    BCP_cut** cuts = &p.node->cuts[p.core->cutnum()];
    const int new_added_num = p.node->cuts.size() - p.core->cutnum();
    if (new_added_num > 0) {
	expl._new_objs.reserve(new_added_num);
	expl._change.reserve(new_added_num);
	for (int i = 0; i < new_added_num; ++i) {
	    BCP_cut* c = cuts[i];
	    const int bcpind = c->bcpind();
	    // while we make the explicit description make sure that all extra
	    // cuts have positive bcpind. After all, they are being sent to
	    // the TM right now.
	    if (bcpind < 0) {
		cuts_to_tm.push_back(c);
		expl._new_objs.unchecked_push_back(-bcpind);
	    } else {
		expl._new_objs.unchecked_push_back(bcpind);
	    }
	    expl._change.unchecked_push_back(BCP_obj_change(c->lb(), c->ub(),
							    c->status()));
	}
    }
    
    // Now create a WrtParent description and see which one is shorter. Also,
    // we'll need the list of deleted cutiable positions when we set of the
    // warmstart information to be sent.
    BCP_obj_set_change wrtp;
    wrtp._storage = BCP_Storage_WrtParent;

    const BCP_vec<int>& old_added_bcpind = p.parent->cut_set._new_objs;
    const BCP_vec<BCP_obj_change>& old_added_desc = p.parent->cut_set._change;
    const int old_added_num = old_added_bcpind.size();
    wrtp._del_change_pos.reserve(old_added_num);

    BCP_vec<int> chpos;
    chpos.reserve(new_added_num);

    int i, j;

    // first check how many entry has been deleted from oldcuts
    for (i = 0, j = 0; i < new_added_num && j < old_added_num; ++j) {
	const BCP_cut* const c = cuts[i];
	const BCP_obj_change& old = old_added_desc[j];
	if (c->bcpind() == old_added_bcpind[j]) {
	    // added_bcpind ALWAYS has real indices, so this really separates
	    if (c->lb()!=old.lb || c->ub()!=old.ub || c->status()!=old.stat)
		chpos.unchecked_push_back(i);
	    ++i;
	} else {
	    wrtp._del_change_pos.unchecked_push_back(j);
	}
    }
    // append the remains of old_added to _del_change_pos
    for ( ; j < old_added_num; ++j) {
	wrtp._del_change_pos.unchecked_push_back(j);
    }
    // _deleted_num is the current length of _del_change_pos
    wrtp._deleted_num = wrtp._del_change_pos.size();

    // the rest are the set of really new cuts, and also the position of those
    // cuts must be appended to chpos.
    wrtp._new_objs.reserve(new_added_num - i);
    for ( ; i < new_added_num; ++i){
	const int bcpind = cuts[i]->bcpind();
	wrtp._new_objs.unchecked_push_back(bcpind > 0 ? bcpind : -bcpind);
	chpos.unchecked_push_back(i);
    }
    // append chpos to _del_change_pos to get the final list
    wrtp._del_change_pos.append(chpos);
    
    // finally, create _change: just pick up things based on chpos
    const int chnum = chpos.size();
    wrtp._change.reserve(chnum);
    for (i = 0; i < chnum; ++i) {
	const BCP_cut* const cut = cuts[chpos[i]];
	wrtp._change.unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
							cut->status()));
    }
   
    deleted_pos.clear();
    deleted_pos.append(wrtp._del_change_pos.begin(),
		       wrtp._del_change_pos.entry(wrtp.deleted_num()));

    // Whether we'll pack the Explicit or the WrtParent description, the new
    // cuts need to be packed. Pack them first, so by the time the positive
    // bcpind in expl (or wrtp) arrives to the TM, the cut already exists in
    // the TM.
    int num = cuts_to_tm.size();
    p.msg_buf.pack(num);
    for (i = 0; i < num; ++i) {
	assert(cuts_to_tm[i]->bcpind() < 0);
	p.pack_cut(*cuts_to_tm[i]);
	cuts_to_tm[i]->set_bcpind_flip();
    }

    // if the TM storage is WrtParent then pack the shorter
    // FIXME: why only if TM storage is WrtParent ???
    if ((p.node->tm_storage.cut_change == BCP_Storage_WrtParent) &&
	(expl.pack_size() > wrtp.pack_size())) {
	wrtp.pack(p.msg_buf);
    } else {
	expl.pack(p.msg_buf);
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
      const bool def = p.param(BCP_lp_par::ReportWhenDefaultIsExecuted);
      if (p.node->tm_storage.warmstart != BCP_Storage_WrtParent) {
	  p.packer->pack_warmstart(p.node->warmstart, p.msg_buf, def);
      } else {
	  double petol = 0.0;
	  double detol = 0.0;
	  p.lp_solver->getDblParam(OsiPrimalTolerance, petol);
	  p.lp_solver->getDblParam(OsiDualTolerance, detol);
	  // this return an explicit storage if that's shorter!
	  BCP_warmstart* ws_change =
	      p.node->warmstart->as_change(p.parent->warmstart,
					   del_vars, del_cuts, petol, detol);
	  p.packer->pack_warmstart(ws_change, p.msg_buf, def);
	  delete ws_change;
	  ws_change = 0;
      }
   }      
}

//#############################################################################

static inline void BCP_lp_pack_user_data(BCP_lp_prob& p)
{
   bool has_data = p.node->user_data != 0;
   p.msg_buf.pack(has_data);
   if (has_data) {
      p.packer->pack_user_data(p.node->user_data, p.msg_buf);
   }
}

//#############################################################################

static inline int
BCP_lp_pack_branching_info(BCP_lp_prob& p, BCP_presolved_lp_brobj* lp_brobj)
{
   const int child_num = lp_brobj->candidate()->child_num;

   // collect the lower bounds on the children
   BCP_vec<double> lpobj;
   lpobj.reserve(child_num);
   for (int i = 0; i < child_num; ++i) {
      lpobj.unchecked_push_back(lp_brobj->lpres(i).objval());
   }

   // The qualities are the same (for now) as the lpobjs
   BCP_vec<double> qualities(lpobj);

   const BCP_vec<BCP_child_action>& action = lp_brobj->action();
   const BCP_vec<BCP_user_data*>& user_data = lp_brobj->user_data();

   // now pack all those stuff
   BCP_buffer& buf = p.msg_buf;
   buf.pack(p.node->dive).pack(action).pack(qualities).pack(lpobj);

   for (int i = 0; i < child_num; ++i) {
     bool has_user_data = user_data[i] != 0;
     buf.pack(has_user_data);
     if (has_user_data) {
       p.packer->pack_user_data(user_data[i], buf);
     }
   }

   BCP_internal_brobj int_brobj(*lp_brobj->candidate());
   int_brobj.pack(buf);

   int keep = -1;
   if (p.node->dive != BCP_DoNotDive){
      for (int i = child_num - 1; i >= 0; --i)
	 if (action[i] == BCP_KeepChild) {
	    if (keep == -1)
	       keep = i;
	    else
	       throw BCP_fatal_error("LP : Can't keep more than one child!\n");
         }
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
   // is non-null) or if we got to send the description of fathomed nodes, too.
   const bool send_desc = brobj || p.param(BCP_lp_par::SendFathomedNodeDesc);
   buf.pack(send_desc);
   if (send_desc) {
      // Pack the core (WrtCore, WrtParent or Explicit)
      BCP_lp_pack_core(p);  // BCP_problem_core_change
      // pack the algo var set change (or pack them explicitly)
      BCP_vec<int> del_vars;
      BCP_lp_pack_noncore_vars(p, del_vars);

      BCP_vec<int> del_cuts;
      BCP_lp_pack_noncore_cuts(p, del_cuts);

      // At this point there aren't supposed to be any ws info. It was deleted
      // when the lp formulation was created. Test this.
      if (p.node->warmstart) {
	 throw BCP_fatal_error("\
LP: there is ws info in BCP_lp_send_node_description()!\n");
      }
      // If necessary, get and pack the warmstart info
      CoinWarmStart* ws = NULL;
      switch (p.param(BCP_lp_par::WarmstartInfo)) {
      case BCP_WarmstartNone:
	break;
      case BCP_WarmstartRoot:
	if (node.index == 0) { // we are in the root
	  ws = p.lp_solver->getWarmStart();
	  if (ws) {
	    BCP_warmstart* bws = BCP_lp_convert_CoinWarmStart(p, ws);
	    if (bws) {
	      const bool def = p.param(BCP_lp_par::ReportWhenDefaultIsExecuted);
	      BCP_buffer wsbuf;
	      p.packer->pack_warmstart(bws, wsbuf, def);
	      p.msg_env->send(p.get_parent() /*tree_manager*/,
			      BCP_Msg_WarmstartRoot, wsbuf);
	      p.warmstartRoot = ws;
	      delete bws;
	    }
	  }
	}
	break;
      case BCP_WarmstartParent:
	ws = p.lp_solver->getWarmStart();
	p.node->warmstart = BCP_lp_convert_CoinWarmStart(p, ws);
	BCP_lp_pack_warmstart(p, del_vars, del_cuts);
	BCP_lp_pack_user_data(p);
      }
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
      p.msg_env->send(p.get_parent() /*tree_manager*/,
		      BCP_Msg_NodeDescriptionWithBranchingInfo, buf);
   }else{
      // we came from fathom()
      p.msg_env->send(p.get_parent() /*tree_manager*/, msgtag, buf);
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
      p.msg_env->receive(p.get_parent() /*tree_manager*/,
			 BCP_Msg_DivingInfo, buf, -1);
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
