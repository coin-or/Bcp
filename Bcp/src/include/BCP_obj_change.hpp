// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_OBJ_CHANGE_H
#define _BCP_OBJ_CHANGE_H

//#############################################################################

#include "BCP_enum.hpp"
#include "BCP_vector.hpp"

//#############################################################################

class BCP_var;
class BCP_cut;

class BCP_cut_set;
class BCP_var_set;

class BCP_buffer;

//#############################################################################

class BCP_obj_change {
public:
    double lb;
    double ub;
    BCP_obj_status stat;
public:
    BCP_obj_change(const double lower, const double upper,
		   const BCP_obj_status status) :
	lb(lower), ub(upper), stat(status) {}
    BCP_obj_change() : lb(0), ub(0), stat(BCP_ObjNoInfo) {}
    ~BCP_obj_change() {}
    // default copy constructor and assignment operator are fine

    static inline int pack_size() {
	return 2 * sizeof(double) + sizeof(BCP_obj_status);
    }
};

inline bool operator==(const BCP_obj_change& ch0, const BCP_obj_change& ch1)
{
    return ch0.lb == ch1.lb && ch0.ub == ch1.ub && ch0.stat == ch1.stat;
}

inline bool operator!=(const BCP_obj_change& ch0, const BCP_obj_change& ch1)
{
    return ch0.lb != ch1.lb || ch0.ub != ch1.ub || ch0.stat != ch1.stat;
}

//#############################################################################

/** This class stores data about how an object set (set of vars or set of
    cuts) changes
*/

class BCP_obj_set_change {
    // _storage tells how the var_set is stored.
    // -- If wrt parent then _deleted_num tells how many are deleted from the
    // parent, and their indices are listed on the first _deleted_num slots of
    // _del_change_pos. The rest of _del_change_pos shows the indices of
    // the vars that have some param changed. _change contains these changes
    // and finally _new_vars has the vars to be added, that is, their bcpind,
    // which might be the negative of the real bcpind in case a var has not
    // yet been sent to the TM.
    // -- If explicite list then _new_vars has the complete list.
private:
    // disable default copy constructor and assignment operator
    // BCP_obj_set_change(const BCP_obj_set_change&);
    // BCP_obj_set_change& operator=(const BCP_obj_set_change&);
public:
    BCP_storage_t           _storage;
    int                     _deleted_num;
    BCP_vec<int>            _del_change_pos;
    BCP_vec<BCP_obj_change> _change;
    BCP_vec<int>            _new_objs;
public:
    BCP_obj_set_change() :
	_storage(BCP_Storage_Explicit), _deleted_num(0),
	_del_change_pos(), _change(), _new_objs() {}
    // default copy constructor and assignment op are OK (though expensive...)
    ~BCP_obj_set_change() {}
    
    inline BCP_storage_t storage() const { return _storage; }

    inline int deleted_num() const { return _deleted_num; }
    inline int changed_num() const { return _change.size(); }
    inline int added_num() const { return _new_objs.size(); }

    void update(const BCP_obj_set_change& objs_change);
    void swap(BCP_obj_set_change& x);    // *INLINE ?*

    int pack_size() const;
    void pack(BCP_buffer& buf) const;
    void unpack(BCP_buffer& buf);
};

#if 0
//#############################################################################

class BCP_var_set_change {
    // _storage tells how the var_set is stored.
    // -- If wrt parent then _deleted_num tells how many are deleted from the
    // parent, and their indices are listed on the first _deleted_num slots of
    // _del_change_pos. The rest of _del_change_pos shows the indices of
    // the vars that have some param changed. _change contains these changes
    // and finally _new_vars has the vars to be added, that is, their bcpind,
    // which might be the negative of the real bcpind in case a var has not
    // yet been sent to the TM.
    // -- If explicite list then _new_vars has the complete list.
private:
    // disable default copy constructor and assignment operator
    BCP_var_set_change(const BCP_var_set_change&);
    BCP_var_set_change& operator=(const BCP_var_set_change&);
public:
    BCP_storage_t           _storage;
    int                     _deleted_num;
    BCP_vec<int>            _del_change_pos;
    BCP_vec<BCP_obj_change> _change;
    BCP_vec<int>            _new_vars;
public:
    BCP_var_set_change() :
	_storage(BCP_Storage_Explicit), _deleted_num(0),
	_del_change_pos(), _change(), _new_vars() {}
#if 0
    BCP_var_set_change(BCP_vec<BCP_var*>::const_iterator firstvar,
		       BCP_vec<BCP_var*>::const_iterator lastvar);
    BCP_var_set_change(BCP_vec<BCP_var*>::const_iterator firstvar,
		       BCP_vec<BCP_var*>::const_iterator lastvar,
		       const BCP_vec<int>& added_index,
		       const BCP_vec<BCP_obj_change>& added_desc);
#endif
    ~BCP_var_set_change() {}
    
    void swap(BCP_var_set_change& x);    // *INLINE ?*

    inline BCP_storage_t storage() const { return _storage; }

    inline int deleted_num() const { return _deleted_num; }
    inline int changed_num() const { return _change.size(); }
    inline int added_num() const { return _new_vars.size(); }

    int pack_size() const;

    void update(const BCP_var_set_change& vars_change);
};

//#############################################################################

class BCP_cut_set_change {
    // _storage tells how the cut_set is stored.
    // -- If wrt parent then _deleted_num tells how many are deleted from the
    // parent, and their indices are listed on the first _deleted_num slots of
    // _del_change_pos. The rest of _del_change_pos shows the indices of
    // the cuts that have some param changed. _change contains these changes
    // and finally _new_cuts has the cuts to be added, that is, their bcpind,
    // which might be the negative of the real bcpind in case a var has not
    // yet been sent to the TM.
    // -- If explicite list then _new_cuts has the complete list.
private:
    // disable default copy constructor and assignment operator
    BCP_cut_set_change(const BCP_cut_set_change&);
    BCP_cut_set_change& operator=(const BCP_cut_set_change&);
public:
    BCP_storage_t           _storage;
    int                     _deleted_num;
    BCP_vec<int>            _del_change_pos;
    BCP_vec<BCP_obj_change> _change;
    BCP_vec<int>            _new_cuts;
public:
    BCP_cut_set_change() :
	_storage(BCP_Storage_Explicit), _deleted_num(0),
	_del_change_pos(), _change(), _new_cuts() {}
#if 0
    BCP_cut_set_change(BCP_vec<BCP_cut*>::const_iterator firstcut,
		       BCP_vec<BCP_cut*>::const_iterator lastcut);
    BCP_cut_set_change(BCP_vec<BCP_cut*>::const_iterator firstcut,
		       BCP_vec<BCP_cut*>::const_iterator lastcut,
		       const BCP_vec<int>& added_index,
		       const BCP_vec<BCP_obj_change>& added_desc);
#endif
    ~BCP_cut_set_change() {}

    void swap(BCP_cut_set_change& x);    // *INLINE ?*

    inline BCP_storage_t storage() const { return _storage; }

    inline int deleted_num() const { return _deleted_num; }
    inline int changed_num() const { return _change.size(); }
    inline int added_num() const { return _new_cuts.size(); }

    int pack_size() const;

    void update(const BCP_cut_set_change& cuts_change);
};

//#############################################################################
#endif

#endif
