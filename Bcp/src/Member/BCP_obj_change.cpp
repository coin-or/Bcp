// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <CoinDistance.hpp>
#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_obj_change.hpp"
// #include "BCP_var.hpp"
// #include "BCP_cut.hpp"

//#############################################################################

void
BCP_obj_set_change::update(const BCP_obj_set_change& objs_change)
{
   if (objs_change.storage() == BCP_Storage_Explicit){
      _new_objs.clear();
      _change.clear();
   }else{
      if (objs_change.deleted_num() > 0){
	 BCP_vec<int>::const_iterator firstdel =
	    objs_change._del_change_pos.begin();
	 BCP_vec<int>::const_iterator lastdel =
	    objs_change._del_change_pos.entry(objs_change.deleted_num());
	 _new_objs.erase_by_index(firstdel, lastdel);
	 _change.erase_by_index(firstdel, lastdel);
      }
   }
   if (objs_change.added_num() > 0) {
      _new_objs.append(objs_change._new_objs);
      _change.append(objs_change._change.entry(objs_change.changed_num() -
					       objs_change.added_num()),
		     objs_change._change.end());
   }
   if (objs_change.changed_num() - objs_change.added_num() > 0){
#if PARANOID
      if (objs_change.storage() != BCP_Storage_WrtParent)
	 throw BCP_fatal_error("BCP_obj_set_change::update(): oops\n");
#endif
      BCP_vec<BCP_obj_change>::const_iterator firstch =
	 objs_change._change.begin();
      BCP_vec<BCP_obj_change>::const_iterator lastch =
	 objs_change._change.entry(objs_change.changed_num() -
				   objs_change.added_num());
      BCP_vec<int>::const_iterator firstchpos =
	 objs_change._del_change_pos.entry(objs_change.deleted_num());
      while (firstch != lastch) {
	 _change[*firstchpos] = *firstch;
	 ++firstch;
	 ++firstchpos;
      }
   }
}

//=============================================================================

void
BCP_obj_set_change::swap(BCP_obj_set_change& x)
{
   std::swap(_storage, x._storage);
   std::swap(_deleted_num, x._deleted_num);
   _del_change_pos.swap(x._del_change_pos);
   _change.swap(x._change);
   _new_objs.swap(x._new_objs);
}

//=============================================================================

int 
BCP_obj_set_change::pack_size() const
{
   return ( 5 * sizeof(int) + _del_change_pos.size() * sizeof(int) +
	    _change.size() * BCP_obj_change::pack_size() +
	    _new_objs.size() * sizeof(int) );
}

//-----------------------------------------------------------------------------

void
BCP_obj_set_change::pack(BCP_buffer& buf) const
{
    buf.pack(_storage);
    buf.pack(_deleted_num);
    buf.pack(_del_change_pos);
    buf.pack(_change);
    buf.pack(_new_objs);
}

//-----------------------------------------------------------------------------

void
BCP_obj_set_change::unpack(BCP_buffer& buf)
{
    buf.unpack(_storage);
    buf.unpack(_deleted_num);
    buf.unpack(_del_change_pos);
    buf.unpack(_change);
    buf.unpack(_new_objs);
}

//-----------------------------------------------------------------------------

void
BCP_obj_set_change::print() const
{
  printf("        storage: %i\n", _storage);
  printf("        deleted_num: %i\n", _deleted_num);
  if (_deleted_num > 0) {
    printf("        deleted pos:\n");
    for (int i = 0; i < _deleted_num; ++i) {
      printf("          %i\n", _del_change_pos[i]);
    }
  }
  if (_deleted_num < _del_change_pos.size()) {
    printf("        changed pos/newval:\n");
    const int added = added_num();
    for (int i = 0; i < added; ++i) {
      printf("          pos %i,  bcpind %i,  status %i,  lb %lf,  ub %lf\n",
	     _del_change_pos[_deleted_num+i], _new_objs[i],
	     _change[i].stat, _change[i].lb, _change[i].ub);
    }
  }
}

//#############################################################################
#if 0
//#############################################################################

void
BCP_var_set_change::update(const BCP_var_set_change& vars_change)
{
   if (vars_change.storage() == BCP_Storage_Explicit){
      _new_vars.clear();
      _change.clear();
   }else{
      if (vars_change.deleted_num() > 0){
	 BCP_vec<int>::const_iterator firstdel =
	    vars_change._del_change_pos.begin();
	 BCP_vec<int>::const_iterator lastdel =
	    vars_change._del_change_pos.entry(vars_change.deleted_num());
	 _new_vars.erase_by_index(firstdel, lastdel);
	 _change.erase_by_index(firstdel, lastdel);
      }
   }
   if (vars_change.added_num() > 0) {
      _new_vars.append(vars_change._new_vars);
      _change.append(vars_change._change.entry(vars_change.changed_num() -
					       vars_change.added_num()),
		     vars_change._change.end());
   }
   if (vars_change.changed_num() - vars_change.added_num() > 0){
#if PARANOID
      if (vars_change.storage() != BCP_Storage_WrtParent)
	 throw BCP_fatal_error("BCP_update_varset_and_changes(): oops\n");
#endif
      BCP_vec<BCP_obj_change>::const_iterator firstch =
	 vars_change._change.begin();
      BCP_vec<BCP_obj_change>::const_iterator lastch =
	 vars_change._change.entry(vars_change.changed_num() -
				   vars_change.added_num());
      BCP_vec<int>::const_iterator firstchpos =
	 vars_change._del_change_pos.entry(vars_change.deleted_num());
      while (firstch != lastch) {
	 _change[*firstchpos] = *firstch;
	 ++firstch;
	 ++firstchpos;
      }
   }
}

