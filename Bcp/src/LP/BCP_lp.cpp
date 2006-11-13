// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>

#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp.hpp"
#include "OsiSolverInterface.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"

#include "BCP_lp_user.hpp"

#include "BCP_warmstart.hpp"

//#############################################################################

void
BCP_lp_statistics::pack(BCP_buffer& buf)
{
   buf.pack(time_cut_generation)
      .pack(time_var_generation)
      .pack(time_heuristics)
      .pack(time_lp_solving)
      .pack(time_branching);
}
void
BCP_lp_statistics::unpack(BCP_buffer& buf)
{
   buf.unpack(time_cut_generation)
      .unpack(time_var_generation)
      .unpack(time_heuristics)
      .unpack(time_lp_solving)
      .unpack(time_branching);
}
void
BCP_lp_statistics::display() const
{
  printf("LP statistics:\n");
  printf("   time in cut generation  : %12.3f sec\n", time_cut_generation);
  printf("   time in var generation  : %12.3f sec\n", time_var_generation);
  printf("   time in heuristics      : %12.3f sec\n", time_heuristics);
  printf("   time in solving LPs     : %12.3f sec\n", time_lp_solving);
  printf("   time in strong branching: %12.3f sec\n", time_branching);
  printf("\n");
}
void
BCP_lp_statistics::add(const BCP_lp_statistics& stat)
{
  time_cut_generation += stat.time_cut_generation;
  time_var_generation += stat.time_var_generation;
  time_heuristics     += stat.time_heuristics    ;
  time_lp_solving     += stat.time_lp_solving    ;
  time_branching      += stat.time_branching     ;
}

//#############################################################################

BCP_lp_prob::BCP_lp_prob(BCP_proc_id* my_id, BCP_proc_id* parent) :
   BCP_process(my_id, parent),
   user(0),
   master_lp(0),
   lp_solver(0),
   msg_env(0),
   core(new BCP_problem_core),
   core_as_change(new BCP_problem_core_change),
   node(new BCP_lp_node),
   parent(new BCP_lp_parent),
   lp_result(new BCP_lp_result),
   var_bound_changes_since_logical_fixing(0),
   local_var_pool(new BCP_lp_var_pool),
   local_cut_pool(new BCP_lp_cut_pool),
   next_var_index(0),
   last_var_index(0),
   next_cut_index(0),
   last_cut_index(0),
   upper_bound(1e100),
   phase(0)
{}

BCP_lp_prob::~BCP_lp_prob() {

   delete user;
   delete master_lp;
   delete lp_solver;

   delete core;
   delete core_as_change;

   delete node;
   delete parent;

   delete lp_result;
   purge_ptr_vector(slack_pool);

   delete local_var_pool;
   delete local_cut_pool;

   // these are vectors of ptrs, but they need not be purged. they just point
   // into the appropriate fields of p.node->vars/cuts
   //    purge_ptr_vector(all_vars);
   //    purge_ptr_vector(all_cuts);

   // delete tree_manager; this pointer must NOT be deleted!!!
}

//=============================================================================

void
BCP_lp_prob::pack_var(BCP_process_t target_proc, const BCP_var& var)
{
  const int bcpind = var.bcpind();
  msg_buf.pack(bcpind);
  if (target_proc == BCP_ProcessType_TM && bcpind > 0)
    return;

  const BCP_object_t obj_t = var.obj_type();
  const BCP_obj_status varstat = var.status();
  const BCP_var_t var_t = var.var_type();
  const double obj = var.obj();
  const double lb = var.lb();
  const double ub = var.ub();
  msg_buf.pack(obj_t).pack(varstat).pack(var_t).pack(obj).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_AlgoObj:
    user->pack_var_algo(&dynamic_cast<const BCP_var_algo&>(var), msg_buf);
    break;
  default:
    throw BCP_fatal_error("BCP_lp_prob::_pack_var(): unexpected obj_t.\n");
  }
}

//-----------------------------------------------------------------------------

BCP_var* 
BCP_lp_prob::unpack_var()
{
  BCP_object_t obj_t;
  int bcpind;
  BCP_var_t var_t;
  double obj, lb, ub;
  BCP_obj_status varstat;
  msg_buf.unpack(bcpind)
         .unpack(obj_t).unpack(varstat)
         .unpack(var_t).unpack(obj).unpack(lb).unpack(ub);

  BCP_var* var = 0;
  switch (obj_t) {
  case BCP_CoreObj:
    var = new BCP_var_core(var_t, obj, lb, ub);
    break;
  case BCP_AlgoObj:
    var = user->unpack_var_algo(msg_buf);
    var->set_var_type(var_t);
    var->change_bounds(lb, ub);
    var->set_obj(obj);
    break;
  default:
    throw BCP_fatal_error("BCP_lp_prob::_unpack_var(): unexpected obj_t.\n");
  }
  var->set_bcpind(bcpind);
  var->set_status(varstat);

  return var;
}

