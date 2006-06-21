// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_VAR_H
#define _BCP_VAR_H

// This file is fully docified.

//#############################################################################

#include "BCP_error.hpp"
#include "BCP_enum.hpp"
#include "BCP_vector.hpp"
#include "BCP_obj_change.hpp"

//#############################################################################

// Generic variable definition.

/**
   Abstract base class that defines members common to all types of variables.
   Classes describing the three types of variables
   (\link BCP_var_core core \endlink, \link BCP_var_indexed indexed \endlink
   and \link BCP_var_algo algorithmic \endlink) are derived from this class.
   No object of type BCP_var can exist (having purely virtual
   members in the class enforces this restriction). */

class BCP_var{
private:
  /**@name Disabled methods */
  /*@{*/
    /** The default constructor is declared but not defined to disable it.
        (Just to make certain that the compiler doesn't generate it.) */
    BCP_var();
    /** The copy constructor is declared but not defined to disable it. */
    BCP_var(const BCP_var&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_var& operator=(const BCP_var&);
  /*@}*/

private:
  /**@name Private data members
     These are used only by BCP, the user need not worry about them. */
  /*@{*/
    /** The internal, unique index of the variable. */
    int  _bcpind;
    /** The status of the variable. */ 
    BCP_obj_status _status;
  /*@}*/

protected:
  /**@name Protected data members */
  /*@{*/
    /** The integrality type of the variable. */
    BCP_var_t _var_type;
    /** The objective coefficient. */
    double    _obj;
    /** Lower bound on the value the variable can take. */
    double    _lb;
    /** Upper bound on the value the variable can take. */
    double    _ub;
  /*@}*/

public:
  /**@name Constructor and destructor 
     Note that there is no default constructor. There is no such thing as
     "default variable". */
  /*@{*/
    /** The constructor sets the internal index of the variable to zero and the
        other data members to the given arguments. */
    BCP_var(const BCP_var_t var_type,
	    const double obj, const double lb, const double ub) :
       _bcpind(0), _status(BCP_ObjNoInfo),
       _var_type(var_type), _obj(obj), _lb(lb), _ub(ub) {}
    /** The destructor is virtual so that the appropriate destructor is invoked
        for every variable. */
    virtual ~BCP_var() {}
  /*@}*/

  /**@name Query methods */
  /*@{*/
    /** Return the type of the variable. */
    virtual BCP_object_t obj_type() const = 0;
    /** Return the integrality type of the variable. */
    inline BCP_var_t var_type() const   { return _var_type; }
    /** Return the objective coefficient. */
    inline double obj() const           { return _obj; }
    /** Return the lower bound. */
    inline double lb() const            { return _lb; }
    /** Return the upper bound. */
    inline double ub() const            { return _ub; }
    /** Return the internal index of the variable. */
    inline int bcpind() const { return _bcpind; }

    /**@name Query methods about the status of the variable */
    /*@{*/
      /** Return the status of the variable. */
      inline BCP_obj_status status() const   { return _status; }
      /** Return whether the variable should be sent to the Variable Pool
    	process. (Assuming that it stays in the formulation long enough to
    	qualify to be sent to the Variable Pool at all. */
      inline bool dont_send_to_pool() const {
        return _status & BCP_ObjDoNotSendToPool ? true : false;
      }
      /** Return whether the variable is fixed or not. */
      inline bool is_fixed() const {
        return (_status & BCP_ObjInactive) != 0;
      }
      /** Return whether the variable is fixed to zero or not. */
      inline bool is_fixed_to_zero() const {
        return (_status & BCP_ObjInactive) && _lb == 0;
      }
      /** Return whether the variable is marked NotRemovable. Examples of such
          variables include ??? */
      inline bool is_non_removable() const {
        return (_status & BCP_ObjNotRemovable) ? true : false;
      }
      /** Return whether the variable is removable from the formulation at the
          time of the query. (It is if it is not non_removable and it is fixed
	  to zero.) */
      inline bool is_removable() const {
        return (_status & BCP_ObjNotRemovable) ? false : is_fixed_to_zero();
      }
      /** Return whether the variable must be removed from the formulation.
	  There are very few circumstances when this flag is set; all of them
	  are completely internal to BCP. */
      inline bool is_to_be_removed() const {
        return (_status & BCP_ObjToBeRemoved) != 0;
      }
    /*@}*/
  /*@}*/

  /**@name Modifying methods */
  /*@{*/
    /** Test (and set) whether the var is fixed (inactive) */
    inline void test_inactive() {
      if (_ub - _lb < 1e-8)
	_status = static_cast<BCP_obj_status>(_status | BCP_ObjInactive);
    }
    /** Set the integrality type of the variable. */
    inline void set_var_type(const BCP_var_t type) { _var_type = type; }
    /** Set the objective coefficient. */
    inline void set_obj(const double obj)          { _obj = obj; }
    /** Set the lower bound. */
    inline void set_lb(const double lb)            {
      _lb = lb;
      test_inactive();
    }
    /** Set the upper bound. */
    inline void set_ub(const double ub)            {
      _ub = ub;
      test_inactive();
    }
    /** Set the lower/upper bounds and the status of the variable
        simultaneously to the values given in the data members of the argument.
    */ 
    inline void change_lb_ub_st(const BCP_obj_change& change) {
       _lb = change.lb;
       _ub = change.ub;
       _status = change.stat;
       test_inactive();
    }
    /** Change the lower and upper bounds to the given values. */
    inline void change_bounds(const double lb, const double ub) {
       _lb = lb;
       _ub = ub;
       test_inactive();
    }
    /** Set the internal index of the variable. */
    inline void set_bcpind(const int bcpind)  { _bcpind = bcpind; }

    /**@name Status modifying methods */
    /*@{*/
      /** Set the status of the variable. */
      inline void set_status(const BCP_obj_status status) { _status = status; }
      /** Set/unset the flag controlling whether the variable could be sent to
          the Variable Pool process. */
      inline void dont_send_to_pool(bool flag) {
         _status =
	   static_cast<BCP_obj_status>(flag ?
				       _status | BCP_ObjDoNotSendToPool :
				       _status & ~BCP_ObjDoNotSendToPool);
      }
      /** Mark the variable as active. Note that when this method is invoked
	  the lp formulation must be modified as well: the original bounds of
	  the variable must be reset. */
      inline void make_active() {
         _status = static_cast<BCP_obj_status>(_status & ~BCP_ObjInactive);
      }
      /** Mark the variable as NotRemovable. */
      inline void make_non_removable() {
         _status =
	   static_cast<BCP_obj_status>((_status & ~BCP_ObjToBeRemoved) |
				       BCP_ObjNotRemovable);
      }
      /** Mark the variable as ToBeRemoved. It will actually be removed
          immediately after all variables that have to be marked this way are
          marked. */ 
      inline void make_to_be_removed() {
         _status = BCP_ObjToBeRemoved;
      }
  /*@}*/

  /**@name Display */
  /*@{*/
  /** Display the object type, internal index, and the value given in the
      argument. (This value is usually the variable's value in an LP
      solution.) */
    void display(const double val) const;
  /*@}*/
};

//#############################################################################
//#############################################################################

/**
   Core variables are the variables that always stay in the LP formulation.
   Therefore the data members in the base class are quite sufficient to
   describe the variable. The only thing that has to be done here is
   overriding the pure virtual method obj_type(). */

class BCP_var_core : public BCP_var {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_var_core();
   /** The assignment operator is declared but not defined to disable it. */
   BCP_var_core& operator=(const BCP_var_core&);
   /*@}*/

public:
   /**@name Constructors and destructor */
   /*@{*/
   /** The copy constructor makes a replica of the argument. */
   BCP_var_core(const BCP_var_core& x) :
      BCP_var(x._var_type, x._obj, x._lb, x._ub) {
      set_bcpind(x.bcpind());
      set_status(x.status());
   }
   /** This constructor just sets the data members to the given values. See
       also the constructor BCP_var. */
   BCP_var_core(const BCP_var_t var_type,
		const double obj, const double lb, const double ub) :
      BCP_var(var_type, obj, lb, ub) {}
   /** The destructor deletes the object. */
   ~BCP_var_core() {}
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return BCP_CoreObj indicating that the object is a core variable. */
   inline BCP_object_t obj_type() const  { return BCP_CoreObj; }
   /*@}*/
};

//#############################################################################

/** Implementation-wise indexed variables differ from core variables only in
    having a user given index. This index is a unique user
    specified ID which enables the user to expand the variable into a column
    that can be added to the LP formulation. */

class BCP_var_indexed : public BCP_var {
private:
   /**@name Data members */
   /*@{*/
   /** The only (not inherited) data member is the user given index of the 
       variable. */
   int _index;  
   /*@}*/
private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_var_indexed();
   /** The copy constructor is declared but not defined to disable it. */
   BCP_var_indexed(const BCP_var_indexed&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_var_indexed& operator=(const BCP_var_indexed&);
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** This constructor just sets the data members to the given values. See
       also the constructor of BCP_var. */
   BCP_var_indexed(const int index, const BCP_var_t var_type,
		   const double obj, const double lb, const double ub) :
      BCP_var(var_type, obj, lb, ub), _index(index) {}
   /** The destructor deletes the object. */
   ~BCP_var_indexed() {}
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return the user index of the variable. */
   inline int index() const               { return _index; }
   /** Return BCP_IndexedObj indicating that the object is an indexed
       variable. */
   inline BCP_object_t obj_type() const   { return BCP_IndexedObj; }
   /*@}*/
};

//#############################################################################

// This is the class the user should derive his/her own algorithmic vars

/** 
   This is the class from which the user should derive her own algorithmic
   variables. Note that such an object cannot be constructed (it has pure
   virtual methods), only objects with types derived from
   BCP_var_algo can be created. Such objects are constructed
   either directly by the user or by the unpacking functions of the
   \c BCP_xx_user classes. */

class BCP_var_algo : public BCP_var {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The default constructor is declared but not defined to disable it. */
   BCP_var_algo();
   /** The copy constructor is declared but not defined to disable it. */
   BCP_var_algo(const BCP_var_algo&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_var_algo& operator=(const BCP_var_algo&);
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** This constructor just sets the data members to the given values. See
       also the constructor of BCP_var. */
   BCP_var_algo(const BCP_var_t var_type,
		const double obj, const double lb, const double ub) :
      BCP_var(var_type, obj, lb, ub) {}
   /** The destructor deletes the object. */
   virtual ~BCP_var_algo() = 0;
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return \c BCP_AlgoObj indicating that the object is an
       algorithmic variable. */
   inline BCP_object_t obj_type() const   { return BCP_AlgoObj; }
   /*@}*/
};

//#############################################################################
//#############################################################################

/**
   This class is just a collection of pointers to variables with a number of
   methods to manipulate these variables and/or select certain entries. */

class BCP_var_set : public BCP_vec<BCP_var*> {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_var_set(const BCP_var_set&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_var_set& operator=(const BCP_var_set&);
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** The default constructor creates a variable set with no variables in it.
    */ 
   BCP_var_set() {}
   /** The destructor empties the variable set. <em>NOTE</em>: the destructor
       does NOT delete the variables the members of the variable set point to.
   */ 
   ~BCP_var_set() {}
   /*@}*/

   /**@name Modifying methods */
   /*@{*/
   /** Append the variables in the vector <code>x</code> to the end of the
       variable set. */
   inline void append(const BCP_vec<BCP_var*>& x) {
      BCP_vec<BCP_var*>::append(x);
   }
   /** Append the variables in <code>[first, last)</code> to the end of the
       variable set. */
   inline void append(BCP_var_set::const_iterator first,
		      BCP_var_set::const_iterator last){
      BCP_vec<BCP_var*>::append(first, last);
   }

   /** Set the lower/upper bound pairs of the entries given by the contents of
       <code>pos</code> to the values in
       <code>[bounds, bounds + pos</code>. <code>size())</code>. */
   void set_lb_ub(const BCP_vec<int>& pos,
		  BCP_vec<double>::const_iterator bounds);
   /** Set the lower/upper bound pairs and the stati of the first
       <code>cc</code>. <code>size()</code> entries to the triplets given in
       the vector. 
       This method is invoked when the variable set is all the variables in
       the current formulation and we want to change the triplets for the core
       variables, which are at the beginning of that variable set. */
   void set_lb_ub_st(const BCP_vec<BCP_obj_change>& vc);
   /** Set the lower/upper bound pairs and the stati of the entries given by
       the content of <code>[pos, pos + cc</code>. <code>size())</code> to the
       triplets contained in <code>cc</code>. */
   void set_lb_ub_st(BCP_vec<int>::const_iterator pos,
		     const BCP_vec<BCP_obj_change>& vc);
   /*@}*/
   //--------------------------------------------------------------------------
   /**@name Methods related to deleting variables from the variable set */
   /*@{*/
   /** Collect the indices of the variables marked to be removed. Since core
       variables are never removed, we pass the number of core variables in
       the first argument to speed up things a little. */
   void deletable(const int bvarnum, BCP_vec<int>& collection);
   // *FIXME* shouldn't we keep a local var pool in the lp process and move
   // deletables to this pool? compare w/ cuts...
   /*@}*/
   //--------------------------------------------------------------------------
};

#endif
