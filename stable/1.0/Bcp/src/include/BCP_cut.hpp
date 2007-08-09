// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_CUT_H
#define _BCP_CUT_H

// This file is fully docified.

//#############################################################################

#include "BCP_math.hpp"
#include "BCP_error.hpp"
#include "BCP_enum.hpp"
#include "BCP_vector.hpp"
#include "BCP_obj_change.hpp"

//#############################################################################

// Generic cut definition

/**
   Abstract base class that defines members common to all types of cuts.
   Classes describing the three types of cuts (core, indexed and algorithmic)
   are derived from this class. No object of type BCP_cut can
   exist (having purely virtual members in the class enforces this
   restriction).  
 */

class BCP_cut{
private:
  /**@name Disabled methods */
  /*@{*/
    /** The default constructor is declared but not defined to disable it. */
    BCP_cut();
    /** The copy constructor is declared but not defined to disable it. */
    BCP_cut(const BCP_cut&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_cut& operator=(const BCP_cut&);
  /*@}*/

private:
  /** These data members are used only by BCP, the user need not worry about
      them. */ 
  /*@{*/
    /** The internal, unique index of the cut. */
    int  _bcpind;
    /** The status of the cut. */
    BCP_obj_status _status;
    /** Effectiveness counter (used only in the LP process). */
    int            _eff_cnt;
  /*@}*/

protected:
  /*@{*/
    /** Lower bound of the cut. */
    double         _lb;
    /** Upper bound of the cut. */
    double         _ub;
  /*@}*/

public:
  /**@name Constructor and destructor
     Note that there is no default constructor. There is no such thing as
     "default cut". */
  /*@{*/
    /** The constructor sets the internal index of the cut to zero and the
        other data members to the given arguments. */
    BCP_cut(const double lb, const double ub) :
       _bcpind(0), _status(BCP_ObjNoInfo), _eff_cnt(0), _lb(lb), _ub(ub) {}
    /** The destructor is virtual so that the appropriate destructor is invoked
        for every cut. */
    virtual ~BCP_cut() {}
  /*@}*/
   
  /**@name Query methods */
  /*@{*/
    /** Return the type of the variable. */
    virtual BCP_object_t obj_type() const = 0;
    /** Return the effectiveness count of the cut (only in LP process). */
    inline int effective_count() const   { return _eff_cnt; }
    /** Return the lower bound on the cut. */
    inline double lb() const             { return _lb; }
    /** Return the upper bound on the cut. */
    inline double ub() const             { return _ub; }
    /** Return the internal index of the cut. */
    inline int bcpind() const  { return _bcpind; }

    /* *@name Query methods about the status of the variable */
    /* @{*/
      /** Return the status of the cut. */
      inline BCP_obj_status status() const { return _status; }
      /** Return whether the cut should be sent to the Cut Pool process.
          (Assuming that it stays in the formulation long enough to qualify to
	  be sent to the Cut Pool at all. */
      inline bool dont_send_to_pool() const {
         return _status & BCP_ObjDoNotSendToPool ? true : false;
      }
      /** Return whether the cut marked as NotRemovable. Such cuts are, e.g.,
	  the branching cuts. */
      inline bool is_non_removable() const {
         return (_status & BCP_ObjNotRemovable) ? true : false;
      }
      /** Return whether the cut must be removed from the formulation. There
	  are very few circumstances when this flag is set; all of them are
          completely internal to BCP. */
      inline bool is_to_be_removed() const {
         return (_status & BCP_ObjToBeRemoved) != 0;
      }
    /* @}*/
  /*@}*/

  /**@name Modifying methods */
  /*@{*/
    /** Set the effectiveness count to the given value. */
    inline void set_effective_count(const int cnt) { _eff_cnt = cnt; }
    /** Increase the effectiveness count by 1 (or to 1 if it was negative).
        Return the new effectiveness count. */
    inline int increase_effective_count() {
       _eff_cnt = _eff_cnt <= 0 ? 1 : _eff_cnt + 1;
       return _eff_cnt;
    }
    /** Decrease the effectiveness count by 1 (or to -1 if it was positive).
        Return the new effectiveness count. */
    inline int decrease_effective_count() {
       _eff_cnt = _eff_cnt >= 0 ? -1 : _eff_cnt - 1;
       return _eff_cnt;
    }
    /** Set the lower bound on the cut. */
    inline void set_lb(const double lb) { _lb = lb; }
    /** Set the upper bound on the cut. */
    inline void set_ub(const double ub) { _ub = ub; }
    /** Set the lower/upper bounds and the status of the cut simultaneously to
        the values given in the data members of the argument. */
    inline void change_lb_ub_st(const BCP_obj_change& change) {
       _lb = change.lb;
       _ub = change.ub;
       _status = change.stat;
       if (_lb < -BCP_DBL_MAX/10 && _ub > BCP_DBL_MAX/10)
  	 _status = static_cast<BCP_obj_status>(_status | BCP_ObjInactive);
    }
    /** Change just the lower/upper bounds. */
    inline void change_bounds(const double lb, const double ub) {
       _lb = lb;
       _ub = ub;
       if (lb < BCP_DBL_MAX/10 && ub > BCP_DBL_MAX/10)
  	 _status = static_cast<BCP_obj_status>(_status | BCP_ObjInactive);
    }
    /** Set the internal index of the cut. */
    inline void set_bcpind(const int bcpind)  { _bcpind = bcpind; }

    /* *@name Status modifying methods */
    /* @{*/
      /** Set the status of the cut. */
      inline void set_status(const BCP_obj_status stat) { _status = stat; }
      /** Set/unset the flag controlling whether the cut could be sent to the
	  Cut Pool process. */
      inline void dont_send_to_pool(bool flag) {
         _status = static_cast<BCP_obj_status>(flag ?
      					    _status | BCP_ObjDoNotSendToPool :
      					    _status & ~BCP_ObjDoNotSendToPool);
      }
      /** Mark the cut as active. Note that when this method is invoked the lp
          formulation must be modified as well: the original bounds of the cut
          must be reset. */
      inline void make_active() {
         _status = static_cast<BCP_obj_status>(_status & ~BCP_ObjInactive);
      }
      /** Mark the cut as NotRemovable. */
      inline void make_non_removable() {
         _status =
	   static_cast<BCP_obj_status> ((_status & ~BCP_ObjToBeRemoved) |
					BCP_ObjNotRemovable);
      }
      /** Mark the cut as ToBeRemoved. It will actually be removed immediately
          after all cuts that have to be marked this way are marked. */
      inline void make_to_be_removed() {
         _status = BCP_ObjToBeRemoved;
      }
    /* @}*/
  /*@}*/
};

//#############################################################################
//#############################################################################

/**
   Core cuts are the cuts that always stay in the LP formulation. Therefore
   the data members in the base class are quite sufficient to describe the
   cut. The only thing that has to be done here is overriding the pure virtual
   method obj_type().
*/

class BCP_cut_core : public BCP_cut {

private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_cut_core();
   /** The assignment operator is declared but not defined to disable it. */
   BCP_cut_core& operator=(const BCP_cut_core&);
   /*@}*/
   
public:
   /**@name Constructors and desctructor */
   /*@{*/
   /** The copy constructor makes a replica of the argument. */
   BCP_cut_core(const BCP_cut_core& x) : BCP_cut(x._lb, x._ub) {
      set_bcpind(x.bcpind());
      set_status(x.status());
      set_effective_count(x.effective_count());
   }
   /** This constructor just sets the data members to the given values. See
       also the constructor of BCP_cut. */
   BCP_cut_core(const double lb, const double ub) : BCP_cut(lb, ub) {}
   /** The destructor deletes the object. */
   ~BCP_cut_core() {}
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return <code>BCP_CoreObj</code> indicating that the object is a core
       cut. */
   inline BCP_object_t obj_type() const  { return BCP_CoreObj; }
   /*@}*/
};

//#############################################################################

/** Implementation-wise indexed cuts differ from core cuts only in having a
    user given index. This index is a unique user specified ID which enables
    the user to expand the cut into a constraint that can be added to the LP
    formulation. */

class BCP_cut_indexed : public BCP_cut {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_cut_indexed();
   /** The copy constructor is declared but not defined to disable it. */
   BCP_cut_indexed(const BCP_cut_indexed&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_cut_indexed& operator=(const BCP_cut_indexed&);
   /*@}*/

private:
   /**@name Data members */
   /*@{*/
   /** The only (not inherited) data member is the user-given name of the 
       cut. */
   int _index;
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** This constructor just sets the data members to the given values. See
       also the constructor of \URL[<code>BCP_cut</code>{BCP_cut.html}. */
   BCP_cut_indexed(const int index, const double lb, const double ub) :
      BCP_cut(lb, ub), _index(index) {}
   /** The destructor deletes the object. */
   ~BCP_cut_indexed() {}
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return the user index of the cut. */
   inline int index() const             { return _index; }
   /** Return BCP_IndexedObj indicating that the object is an indexed cut. */
   inline BCP_object_t obj_type() const { return BCP_IndexedObj; }
   /*@}*/
};

//#############################################################################

// This is the class the user should derive his/her own algorithmic cuts

/** 
   This is the class from which the user should derive her own algorithmic
   cuts. Note that such an object cannot be constructed (it has pure virtual
   methods), only objects with types derived from BCP_cut_algo
   can be created. Such objects are constructed
   either directly by the user or by the unpacking functions of the
   \c BCP_xx_user classes. */

class BCP_cut_algo : public BCP_cut {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_cut_algo();
   /** The copy constructor is declared but not defined to disable it. */
   BCP_cut_algo(const BCP_cut_algo&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_cut_algo& operator=(const BCP_cut_algo&);
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** This constructor just sets the data members to the given values. See
       also the constructor of BCP_cut. */
   BCP_cut_algo(const double lb, const double ub) : BCP_cut(lb, ub) {}
   /** The destructor deletes the object. */
   virtual ~BCP_cut_algo() = 0;
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return \c BCP_AlgoObj indicating that the object is an
       algorithmic cut. */
   inline BCP_object_t obj_type() const   { return BCP_AlgoObj; }
   /*@}*/
};

//#############################################################################
//#############################################################################

/**
   This class is just a collection of pointers to cuts with a number of
   methods to manipulate these cuts and/or select certain entries. */

class BCP_cut_set : public BCP_vec<BCP_cut*> {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_cut_set(const BCP_cut_set&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_cut_set& operator=(const BCP_cut_set&);
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** The default constructor creates a cut set with no cuts in it. */
    BCP_cut_set() {}
   /** The destructor empties the cut set. <em>NOTE</em>: the destructor does
       NOT delete the cuts the members of the cut set point to. */
   ~BCP_cut_set() {}
   /*@}*/


   /**@name Modifying methods */
   /*@{*/
   /** Append the cuts in the vector <code>x</code> to the end of the cut
       set. */
   inline void append(const BCP_vec<BCP_cut*>& x) {
      BCP_vec<BCP_cut*>::append(x);
   }
   /** Append the cuts in <code>[first, last)</code> to the end of the cut
       set. */
   inline void append(BCP_cut_set::const_iterator first,
		      BCP_cut_set::const_iterator last){
      BCP_vec<BCP_cut*>::append(first, last);
   }

   /** Set the lower/upper bound pairs of the entries given by the contents of
       <code>pos</code> to the values in
       <code>[bounds, bounds + pos</code>. <code>size())</code>. */
   void set_lb_ub(const BCP_vec<int>& pos,
		  BCP_vec<double>::const_iterator bounds);
   /** Set the lower/upper bound pairs and the stati of the first
       <code>cc</code>. <code>size()</code> entries to the triplets given in
       the vector. 
       This method is invoked when the cut set is all the cuts in the current
       formulation and we want to change the triplets for the core cuts, which
       are at the beginning of that cut set. */
   void set_lb_ub_st(const BCP_vec<BCP_obj_change>& cc);
   /** Set the lower/upper bound pairs and the stati of the entries given by
       the content of <code>[pos, pos + cc</code>. <code>size())</code> to the
       triplets contained in <code>cc</code>. */
   void set_lb_ub_st(BCP_vec<int>::const_iterator pos,
		     const BCP_vec<BCP_obj_change>& cc);
   /*@}*/
#if 0
   //--------------------------------------------------------------------------
   /**@name Methods that select a subset of the cuts
      These methods fill up <code>coll</code> with the indices of the
      selected cuts. A cut is selected if it 
      <ol>
        <li> is not already free
	<li> is not non-removable
	<li> has been ineffective for at least <code>ineff_limit</code>
	     iterations
        <li> is ineffective now according to the criteria of the method.
      </ol>
      The first cut tested is the <code>first_to_check</code>-th one. (This
      argument is there so that we can skip the core cuts easily.) Also, for
      ineffective cuts the effectiveness count is decreased, for effective
      cuts it is increased. */
   /*@{*/
   /** Those cuts are considered ineffective whose corresponding slack value
       is nonzero (greater than <code>etol</code>). */
   void nonzero_slack(int first_to_check, const double * slacks,
		      const double etol, const int ineff_limit,
		      BCP_vec<int>& coll);
   /** Those cuts are considered ineffective whose corresponding dual value
       is zero (its absolute value is less than <code>etol</code>). */
   void zero_dual(int first_to_check, const double * duals,
		  const double etol, const int ineff_limit,
		  BCP_vec<int>& coll);
   /*@}*/
#endif
   /**@name Methods related to deleting cuts from the cut set */
   /*@{*/
#if 0
   /** Collect the indices of the cuts marked to be removed. Since core cuts
       are never removed, we pass the number of core cuts in the first
       argument to speed up things a little. */
   void deletable(const int bcutnum, BCP_vec<int>& collection) const;
#endif
   /** Move the cut pointers whose indices are listed in
       <code>deletable_cuts</code> into the <code>pool</code>. Note that this
       method does NOT compress the cut set, it merely replaces the cut
       pointers with 0 pointers. */
   void move_deletable_to_pool(const BCP_vec<int>& deletable_cuts,
			       BCP_vec<BCP_cut*>& pool);
   /*@}*/
   //-----------------------------------------------------------------------
};

#endif
