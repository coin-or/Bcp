// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <map>

#include "BCP_math.hpp"
#include "BCP_process.hpp"
#include "BCP_lp.hpp"
#include "BCP_tm.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_solution.hpp"

#include "BCP_warmstart.hpp"

BCP_tm_prob::BCP_tm_prob() :
   BCP_process(0, -1),
   user(0),
   msg_env(0),
   lp_stat(0),
   feas_sol(0),
   upper_bound(BCP_DBL_MAX),
   core(0),
   core_as_change(0),
   next_cut_index_set_start(1),
   next_var_index_set_start(1),
   candidate_list()
{}

BCP_tm_prob::~BCP_tm_prob()
{
   delete user;
   delete packer;

   delete lp_stat;
   
   delete feas_sol;

   delete core;
   delete core_as_change;
}

//#############################################################################

void
BCP_tm_stat::print(bool final, double t)
{
  bool do_print = false;
  if (final) {
    printf("TM: final statistics:\n");
    do_print = true;
  } else {
    if (floor(t/1200) > cnt) {
	cnt = static_cast<int>(floor(t/1200));
	printf("TM: statistics at %12.6f:\n", t);
	do_print = true;
    }
  }
  if (do_print) {
    for (int i = 0; i <= num_lp; ++i) {
      if ((wait_time[i] > 5e-3) ||
          (numQueueLength[i]>0 && sumQueueLength[i]/numQueueLength[i] > 5)) {
        if (numQueueLength[i] > 0) {
          printf("TM:    With %5i LP working:  wait: %12.6f   queue: %.2f\n",
		 i, wait_time[i], sumQueueLength[i]/numQueueLength[i]);
	} else {
	  printf("TM:    With %5i LP working:  wait: %12.6f   queue: %.2f\n",
		 i, wait_time[i], 0.0);
	}
      }
    }
  }
}

//#############################################################################

void
BCP_tm_prob::pack_var(const BCP_var& var)
{
  const int bcpind = var.bcpind();
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
  case BCP_AlgoObj:
    packer->pack_var_algo(&dynamic_cast<const BCP_var_algo&>(var), msg_buf);
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
  double obj, lb, ub;
  BCP_obj_status stat;
  buf.unpack(obj_t).unpack(stat)
     .unpack(var_t).unpack(obj).unpack(lb).unpack(ub);
  switch (obj_t) {
   case BCP_CoreObj:
     var = new BCP_var_core(var_t, obj, lb, ub);
     break;
   case BCP_AlgoObj:
     var = packer->unpack_var_algo(buf);
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

int
BCP_tm_prob::unpack_var()
{
    int bcpind;
    BCP_var* var = 0;
    msg_buf.unpack(bcpind);
    if (bcpind == 0) {
	throw BCP_fatal_error("BCP_tm_prob::unpack_var(): 0 bcpind.\n");
    }
    if (bcpind > 0) {
	if (vars_local.find(bcpind) != vars_local.end() ||
	    vars_remote.find(bcpind) != vars_remote.end() ) {
	    throw BCP_fatal_error("\
BCP_tm_prob::unpack_var(): received a var with positive bcpind, \n\
                           but the var already exists.\n");
	}
    } else {
	var = unpack_var_without_bcpind(msg_buf);
	if (vars_local.find(-bcpind) != vars_local.end() ||
	    vars_remote.find(-bcpind) != vars_remote.end() ) {
	    // It's OK, we got it from some other place. Nothing to do.
	    delete var;
	} else {
	    var->set_bcpind(-bcpind);
	    vars_local[-bcpind] = var;
	}
    }
    return bcpind;
}

//#############################################################################

void
BCP_tm_prob::pack_cut(const BCP_cut& cut)
{
  const int bcpind = cut.bcpind();
  const BCP_object_t obj_t = cut.obj_type();
  const BCP_obj_status stat = cut.status();
  const double lb = cut.lb();
  const double ub = cut.ub();
  msg_buf.pack(bcpind)
         .pack(obj_t).pack(stat).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_AlgoObj:
      packer->pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), msg_buf);
    break;
  default:
    throw BCP_fatal_error("BCP_tm_prob::_pack_cut(): unexpected obj_t.\n");
  }
}

//-----------------------------------------------------------------------------

BCP_cut* 
BCP_tm_prob::unpack_cut_without_bcpind(BCP_buffer& buf)
{
  BCP_cut* cut = 0;
  BCP_object_t obj_t;
  double lb, ub;
  BCP_obj_status stat;
  buf.unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);
  switch (obj_t) {
   case BCP_CoreObj:
     cut = new BCP_cut_core(lb, ub);
     break;
   case BCP_AlgoObj:
     cut = packer->unpack_cut_algo(buf);
     cut->change_bounds(lb, ub);
     break;
   default:
     throw BCP_fatal_error("BCP_tm_prob::_unpack_cut(): unexpected obj_t.\n");
  }
  cut->set_status(stat);
  return cut;
}

//-----------------------------------------------------------------------------

int
BCP_tm_prob::unpack_cut()
{
    int bcpind;
    BCP_cut* cut = 0;
    msg_buf.unpack(bcpind);
    if (bcpind == 0) {
	throw BCP_fatal_error("BCP_tm_prob::unpack_cut(): 0 bcpind.\n");
    }
    if (bcpind > 0) {
	if (cuts_local.find(bcpind) != cuts_local.end() ||
	    cuts_remote.find(bcpind) != cuts_remote.end() ) {
	    throw BCP_fatal_error("\
BCP_tm_prob::unpack_cut(): received a cut with positive bcpind, \n\
                           but the cut already exists.\n");
	}
    } else {
	cut = unpack_cut_without_bcpind(msg_buf);
	if (cuts_local.find(-bcpind) != cuts_local.end() ||
	    cuts_remote.find(-bcpind) != cuts_remote.end() ) {
	    // It's OK, we got it from some other place. Nothing to do.
	    delete cut;
	} else {
	    cut->set_bcpind(-bcpind);
	    cuts_local[-bcpind] = cut;
	}
    }
    return bcpind;
}
