// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_math.hpp"
#include "BCP_message.hpp"
#include "BCP_vector.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_vg.hpp"
#include "BCP_vg_user.hpp"

BCP_vg_prob::BCP_vg_prob(int my_id, int parent) :
   BCP_process(my_id, parent),
   user(0), msg_env(0), core(new BCP_problem_core),
   upper_bound(BCP_DBL_MAX), phase(0) {}


BCP_vg_prob::~BCP_vg_prob()
{
   delete user;   user = 0;
   delete packer; packer = 0;
   delete core;   core = 0;
   purge_ptr_vector(cuts);
}

bool
BCP_vg_prob::probe_messages()
{
   while (msg_env->probe(BCP_AnyProcess, BCP_Msg_UpperBound)) {
      msg_env->receive(BCP_AnyProcess, BCP_Msg_UpperBound, msg_buf, 0);
      double new_ub;
      msg_buf.unpack(new_ub);
      if (new_ub < upper_bound)
	 upper_bound = new_ub;
   }

   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForVG_DualNonzeros))
      return true;
   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForVG_DualFull))
      return true;
   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_ForVG_User))
      return true;

   if (msg_env->probe(BCP_AnyProcess, BCP_Msg_FinishedBCP))
      return true;
   return false;
}

BCP_cut* 
BCP_vg_prob::unpack_cut()
{
  BCP_object_t obj_t;
  int bcpind;
  double lb, ub;
  BCP_obj_status stat;
  msg_buf.unpack(bcpind)
         .unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);

  BCP_cut* cut = 0;
  switch (obj_t) {
  case BCP_CoreObj:
    cut = new BCP_cut_core(lb, ub);
    break;
  case BCP_AlgoObj:
    cut = packer->unpack_cut_algo(msg_buf);
    cut->change_bounds(lb, ub);
    break;
  default:
    throw BCP_fatal_error("BCP_vg_prob::_unpack_cut(): unexpected obj_t.\n");
  }
  cut->set_bcpind(bcpind);
  cut->set_status(stat);

  return cut;
}

