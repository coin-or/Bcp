// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_message_tag.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"

#include "BCP_cg.hpp"
#include "BCP_cg_user.hpp"

//#############################################################################
// Informational methods for the user
double BCP_cg_user::upper_bound() const    { return p->ub(); }
int BCP_cg_user::current_phase() const     { return p->phase; }
int BCP_cg_user::current_level() const     { return p->node_level; }
int BCP_cg_user::current_index() const     { return p->node_index; }
int BCP_cg_user::current_iteration() const { return p->node_iteration; }

//#############################################################################
// Informational methods for the user
/* Methods to get/set BCP parameters on the fly */
char
BCP_cg_user::get_param(const BCP_cg_par::chr_params key) const
{ return p->par.entry(key); }
int
BCP_cg_user::get_param(const BCP_cg_par::int_params key) const
{ return p->par.entry(key); }
double
BCP_cg_user::get_param(const BCP_cg_par::dbl_params key) const
{ return p->par.entry(key); }
const BCP_string&
BCP_cg_user::get_param(const BCP_cg_par::str_params key) const
{ return p->par.entry(key); }

void BCP_cg_user::set_param(const BCP_cg_par::chr_params key, const bool val)
{ return p->par.set_entry(key, val); }
void BCP_cg_user::set_param(const BCP_cg_par::chr_params key, const char val)
{ return p->par.set_entry(key, val); }
void BCP_cg_user::set_param(const BCP_cg_par::int_params key, const int val)
{ return p->par.set_entry(key, val); }
void BCP_cg_user::set_param(const BCP_cg_par::dbl_params key, const double val)
{ return p->par.set_entry(key, val); }
void BCP_cg_user::set_param(const BCP_cg_par::str_params key, const char * val)
{ return p->par.set_entry(key, val); }

//#############################################################################

void
BCP_cg_user::send_cut(const BCP_cut& cut)
{
  BCP_buffer& buf = p->msg_buf;
  buf.clear();
  const int bcpind = cut.bcpind();
  const BCP_object_t obj_t = cut.obj_type();
  const BCP_obj_status stat = cut.status();
  const double lb = cut.lb();
  const double ub = cut.ub();
  buf.pack(bcpind).pack(obj_t).pack(stat).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_IndexedObj:
    {
      const int index = (dynamic_cast<const BCP_cut_indexed&>(cut)).index();
      buf.pack(index);
    }
    break;
  case BCP_AlgoObj:
    pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), buf);
    break;
  default:
    throw BCP_fatal_error("BCP_cg_prob::_pack_cut(): unexpected obj_t.\n");
  }
  p->msg_env->send(p->sender, BCP_Msg_CutDescription, buf);
}

//#############################################################################

void
BCP_cg_user::unpack_module_data(BCP_buffer& buf)
{
  if (p->par.entry(BCP_cg_par::ReportWhenDefaultIsExecuted)) {
    printf(" CG: Default unpack_module_data() executed.\n");
  }
}

//#############################################################################

void
BCP_cg_user::unpack_primal_solution(BCP_buffer& buf)
{
  if (p->par.entry(BCP_cg_par::ReportWhenDefaultIsExecuted)) {
    printf(" CG: Default unpack_primal_solution() executed.\n");
  }

   if (buf.msgtag() == BCP_Msg_ForCG_User) {
      throw BCP_fatal_error("\
CG: BCP_Msg_LpSolution_User message arrived but the unpack_primal_solution()\n\
    function is not overridden.\n");
   } else {
      int varnum;
      buf.unpack(varnum);
      if (varnum > 0) {
	 // Just to be on the safe side... If it's already empty, it won't
	 // take long.
	 purge_ptr_vector(p->vars);
	 p->x.clear();
	 p->vars.reserve(varnum);
	 p->x.reserve(varnum);
	 double val;
	 while (--varnum >= 0) {
	    buf.unpack(val);
	    p->x.unchecked_push_back(val);
	    p->vars.unchecked_push_back(p->unpack_var());
	 }
      }
   }
}

//#############################################################################

void
BCP_cg_user::generate_cuts(BCP_vec<BCP_var*>& vars, BCP_vec<double>& x)
{
  if (p->par.entry(BCP_cg_par::ReportWhenDefaultIsExecuted)) {
    printf(" CG: Default generate_cuts() executed.\n");
  }
}

//#############################################################################

BCP_var_algo*
BCP_cg_user::unpack_var_algo(BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_cg_user::unpack_var_algo() invoked but not overridden!\n");
  return 0; // to satisfy aCC on HP-UX
}

void
BCP_cg_user::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_cg_user::pack_cut_algo() invoked but not overridden!\n");
}

//#############################################################################

