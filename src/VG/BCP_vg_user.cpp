// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cassert>
#include "BCP_message_tag.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"

#include "BCP_vg.hpp"
#include "BCP_vg_user.hpp"

//#############################################################################
// Informational methods for the user
double BCP_vg_user::upper_bound() const    { return p->ub(); }
int BCP_vg_user::current_phase() const     { return p->phase; }
int BCP_vg_user::current_level() const     { return p->node_level; }
int BCP_vg_user::current_index() const     { return p->node_index; }
int BCP_vg_user::current_iteration() const { return p->node_iteration; }

//#############################################################################
// Informational methods for the user
/* Methods to get/set BCP parameters on the fly */
char
BCP_vg_user::get_param(const BCP_vg_par::chr_params key) const
{ return p->par.entry(key); }
int
BCP_vg_user::get_param(const BCP_vg_par::int_params key) const
{ return p->par.entry(key); }
double
BCP_vg_user::get_param(const BCP_vg_par::dbl_params key) const
{ return p->par.entry(key); }
const BCP_string&
BCP_vg_user::get_param(const BCP_vg_par::str_params key) const
{ return p->par.entry(key); }

void BCP_vg_user::set_param(const BCP_vg_par::chr_params key, const bool val)
{ p->par.set_entry(key, val); }
void BCP_vg_user::set_param(const BCP_vg_par::chr_params key, const char val)
{ p->par.set_entry(key, val); }
void BCP_vg_user::set_param(const BCP_vg_par::int_params key, const int val)
{ p->par.set_entry(key, val); }
void BCP_vg_user::set_param(const BCP_vg_par::dbl_params key, const double val)
{ p->par.set_entry(key, val); }
void BCP_vg_user::set_param(const BCP_vg_par::str_params key, const char * val)
{ p->par.set_entry(key, val); }

//#############################################################################

void
BCP_vg_user::send_var(const BCP_var& var)
{
  BCP_buffer& buf = p->msg_buf;
  buf.clear();
  const int bcpind = var.bcpind();
  const BCP_object_t obj_t = var.obj_type();
  const BCP_obj_status stat = var.status();
  const BCP_var_t var_t = var.var_type();
  const double obj = var.obj();
  const double lb = var.lb();
  const double ub = var.ub();
  buf.pack(bcpind)
     .pack(obj_t).pack(stat).pack(var_t).pack(obj).pack(lb).pack(ub);
  assert(obj_t == BCP_AlgoObj);
  p->packer->pack_var_algo(&dynamic_cast<const BCP_var_algo&>(var), buf);
  p->msg_env->send(p->sender, BCP_Msg_VarDescription, buf);
}

//#############################################################################

void
BCP_vg_user::unpack_module_data(BCP_buffer& buf)
{
  if (p->par.entry(BCP_vg_par::ReportWhenDefaultIsExecuted)) {
    printf(" VG: Default unpack_module_data() executed.\n");
  }
}

//#############################################################################

void
BCP_vg_user::unpack_dual_solution(BCP_buffer& buf)
{
  if (p->par.entry(BCP_vg_par::ReportWhenDefaultIsExecuted)) {
    printf(" VG: Default unpack_dual_solution() executed.\n");
  }
  
   if (buf.msgtag() == BCP_Msg_ForVG_User) {
      throw BCP_fatal_error("\
VG: BCP_Msg_LpSolution_User message arrived but the unpack_dual_solution() \n\
    function is not overridden.\n");
   } else {
      int cutnum;
      buf.unpack(cutnum);
      if (cutnum > 0) {
	 // Just to be on the safe side... If it's already empty, it won't
	 // take long.
	 purge_ptr_vector(p->cuts);
	 p->pi.clear();
	 p->cuts.reserve(cutnum);
	 p->pi.reserve(cutnum);
	 double val;
	 while (--cutnum >= 0) {
	    buf.unpack(val);
	    p->pi.unchecked_push_back(val);
	    p->cuts.unchecked_push_back(p->unpack_cut());
	 }
      }
   }
}

//#############################################################################

void
BCP_vg_user::generate_vars(BCP_vec<BCP_cut*>& cuts, BCP_vec<double>& pi)
{
  if (p->par.entry(BCP_vg_par::ReportWhenDefaultIsExecuted)) {
    printf(" VG: Default generate_vars() executed.\n");
  }
}
