// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROBLEM_CORE_H
#define _BCP_PROBLEM_CORE_H

// This file is fully docified.

#include "BCP_vector.hpp"
#include "BCP_matrix.hpp"
#include "BCP_buffer.hpp"
#include "BCP_enum.hpp"
#include "BCP_obj_change.hpp"

//#############################################################################

class BCP_var_core;
class BCP_cut_core;
class BCP_indexed_pricing_list;

class BCP_buffer;

class BCP_internal_brobj;

//#############################################################################

/**
  This class describes the core of the MIP problem, the variables/cuts in it
  as well as the matrix corresponding the core variables and cuts. Core cuts
  and variables never leave the formulation. */

class BCP_problem_core{
private:
  /**@name Private and disabled methods */
  /*@{*/
    /** Delete all data members. This method purges the pointer vector members,
	i.e., deletes the object the pointers in the vectors point to. */
    inline void clear();
    /** The copy constructor is declared but not defined to disable it. */
    BCP_problem_core(const BCP_problem_core&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_problem_core& operator=(const BCP_problem_core&);
  /*@}*/
public:
  /**@name Data members */
  /*@{*/
    /** A vector of pointers to the variables in the core of the problem. These
	are the variables that always stay in the problem formulation. */
    BCP_vec<BCP_var_core*> vars;
    /** A vector of pointers to the cuts in the core of the problem. These are
        the cuts that always stay in the problem formulation. */
    BCP_vec<BCP_cut_core*> cuts;
    /** A pointer to the constraint matrix corresponding to the core variables
        and cuts. */
    BCP_lp_relax* matrix;
  /*@}*/
public:
  /**@name Constructors and destructor */
  /*@{*/
    /** The default constructor creates an empty core description: no
        variables/cuts and an empty matrix. */
    BCP_problem_core();
    /** This constructor "takes over" the arguments. The created core
	description will have the content of the arguments in its data members
	while the arguments lose their content. */
    BCP_problem_core(BCP_vec<BCP_var_core*>& v, BCP_vec<BCP_cut_core*>& c,
		     BCP_lp_relax*& m) : vars(), cuts(), matrix(m) {
      vars.swap(v);
      cuts.swap(c);
      m = 0;
    }
    /** The desctructor deletes all data members. */
    ~BCP_problem_core();
  /*@}*/

  /**@name Query methods */
  /*@{*/
  /** Return the number of variables in the core. */
    inline size_t varnum() const { return vars.size(); }
    /** Return the number of cuts in the core. */
    inline size_t cutnum() const { return cuts.size(); }
  /*@}*/

  /**@name Packing and unpacking methods */
  /*@{*/
    /** Pack the contents of the core description into the buffer. */
    void pack(BCP_buffer& buf) const; // *INLINE ?*
    /** Unpack the contents of the core description from the buffer. */
    void unpack(BCP_buffer& buf); // *INLINE ?*
  /*@}*/
};

//#############################################################################

// The following class holds the change in the core. It may be WrtParent,
// WrtCore or Explicit. In the latter case indices is empty.

/**
   This class describes changes in the core of the problem. While the set of
   core variables and cuts always remains the same, their bounds and stati may
   change during processing. An object of this type may store the description
   of the bounds and stati of the core of three possible ways:
   <ul>
     <li> explicitly: the bounds and stati of everything in the core is stored
          in this class. In this case <code>..._pos</code> is empty and
	  <code>..._ch</code> stores the bounds and stati.
     <li> with respect to parent: this is the case when a sequence of core
          changes have to be applied to get the current state. Each core
	  change in the sequence describes the changes to be done after the
	  previous change has been applied.
     <li> with respect to core: the current state of the core is given as one
          set of changes with respect to the very original state of the core.
   </ul>
   This class is internal to the framework, the user shouldn't worry about it. 
*/

class BCP_problem_core_change{
private:
  /**@name Private and disabled methods */
  /*@{*/
    /** Clear all vector data members. */
    inline void clear();
    /** The copy constructor is disabled by declaring it private and not
        defining it. */
    BCP_problem_core_change(const BCP_problem_core_change&); // disabled
    /** The assignment operator is disabled by declaring it private and not
        defining it. */
    BCP_problem_core_change& operator=(const BCP_problem_core_change& x);
  /*@}*/
   
public:
  /**@name Data members */
  /*@{*/
    /** Describes how the change is stored. The interpretation of the other
        data members depends on the storage type.
        <ul>
          <li><code>BCP_Storage_NoData</code>: when no data is stored (i.e.,
    	      the change is not described yet) the other members are undefined.
          <li><code>BCP_Storage_WrtCore</code>: with respect to core storage
    	      (as explained in the class description).
    	  <li><code>BCP_Storage_WrtParent</code>: with respect to parent
	      storage (as explained in the class description).
          <li><code>BCP_Storage_Explicit</code>: Explicit storage (as explained
    	      in the class description).
        </ul>
    */
    BCP_storage_t _storage;
    /** The positions of the core variables (in the <code>vars</code> member of
        \URL[<code>BCP_problem_core</code>]{BCP_problem_core.html}) whose
	bounds and/or stati have changed. */ 
    BCP_vec<int>            var_pos;
    /** The new lb/ub/status triplet for each variable for which any of those
        three have changed. */
    BCP_vec<BCP_obj_change> var_ch;
    /** The positions of the core cuts (in the <code>cuts</code> member of 
        \URL[<code>BCP_problem_core</code>]{BCP_problem_core.html}) whose
	bounds and/or stati have changed. */
    BCP_vec<int>            cut_pos;
    /** The new lb/ub/status triplet for each cut for which any of those three
        have changed. */
    BCP_vec<BCP_obj_change> cut_ch;
  /*@}*/

public:
  /**@name Constructors and destructor */
  /*@{*/
    /** This constructor creates a core change with the given storage. The
        other members are empty, i.e., the created object contains NoData; is
        Explicit and empty; is WrtCore or WrtParent but there's no change. <br>
        Note that the argument defaults to WrtCore, this constructor is
        the default constructor as well, and as the default constructor it
        creates a "no change WrtCore" core change. */
    BCP_problem_core_change(BCP_storage_t store = BCP_Storage_WrtCore) :
       _storage(store), var_pos(), var_ch(), cut_pos(), cut_ch() {}
  
