// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
// #include <functional>
#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_temporary.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_matrix.hpp"
#include "BCP_branch.hpp"

//#############################################################################

inline void BCP_problem_core::clear() {
   delete matrix;   matrix = 0;
   purge_ptr_vector(cuts);
   purge_ptr_vector(vars);
}

//-----------------------------------------------------------------------------

BCP_problem_core::BCP_problem_core() :
  vars(), cuts(), matrix(new BCP_lp_relax(true /*colordered*/)) {}

BCP_problem_core::~BCP_problem_core()
{
  clear();
}

//-----------------------------------------------------------------------------

void BCP_problem_core::pack(BCP_buffer& buf) const
{
  buf.pack(vars.size());
  if (vars.size() > 0) {
    BCP_vec<BCP_var_core*>::const_iterator vi = vars.begin() - 1;
    const BCP_vec<BCP_var_core*>::const_iterator lastvi = vars.end();
    while (++vi != lastvi) {
      const BCP_var_core& var = **vi;
      const int bcpind = var.bcpind();
      const BCP_object_t obj_t = var.obj_type();
      const BCP_obj_status stat = var.status();
      const BCP_var_t var_t = var.var_type();
      const double obj = var.obj();
      const double lb = var.lb();
      const double ub = var.ub();
      buf.pack(bcpind)
	 .pack(obj_t).pack(stat).pack(var_t).pack(obj).pack(lb).pack(ub);
    }
  }

  buf.pack(cuts.size());
  if (cuts.size() > 0) {
    BCP_vec<BCP_cut_core*>::const_iterator ci = cuts.begin() - 1;
    const BCP_vec<BCP_cut_core*>::const_iterator lastci = cuts.end();
    while (++ci != lastci) {
      BCP_cut_core& cut = **ci;
      const int bcpind = cut.bcpind();
      const BCP_object_t obj_t = cut.obj_type();
      const BCP_obj_status stat = cut.status();
      const double lb = cut.lb();
      const double ub = cut.ub();
      buf.pack(bcpind).pack(obj_t).pack(stat).pack(lb).pack(ub);
    }
  }
  
  matrix->pack(buf);
}

//-----------------------------------------------------------------------------

void BCP_problem_core::unpack(BCP_buffer& buf)
{
  clear();
  size_t size;
  int bcpind;
  BCP_object_t obj_t;
  BCP_obj_status stat;
  BCP_var_t var_t;
  double obj;
  double lb;
  double ub;

  buf.unpack(size);
  if (size > 0) {
    BCP_var_core* bvar;
    for (vars.reserve(size); size; --size) {
      buf.unpack(bcpind)
	 .unpack(obj_t).unpack(stat)
	 .unpack(var_t).unpack(obj).unpack(lb).unpack(ub);
      bvar = new BCP_var_core(var_t, obj, lb, ub);
      bvar->set_bcpind(bcpind);
      bvar->set_status(stat);
      vars.unchecked_push_back(bvar);
    }
  }

  buf.unpack(size);
  if (size > 0) {
    BCP_cut_core* bcut;
    for (cuts.reserve(size); size; --size) {
      buf.unpack(bcpind).unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);
      bcut = new BCP_cut_core(lb, ub);
      bcut->set_bcpind(bcpind);
      bcut->set_status(stat);
      cuts.unchecked_push_back(bcut);
      }
   }

   matrix = new BCP_lp_relax(true /*colordered*/);
   matrix->unpack(buf);
}

//#############################################################################

inline void BCP_problem_core_change::clear() {
   var_pos.clear();
   var_ch.clear();
   cut_pos.clear();
   cut_ch.clear();
}

BCP_problem_core_change&
BCP_problem_core_change::operator=(const BCP_problem_core_change& x) {
   _storage = x._storage;
   var_pos = x.var_pos;
   var_ch = x.var_ch;
   cut_pos = x.cut_pos;
   cut_ch = x.cut_ch;
   return *this;
}

//-----------------------------------------------------------------------------

