// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <map>

#include "BCP_lp.hpp"
#include "BCP_tm.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_solution.hpp"

#include "BCP_warmstart.hpp"

BCP_tm_prob::BCP_tm_prob() :
   user(0),
   msg_env(0),
   lp_stat(0),
   feas_sol(0),
   upper_bound(DBL_MAX),
   core(0),
   core_as_change(0),
   next_cut_index_set_start(1),
   next_var_index_set_start(1),
   candidates(*this)
{}

BCP_tm_prob::~BCP_tm_prob()
{
   delete user;

   delete lp_stat;
   
   delete feas_sol;

   std::map<BCP_IndexType, BCP_var*>::iterator vari = vars.begin();
   while (vari != vars.end()) {
      delete (vari++)->second;
   }
   std::map<BCP_IndexType, BCP_cut*>::iterator cuti = cuts.begin();
   while (cuti != cuts.end()) {
      delete (cuti++)->second;
   }

   delete core;
   delete core_as_change;
}

//#############################################################################

void
BCP_tm_prob::pack_var(BCP_process_t target_proc, const BCP_var& var)
{
  const BCP_IndexType bcpind = var.bcpind();
  const BCP_object_t obj_t = var.obj_type();
  const BCP_obj_status stat = var.status();
  const BCP_var_t var_t = var.var_type();
  const double obj = var.obj();
  const double lb = var.lb();
  const double ub = var.ub();
  msg_buf.pack(bcpind)
         .pack(obj_t).pack(stat).pack(var_t).pack(obj).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_IndexedObj:
    {
      const int index = (dynamic_cast<const BCP_var_indexed&>(var)).index();
      msg_buf.pack(index);
    }
    break;
  case BCP_AlgoObj:
    user->pack_var_algo(&dynamic_cast<const BCP_var_algo&>(var), msg_buf);
    break;
  default:
    throw BCP_fatal_error("BCP_tm_prob::_pack_var(): unexpected obj_t.\n");
  }
}

//-----------------------------------------------------------------------------

BCP_var* 
BCP_tm_prob::unpack_var_without_bcpind(BCP_buffer& buf)
{
  BCP_var* var = 0;
  BCP_object_t obj_t;
  BCP_var_t var_t;
  int index;
  double obj, lb, ub;
  BCP_obj_status stat;
  buf.unpack(obj_t).unpack(stat)
     .unpack(var_t).unpack(obj).unpack(lb).unpack(ub);
  switch (obj_t) {
   case BCP_CoreObj:
     var = new BCP_var_core(var_t, obj, lb, ub);
     break;
   case BCP_IndexedObj:
     buf.unpack(index);
     var = new BCP_var_indexed(index, var_t, obj, lb, ub);
     break;
   case BCP_AlgoObj:
     var = user->unpack_var_algo(buf);
     var->set_var_type(var_t);
     var->change_bounds(lb, ub);
     var->set_obj(obj);
     break;
   default:
     throw BCP_fatal_error("BCP_tm_prob::_unpack_var(): unexpected obj_t.\n");
  }
  var->set_status(stat);
  return var;
}

//-----------------------------------------------------------------------------

BCP_var* 
BCP_tm_prob::unpack_var()
{
  BCP_IndexType bcpind;
  BCP_var* var = 0;
  msg_buf.unpack(bcpind);
  if (bcpind == 0) {
    throw BCP_fatal_error("BCP_tm_prob::unpack_var(): 0 bcpind arrived.\n");
  }
  if (bcpind < 0) {
     var = unpack_var_without_bcpind(msg_buf);
     var->set_bcpind(bcpind);
     bcpind = -bcpind;
     if (vars.find(bcpind) == vars.end()) {
	var->set_bcpind(bcpind);
	vars[bcpind] = var;
     } else {
	delete var;
	var = vars[bcpind];
     }
  } else {
     var = vars[bcpind];
  }
  return var;
}

//#############################################################################

void
BCP_tm_prob::pack_cut(BCP_process_t target_proc, const BCP_cut& cut)
{
  const BCP_IndexType bcpind = cut.bcpind();
  const BCP_object_t obj_t = cut.obj_type();
  const BCP_obj_status stat = cut.status();
  const double lb = cut.lb();
  const double ub = cut.ub();
  msg_buf.pack(bcpind)
         .pack(obj_t).pack(stat).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_IndexedObj:
    {
      const int index = (dynamic_cast<const BCP_cut_indexed&>(cut)).index();
      msg_buf.pack(index);
    }
    break;
  case BCP_AlgoObj:
    user->pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), msg_buf);
    break;
  default:
    throw BCP_fatal_error("BCP_tm_prob::_pack_cut(): unexpected obj_t.\n");
  }
}

//-----------------------------------------------------------------------------

BCP_cut* 
BCP_tm_prob::unpack_cut()
{
  BCP_IndexType bcpind;
  BCP_cut* cut = 0;
  msg_buf.unpack(bcpind);
  if (bcpind == 0) {
    throw BCP_fatal_error("BCP_tm_prob::unpack_cut(): 0 bcpind arrived.\n");
  }
  if (bcpind < 0) {
    BCP_object_t obj_t;
    int index;
    double lb, ub;
    BCP_obj_status stat;
    msg_buf.unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);
    switch (obj_t) {
    case BCP_CoreObj:
      cut = new BCP_cut_core(lb, ub);
      break;
    case BCP_IndexedObj:
      msg_buf.unpack(index);
      cut = new BCP_cut_indexed(index, lb, ub);
      break;
    case BCP_AlgoObj:
      cut = user->unpack_cut_algo(msg_buf);
      cut->change_bounds(lb, ub);
      break;
    default:
      throw BCP_fatal_error("BCP_tm_prob::_unpack_cut(): unexpected obj_t.\n");
    }
    cut->set_bcpind(bcpind);
    cut->set_status(stat);

    bcpind = -bcpind;
    if (cuts.find(bcpind) == cuts.end()) {
      cut->set_bcpind(bcpind);
      cuts[bcpind] = cut;
    } else {
      delete cut;
      cut = cuts[bcpind];
    }
  } else {
    cut = cuts[bcpind];
  }
  return cut;
}

//#############################################################################

void
BCP_tm_prob::pack_var_set_change(const BCP_var_set_change& ch)
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
	pack_var(BCP_ProcessType_LP, **vari);
	++vari;
      }
    }
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_tm_prob::pack_var_set_change() : Bad storage.\n");
  }
}

//-----------------------------------------------------------------------------

void
BCP_tm_prob::unpack_var_set_change(BCP_var_set_change& ch)
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
BCP_tm_prob::unpack_var_set_change() : Bad storage.\n");
  }
}

//#############################################################################

void
BCP_tm_prob::pack_cut_set_change(const BCP_cut_set_change& ch)
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
	pack_cut(BCP_ProcessType_LP, **cuti);
	++cuti;
      }
    }
    break;

  case BCP_Storage_NoData:
    break;

  case BCP_Storage_WrtCore:
  default:
    throw BCP_fatal_error("\
BCP_tm_prob::pack_cut_set_change() : Bad storage.\n");
  }
}

//-----------------------------------------------------------------------------

void
BCP_tm_prob::unpack_cut_set_change(BCP_cut_set_change& ch)
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
BCP_tm_prob::unpack_cut_set_change() : Bad storage.\n");
  }
}