    /** This constructor creates an Explicit core change description. The first
        <code>bvarnum</code> variables in <code>vars</code> are the core
        variables. Similarly for the cuts. */
    BCP_problem_core_change(int bvarnum, BCP_var_set& vars,
			    int bcutnum, BCP_cut_set& cuts);

    /** Create a core change describing the changes from <code>old_bc</node> to
        <code>new_bc</code>. Both core change arguments must have explicit
        storage. The only reason for passing <code>storage</code> as an
        argument (and not setting it automatically to WrtParent) is that
        <code>old_bc</code> might be the original core in which case storage
        must be set to WrtCore. */
    BCP_problem_core_change(BCP_storage_t storage,
			    BCP_problem_core_change& ocore,
			    BCP_problem_core_change& ncore);

    /** The destructor deletes all data members. */
    ~BCP_problem_core_change() {}
  /*@}*/

  /**@name Query methods */
  /*@{*/
    /** Return the storage type. */
    inline BCP_storage_t storage() const { return _storage; }
    /** Return the number of changed variables (the length of the array 
        <code>var_ch</code>). */
    inline size_t varnum() const { return var_ch.size(); }
    /** Return the number of changed cuts (the length of the array 
        <code>cut_ch</code>). */
    inline size_t cutnum() const { return cut_ch.size(); }
  /*@}*/
  
  /**@name Modifying methods */
  /*@{*/
    /** Set the core change description to be an explicit description (in the
        form of a change) of the given <code>core</code>. */
    BCP_problem_core_change& operator=(const BCP_problem_core& core);
  
    /** If the current storage is not already explicit then replace it with an
	explicit description of the core generated by applying the currently
	stored changes to <code>expl_core</code> (which of course, must be
	explicitly stored). <br>
        NOTE: An exception is thrown if the currently stored change is not
        stored as explicit or WrtCore; or the storage of
	<code>expl_core</code> is not explicit. */
    void ensure_explicit(const BCP_problem_core_change& expl_core);
  
    /** Replace the current explicitly stored core change with one stored with
        respect to the explicitly stored original core change in
        <code>orig_core</code> if the WrtCore description is shorter (requires
        less space to pack into a buffer).<br>
        NOTE: An exception is thrown if either the current storage or that of
        <code>expl_core</code> is not explicit. */
    void make_wrtcore_if_shorter(const BCP_problem_core_change& orig_core);
  
    /** Swap the contents of the current core change with that of
        <code>other</code>. */
    void swap(BCP_problem_core_change& other);
  
    /** Update the current change according to <code>core_change</code>. If the
        storage type of <code>core_change</code> is
        <ul>
           <li> NoData or Explicit then it is simply copied over into this
  	        change.
           <li> WrtParent then the current change is supposed to be the parent
	        and this explicitly stored core change will be updated with the
		changes in <code>core_change</code> (an exception is thrown if
		the current change is not explicitly stored).
           <li> WrtCore storage then the current change will be replaced by
  	       <code>expl_core</code> updated with <code>core_change</code>
  	       (the storage of <code>expl_core</code> must be explicit or an
  	       exception is thrown).
        </ul>
        NOTE: When this function exits the stored change will have either
        explicit or NoData storage. */
    void update(const BCP_problem_core_change& expl_core,
		const BCP_problem_core_change& core_change);
  /*@}*/

  /**@name Packing and unpacking */   
  /*@{*/
    /** Return the buffer size needed to pack the data in the core change. */
    int pack_size() const;
    /** Pack the core change into the buffer. */
    void pack(BCP_buffer& buf) const;
    /** Unpack the core change data from the buffer. */
    void unpack(BCP_buffer& buf);
  /*@}*/
};

#endif