BCP_problem_core_change&
BCP_problem_core_change::operator=(const BCP_problem_core& core) {
   clear();
   _storage = BCP_Storage_Explicit;

   var_ch.reserve(core.varnum());
   BCP_vec<BCP_var_core*>::const_iterator vi = core.vars.begin();
   const BCP_vec<BCP_var_core*>::const_iterator lastvi = core.vars.end();
   for ( ; vi != lastvi; ++vi)
      var_ch.unchecked_push_back(BCP_obj_change((*vi)->lb(), (*vi)->ub(),
						BCP_ObjNotRemovable));
   cut_ch.reserve(core.cutnum());
   BCP_vec<BCP_cut_core*>::const_iterator ci = core.cuts.begin();
   const BCP_vec<BCP_cut_core*>::const_iterator lastci = core.cuts.end();
   for ( ; ci != lastci; ++ci)
      cut_ch.unchecked_push_back(BCP_obj_change((*ci)->lb(), (*ci)->ub(),
						BCP_ObjNotRemovable));
   return *this;
}
      
//-----------------------------------------------------------------------------

BCP_problem_core_change::BCP_problem_core_change(int bvarnum,
						 BCP_var_set& vars,
						 int bcutnum,
						 BCP_cut_set& cuts) :
   _storage(BCP_Storage_Explicit), var_pos(), var_ch(), cut_pos(), cut_ch()
{
   _storage = BCP_Storage_Explicit;
   var_ch.reserve(bvarnum);
   BCP_var_set::const_iterator vari;
   const BCP_var_set::const_iterator lastvari = vars.entry(bvarnum);
   for (vari = vars.begin(); vari != lastvari; ++vari)
      var_ch.unchecked_push_back(BCP_obj_change((*vari)->lb(), (*vari)->ub(),
						(*vari)->status()));
   cut_ch.reserve(bcutnum);
   BCP_cut_set::const_iterator cuti;
   const BCP_cut_set::const_iterator lastcuti = cuts.entry(bcutnum);
   for (cuti = cuts.begin(); cuti != lastcuti; ++cuti)
      cut_ch.unchecked_push_back(BCP_obj_change((*cuti)->lb(), (*cuti)->ub(),
						(*cuti)->status()));
}

//-----------------------------------------------------------------------------

BCP_problem_core_change::
BCP_problem_core_change(BCP_storage_t storage,
			BCP_problem_core_change& ocore,
			BCP_problem_core_change& ncore) :
   _storage(storage), var_pos(), var_ch(), cut_pos(), cut_ch()
{
   if (storage != BCP_Storage_WrtParent && storage != BCP_Storage_WrtCore) {
      throw BCP_fatal_error("\
BCP_problem_core_change() : bad proposed storage\n");
   }

   if (ocore.storage() != BCP_Storage_Explicit)
     throw BCP_fatal_error("BCP_problem_core_change() : bad ocore storage\n");
   if (ncore.storage() != BCP_Storage_Explicit)
     throw BCP_fatal_error("BCP_problem_core_change() : bad ncore storage\n");

   int i;

   const int bvarnum = ncore.varnum();
   var_pos.reserve(bvarnum);
   var_ch.reserve(bvarnum);
   for (i = 0; i < bvarnum; ++i) {
      if (ocore.var_ch[i] != ncore.var_ch[i]) {
	 var_pos.unchecked_push_back(i);
	 var_ch.unchecked_push_back(ncore.var_ch[i]);
      }
   }

   const int bcutnum = ncore.cutnum();
   cut_pos.reserve(bcutnum);
   cut_ch.reserve(bcutnum);
   for (i = 0; i < bcutnum; ++i) {
      if (ocore.cut_ch[i] != ncore.cut_ch[i]) {
	 cut_pos.unchecked_push_back(i);
	 cut_ch.unchecked_push_back(ncore.cut_ch[i]);
      }
   }
}

//-----------------------------------------------------------------------------

void
BCP_problem_core_change::ensure_explicit(const BCP_problem_core_change& ecore)
{
   if (_storage == BCP_Storage_Explicit)
      return;
   if (_storage != BCP_Storage_WrtCore)
      throw BCP_fatal_error("\
BCP_problem_core_change::ensure_explicit() : bad current storage\n");
   if (ecore.storage() != BCP_Storage_Explicit)
      throw BCP_fatal_error("\
BCP_problem_core_change::ensure_explicit() : bad ecore storage\n");

   BCP_problem_core_change new_core;
   new_core.update(ecore, *this);
   swap(new_core);
}

//-----------------------------------------------------------------------------