//=============================================================================

void
BCP_obj_set_change::pack(BCP_buffer& buf)
{
    buf.pack(_storage);
    buf.pack(_deleted_num);
    buf.pack(_del_change_pos);
    buf.pack(_change);
    buf.pack(_new_objs);
}

//-----------------------------------------------------------------------------

void
BCP_obj_set_change::unpack(BCP_buffer& buf)
{
    buf.unpack(_storage);
    buf.unpack(_deleted_num);
    buf.unpack(_del_change_pos);
    buf.unpack(_change);
    buf.unpack(_new_objs);
}

//#############################################################################

void
BCP_cut_set_change::update(const BCP_cut_set_change& cuts_change)
{
   if (cuts_change.storage() == BCP_Storage_Explicit){
      _new_cuts.clear();
      _change.clear();
   }else{
      if (cuts_change.deleted_num() > 0){
	 BCP_vec<int>::const_iterator firstdel =
	    cuts_change._del_change_pos.begin();
	 BCP_vec<int>::const_iterator lastdel =
	    cuts_change._del_change_pos.entry(cuts_change.deleted_num());
	 // BCP_vec<BCP_cut*>& cutset =
	 //   dynamic_cast<BCP_vec<BCP_cut*>&>(*this);
	 _new_cuts.erase_by_index(firstdel, lastdel);
	 _change.erase_by_index(firstdel, lastdel);
      }
   }
   if (cuts_change.added_num() > 0) {
      _new_cuts.append(cuts_change._new_cuts);
      _change.append(cuts_change._change.entry(cuts_change.changed_num() -
					       cuts_change.added_num()),
		     cuts_change._change.end());
   }
   if (cuts_change.changed_num() - cuts_change.added_num() > 0){
#if PARANOID
      if (cuts_change.storage() != BCP_Storage_WrtParent)
	 throw BCP_fatal_error("BCP_update_cutset_and_changes(): oops\n");
#endif
      BCP_vec<BCP_obj_change>::const_iterator firstch =
	 cuts_change._change.begin();
      BCP_vec<BCP_obj_change>::const_iterator lastch =
	 cuts_change._change.entry(cuts_change.changed_num() -
				   cuts_change.added_num());
      BCP_vec<int>::const_iterator firstchpos =
	 cuts_change._del_change_pos.entry(cuts_change.deleted_num());
      while (firstch != lastch) {
	 _change[*firstchpos] = *firstch;
	 ++firstch;
	 ++firstchpos;
      }
   }
}

//#############################################################################
//#############################################################################

BCP_var_set_change::BCP_var_set_change(BCP_vec<BCP_var*>::const_iterator first,
				       BCP_vec<BCP_var*>::const_iterator last):
   _storage(BCP_Storage_Explicit), _deleted_num(0),
   _del_change_pos(), _change(), _new_vars()
{
   if (first != last) {
      const int added_var_num = coinDistance(first, last);
      _change.reserve(added_var_num);
      _new_vars.reserve(added_var_num);
      while (first != last) {
	 const BCP_var* var = *first;
	 _change.unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
						    var->status()));
	 _new_vars.unchecked_push_back(*first);
	 ++first;
      }
   }
}

//-----------------------------------------------------------------------------

BCP_var_set_change::
BCP_var_set_change(BCP_vec<BCP_var*>::const_iterator first,
		   BCP_vec<BCP_var*>::const_iterator last,
		   const BCP_vec<int>& added_bcpind,
		   const BCP_vec<BCP_obj_change>& added_desc) :
   _storage(BCP_Storage_WrtParent), _deleted_num(0),
   _del_change_pos(), _change(), _new_vars()
{
   const int new_added_num = coinDistance(first, last);
   const int old_added_num = added_bcpind.size();
   _del_change_pos.reserve(old_added_num);

   BCP_vec<int> chpos;
   chpos.reserve(new_added_num);

   int i, j;

   // first check how many entry has been deleted from oldvars
   for (i = 0, j = 0; i < new_added_num && j < old_added_num; ++j) {
      const BCP_var* const var = *(first + i);
      const BCP_obj_change& added = added_desc[j];
      if (var->bcpind() == added_bcpind[j]) {
	 // added_bcpind ALWAYS has real indices, so this really separates
	 if (var->lb() != added.lb || var->ub() != added.ub ||
	     var->status() != added.stat)
	    chpos.unchecked_push_back(i);
	 ++i;
      } else {
	 _del_change_pos.unchecked_push_back(j);
      }
   }
   // append the remains of old_added to _del_change_pos
   for ( ; j < old_added_num; ++j) {
      _del_change_pos.unchecked_push_back(j);
   }
   // _deleted_num is the current length of _del_change_pos
   _deleted_num = _del_change_pos.size();

   // the rest are the set of really new vars, and also the position of those
   // vars must be appended to chpos.
   _new_vars.reserve(new_added_num - i);
   for ( ; i < new_added_num; ++i){
      _new_vars.unchecked_push_back(*(first + i));
      chpos.unchecked_push_back(i);
   }
   // append chpos to _del_change_pos to get the final list
   _del_change_pos.append(chpos);

   // finally, create _change: just pick up things based on chpos
   const int chnum = chpos.size();
   _change.reserve(chnum);
   for (i = 0; i < chnum; ++i) {
      const BCP_var* const var = *(first + chpos[i]);
      _change.unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
						 var->status()));
   }
}

