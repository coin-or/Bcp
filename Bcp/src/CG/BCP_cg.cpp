// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_message.hpp"
#include "BCP_vector.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_cg.hpp"
#include "BCP_cg_user.hpp"

#include "BCP_warmstart.hpp"

BCP_cg_prob::BCP_cg_prob(BCP_proc_id* my_id, BCP_proc_id* parent) :
    BCP_process(my_id, parent),
    user(0), msg_env(0), core(new BCP_problem_core),
    upper_bound(DBL_MAX), phase(0) {}


BCP_cg_prob::~BCP_cg_prob()
{
   delete user;   user = 0;
   delete core;   core = 0;
   purge_ptr_vector(vars);
   delete sender;
}

bool
BCP_cg_prob::probe_messages()
{
   while (msg_env->probe(BCP_AnyProcess, BCP_Msg_UpperBound)) {
      msg_env->receive(BCP_AnyProcess, BCP_Msg_UpperBound, msg_buf, 0);
      double new_ub;
      msg_buf.unpack(new_ub);
      if (new_ub < upper_bound)
	 upper_bound = new_ub;
   }

   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForCG_PrimalNonzeros))
      return true;
   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForCG_PrimalFractions))
      return true;
   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForCG_PrimalFull))
      return true;
   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForCG_User))
      return true;

   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_FinishedBCP))
      return true;
   return false;
}

BCP_var* 
BCP_cg_prob::unpack_var()
{
  BCP_object_t obj_t;
  int bcpind;
  BCP_var_t var_t;
  double obj, lb, ub;
  BCP_obj_status stat;
  msg_buf.unpack(bcpind)
         .unpack(obj_t).unpack(stat)
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
    throw BCP_fatal_error("BCP_cg_prob::_unpack_var(): unexpected obj_t.\n");
  }
  var->set_bcpind(bcpind);
  var->set_status(stat);

  return var;
}