void
BCP_problem_core_change::
make_wrtcore_if_shorter(const BCP_problem_core_change& ecore)
{
   if (_storage != BCP_Storage_Explicit)
      throw BCP_fatal_error("\
BCP_problem_core_change::make_wrtcore_if_shorter() : bad storage\n");
   if (ecore.storage() != BCP_Storage_Explicit)
      throw BCP_fatal_error("\
BCP_problem_core_change::make_wrtcore_if_shorter() : bad ecore storage\n");
   if (varnum() != ecore.varnum())
      throw BCP_fatal_error("\
BCP_problem_core_change::make_wrtcore_if_shorter() : nonequal sizes\n");

   int i;

   // save the positions of the changed vars / cuts
   BCP_temp_vec<int> tmp_chvar(ecore.varnum());
   BCP_vec<int>& chvar = tmp_chvar.vec();

   const int bvarnum = var_ch.size();
   for (i = 0; i < bvarnum; ++i)
      if (var_ch[i] != ecore.var_ch[i])
	 chvar.unchecked_push_back(i);

   BCP_temp_vec<int> tmp_chcut(ecore.varnum());
   BCP_vec<int>& chcut = tmp_chcut.vec();

   const int bcutnum = cut_ch.size();
   for (i = 0; i < bcutnum; ++i)
      if (cut_ch[i] != ecore.cut_ch[i])
	 chcut.unchecked_push_back(i);

   if ((chvar.size() + chcut.size()) * sizeof(int) <
       (ecore.varnum() - chvar.size() + ecore.cutnum() - chcut.size()) *
       BCP_obj_change::pack_size()) {
      // wrt core is shorter
      _storage = BCP_Storage_WrtCore;
      var_pos.swap(chvar);
      var_ch.keep_by_index(var_pos);
      cut_pos.swap(chcut);
      cut_ch.keep_by_index(cut_pos);
   }
}

//-----------------------------------------------------------------------------

void BCP_problem_core_change::swap(BCP_problem_core_change& other)
{
   std::swap(_storage, other._storage);

   var_pos.swap(other.var_pos);
   var_ch.swap(other.var_ch);
   cut_pos.swap(other.cut_pos);
   cut_ch.swap(other.cut_ch);
}

//-----------------------------------------------------------------------------

void BCP_problem_core_change::update(const BCP_problem_core_change& expl_core,
				     const BCP_problem_core_change& ch_core)
{
   switch (ch_core._storage){
    case BCP_Storage_Explicit:
      operator=(ch_core); // _storage becomes Explicit
      break;

    case BCP_Storage_NoData:
      _storage = BCP_Storage_NoData;
      break;

    case BCP_Storage_WrtCore:
      if (expl_core.storage() != BCP_Storage_Explicit)
	 throw BCP_fatal_error("BCP_problem_core_change::update():\n\
   ch_core's WrtCore, but expl_core's not Explicit\n");
      operator=(expl_core); // _storage becomes Explicit
      // deliberate fall-through

    case BCP_Storage_WrtParent:
      if (_storage != BCP_Storage_Explicit)
	 throw BCP_fatal_error("BCP_problem_core_change::update():\n\
   _storage isn't Explicit && ch_core's WrtParent\n");
      var_ch.update(ch_core.var_pos, ch_core.var_ch);
      cut_ch.update(ch_core.cut_pos, ch_core.cut_ch);
      break;

    default:
      throw BCP_fatal_error("\
BCP_problem_core_change::update(): bad ch_core storage!\n");
      break;
   }
}

//-----------------------------------------------------------------------------

int BCP_problem_core_change::pack_size() const {
   int siz = sizeof(BCP_storage_t);
   siz += sizeof(int) + sizeof(int) * var_pos.size();
   siz += sizeof(int) + BCP_obj_change::pack_size() * var_ch.size();
   siz += sizeof(int) + sizeof(int) * cut_pos.size();
   siz += sizeof(int) + BCP_obj_change::pack_size() * cut_ch.size();
   return siz;
}

//-----------------------------------------------------------------------------

void BCP_problem_core_change::pack(BCP_buffer& buf) const {
   buf.pack(_storage);
   if (_storage != BCP_Storage_NoData)
      buf.pack(var_pos).pack(cut_pos).pack(var_ch).pack(cut_ch);
}

//-----------------------------------------------------------------------------

void BCP_problem_core_change::unpack(BCP_buffer& buf) {
   buf.unpack(_storage);
   if (_storage != BCP_Storage_NoData)
      buf.unpack(var_pos).unpack(cut_pos).unpack(var_ch).unpack(cut_ch);
}