//-----------------------------------------------------------------------------

void
BCP_var_set_change::swap(BCP_var_set_change& x)
{
   std::swap(_storage, x._storage);
   std::swap(_deleted_num, x._deleted_num);
   _del_change_pos.swap(x._del_change_pos);
   _change.swap(x._change);
   _new_vars.swap(x._new_vars);
}

//-----------------------------------------------------------------------------

int 
BCP_var_set_change::pack_size() const
{
   return ( _del_change_pos.size() * sizeof(int) +
	    _change.size() * BCP_obj_change::pack_size() +
	    _new_vars.size() * (sizeof(bool) + sizeof(int)) );
}

//#############################################################################

BCP_cut_set_change::BCP_cut_set_change(BCP_vec<BCP_cut*>::const_iterator first,
				       BCP_vec<BCP_cut*>::const_iterator last):
   _storage(BCP_Storage_Explicit), _deleted_num(0),
   _del_change_pos(), _change(), _new_cuts()
{
   if (first != last) {
      const int added_cut_num = coinDistance(first, last);
      _change.reserve(added_cut_num);
      _new_cuts.reserve(added_cut_num);
      while (first != last) {
	 const BCP_cut* cut = *first;
	 _change.unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
						    cut->status()));
	 _new_cuts.unchecked_push_back(*first);
	 ++first;
      }
   }
}

//-----------------------------------------------------------------------------

BCP_cut_set_change::
BCP_cut_set_change(BCP_vec<BCP_cut*>::const_iterator first,
		   BCP_vec<BCP_cut*>::const_iterator last,
		   const BCP_vec<int>& added_bcpind,
		   const BCP_vec<BCP_obj_change>& added_desc) :
   _storage(BCP_Storage_WrtParent), _deleted_num(0),
   _del_change_pos(), _change(), _new_cuts()
{
   const int new_added_num = coinDistance(first, last);
   const int old_added_num = added_bcpind.size();
   _del_change_pos.reserve(old_added_num);

   BCP_vec<int> chpos;
   chpos.reserve(new_added_num);

   int i, j;

   // first check how many entry has been deleted from oldcuts
   for (i = 0, j = 0; i < new_added_num && j < old_added_num; ++j) {
      const BCP_cut* const cut = *(first + i);
      const BCP_obj_change& added = added_desc[j];
      if (cut->bcpind() == added_bcpind[j]) {
	 // added_bcpind ALWAYS has real indices, so this really separates
	 if (cut->lb() != added.lb || cut->ub() != added.ub ||
	     cut->status() != added.stat)
	    chpos.unchecked_push_back(i);
	 ++i;
      } else {
	 _del_change_pos.unchecked_push_back(j);
      }
   }
   // append the remains of old_added to _del_change_pos
   for ( ; j < old_added_num; ++j) {
      _del_change_pos.unchecked_push_back(j);
   }
   // _deleted_num is the current length of _del_change_pos
   _deleted_num = _del_change_pos.size();

   // the rest are the set of really new cuts, and also the position of those
   // cuts must be appended to chpos.
   _new_cuts.reserve(new_added_num - i);
   for ( ; i < new_added_num; ++i){
      _new_cuts.unchecked_push_back(*(first + i));
      chpos.unchecked_push_back(i);
   }
   // append chpos to _del_change_pos to get the final list
   _del_change_pos.append(chpos);

   // finally, create _change: just pick up things based on chpos
   const int chnum = chpos.size();
   _change.reserve(chnum);
   for (i = 0; i < chnum; ++i) {
      const BCP_cut* const cut = *(first + chpos[i]);
      _change.unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
						 cut->status()));
   }
}

//-----------------------------------------------------------------------------

void
BCP_cut_set_change::swap(BCP_cut_set_change& x)
{
   std::swap(_storage, x._storage);
   std::swap(_deleted_num, x._deleted_num);
   _del_change_pos.swap(x._del_change_pos);
   _change.swap(x._change);
   _new_cuts.swap(x._new_cuts);
}

//-----------------------------------------------------------------------------

int 
BCP_cut_set_change::pack_size() const
{
   return ( _del_change_pos.size() * sizeof(int) +
	    _change.size() * BCP_obj_change::pack_size() +
	    _new_cuts.size() * (sizeof(bool) + sizeof(int)) );
}
#endif