//#############################################################################

void
BCP_lp_prob::pack_cut(BCP_process_t target_proc, const BCP_cut& cut)
{
  const int bcpind = cut.bcpind();
  msg_buf.pack(bcpind);
  if (target_proc == BCP_ProcessType_TM && bcpind > 0)
    return;

  const BCP_object_t obj_t = cut.obj_type();
  const BCP_obj_status cutstat = cut.status();
  const double lb = cut.lb();
  const double ub = cut.ub();
  msg_buf.pack(obj_t).pack(cutstat).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_AlgoObj:
    user->pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), msg_buf);
    break;
  default:
    throw BCP_fatal_error("BCP_lp_prob::_pack_cut(): unexpected obj_t.\n");
  }
}

//-----------------------------------------------------------------------------

BCP_cut* 
BCP_lp_prob::unpack_cut()
{
  BCP_object_t obj_t;
  int bcpind;
  double lb, ub;
  BCP_obj_status cutstat;
  msg_buf.unpack(bcpind)
         .unpack(obj_t).unpack(cutstat).unpack(lb).unpack(ub);

  BCP_cut* cut = 0;
  switch (obj_t) {
  case BCP_CoreObj:
    cut = new BCP_cut_core(lb, ub);
    break;
  case BCP_AlgoObj:
    cut = user->unpack_cut_algo(msg_buf);
    cut->change_bounds(lb, ub);
    break;
  default:
    throw BCP_fatal_error("BCP_lp_prob::_unpack_cut(): unexpected obj_t.\n");
  }
  cut->set_bcpind(bcpind);
  cut->set_status(cutstat);

  return cut;
}

//#############################################################################

void
BCP_lp_prob::pack_var_set_change(const BCP_var_set_change& ch)
{
  msg_buf.pack(ch._storage);
  switch (ch._storage){
  case BCP_Storage_WrtParent:
  case BCP_Storage_Explicit:
    msg_buf.pack(ch._deleted_num).pack(ch._del_change_pos)
           .pack(ch._change).pack(ch.added_num());
    if (ch.added_num() > 0) {
      BCP_vec<BCP_var*>::const_iterator vari = ch._new_vars.begin();
      BCP_vec<BCP_var*>::const_iterator lastvari = ch._new_vars.end();
      while (vari != lastvari) {
	pack_var(BCP_ProcessType_TM, **vari);
	++vari;
      }
    }
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_lp_prob::pack_var_set_change() : Bad storage.\n");
  }
}

//-----------------------------------------------------------------------------

void
BCP_lp_prob::unpack_var_set_change(BCP_var_set_change& ch)
{
  msg_buf.unpack(ch._storage);
  switch (ch._storage) {
  case BCP_Storage_WrtParent:
  case BCP_Storage_Explicit:
    int num;
    msg_buf.unpack(ch._deleted_num)
           .unpack(ch._del_change_pos)
	   .unpack(ch._change)
	   .unpack(num);
    purge_ptr_vector(ch._new_vars);
    ch._new_vars.reserve(num);
    for ( ; num > 0; --num)
      ch._new_vars.unchecked_push_back(unpack_var());
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_lp_prob::unpack_var_set_change() : Bad storage.\n");
  }
}

//#############################################################################

void
BCP_lp_prob::pack_cut_set_change(const BCP_cut_set_change& ch)
{
  msg_buf.pack(ch._storage);
  switch (ch._storage){
  case BCP_Storage_WrtParent:
  case BCP_Storage_Explicit:
    msg_buf.pack(ch._deleted_num).pack(ch._del_change_pos)
           .pack(ch._change).pack(ch.added_num());
    if (ch.added_num() > 0) {
      BCP_vec<BCP_cut*>::const_iterator cuti = ch._new_cuts.begin();
      BCP_vec<BCP_cut*>::const_iterator lastcuti = ch._new_cuts.end();
      while (cuti != lastcuti) {
	pack_cut(BCP_ProcessType_TM, **cuti);
	++cuti;
      }
    }
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_lp_prob::pack_cut_set_change() : Bad storage.\n");
  }
}

//-----------------------------------------------------------------------------

void
BCP_lp_prob::unpack_cut_set_change(BCP_cut_set_change& ch)
{
  msg_buf.unpack(ch._storage);
  switch (ch._storage) {
  case BCP_Storage_WrtParent:
  case BCP_Storage_Explicit:
    int num;
    msg_buf.unpack(ch._deleted_num)
           .unpack(ch._del_change_pos)
	   .unpack(ch._change)
	   .unpack(num);
    purge_ptr_vector(ch._new_cuts);
    ch._new_cuts.reserve(num);
    for ( ; num > 0; --num)
      ch._new_cuts.unchecked_push_back(unpack_cut());
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_lp_prob::unpack_cut_set_change() : Bad storage.\n");
  }
}
