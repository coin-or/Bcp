// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_VG_USER_H
#define _BCP_VG_USER_H

// This file is fully docified.

#include "BCP_math.hpp"
#include "BCP_vector.hpp"
#include "BCP_vg_param.hpp"
#include "BCP_string.hpp"

class BCP_vg_prob;
class BCP_buffer;
class BCP_cut;
class BCP_cut_algo;
class BCP_var;
class BCP_var_algo;

/**
   The BCP_vg_user class is the base class from which the user
   can derive a problem specific class to be used in the Cut Generator process.

   In that derived class the user can store data to be used in the methods she
   overrides. Also that is the object the user must return in the
   USER_initialize::vg_init() method.
   
   There are two kind of methods in the class. The non-virtual methods are
   helper functions for the built-in defaults, but the user can use them as
   well. The virtual methods execute steps in the BCP algorithm where the user
   might want to override the default behavior.

   The default implementations fall into three major categories. 
   <ul>
     <li> Empty; doesn't do anything and immediately returns (e.g., 
          unpack_module_data()).
          <code>unpack_module_data.</code>)
     <li> There is no reasonable default, so throw an exception. This happens
          if the parameter settings drive the flow of in a way that BCP can't
	  perform the necessary function. This behavior is correct since such
	  methods are invoked only if the parameter settings drive the flow of
	  the algorithm that way, in which case the user better implement those
	  methods. (At the momemnt there is no such method in VG.)
     <li> A default is given. Frequently there are multiple defaults and
          parameters govern which one is selected (e.g.,
	  unpack_dual_solution()).
   </ul>
*/

class BCP_vg_user {
private:
  BCP_vg_prob * p;
public:
  /**@name Methods to set and get the pointer to the BCP_vg_prob
     object. It is unlikely that the users would want to muck around with
     these (especially with the set method!) but they are here to provide
     total control.
  */
  /*@{*/
    /// Set the pointer
    void setVgProblemPointer(BCP_vg_prob * ptr) { p = ptr; }
    /// Get the pointer
    BCP_vg_prob * getVgProblemPointer() { return p; }
  /*@}*/

  /**@name Informational methods for the user. */
  /*@{*/
    /** Return what is the best known upper bound (might be BCP_DBL_MAX) */
    double upper_bound() const;
    /** Return the phase the algorithm is in */
    int current_phase() const;
    /** Return the level of the search tree node for which cuts are being
        generated */
    int current_level() const;
    /** Return the internal index of the search tree node for which cuts are
	being generated */
    int current_index() const;
    /** Return the iteration count within the search tree node for which cuts
	are being generated */
    int current_iteration() const;
  /*@}*/
  
  /**@name Methods to get/set BCP parameters on the fly */
  /*@{*/
    ///
    char              get_param(const BCP_vg_par::chr_params key) const;
    ///
    int               get_param(const BCP_vg_par::int_params key) const;
    ///
    double            get_param(const BCP_vg_par::dbl_params key) const;
    ///
    const BCP_string& get_param(const BCP_vg_par::str_params key) const;

    ///
    void set_param(const BCP_vg_par::chr_params key, const bool val);
    /// 
    void set_param(const BCP_vg_par::chr_params key, const char val);
    ///
    void set_param(const BCP_vg_par::int_params key, const int val);
    ///
    void set_param(const BCP_vg_par::dbl_params key, const double val);
    ///
    void set_param(const BCP_vg_par::str_params key, const char * val);
  /*@}*/

  //---------------------------------------------------------------------------
  /** Pack the argument into the message buffer and send it to the sender of
      the LP solution. Whenever the user generates a variable in the
      generate_vars() method she should invoke this method to immediately send
      off the variable to the LP process. */
   void send_var(const BCP_var& var);

  //===========================================================================
  /**@name Constructor, Destructor */
  /*@{*/
    BCP_vg_user() : p(0) {}
    /** Being virtual, the destructor invokes the destructor for the real type
	of the object being deleted. */
    virtual ~BCP_vg_user() {}
  /*@}*/
  //===========================================================================
  // Here are the user defined functions. For each of them a default is given
  // which can be overridden when the concrete user class is defined.
  //===========================================================================
  /** Unpack the initial information sent to the Variable Generator process
      by the Tree Manager. This information was packed by the method
      BCP_tm_user::pack_module_data() invoked with \c BCP_ProcessType_VG as
      the third (target process type) argument. <br>
      Default: empty method. */
  virtual void
  unpack_module_data(BCP_buffer& buf);

  //---------------------------------------------------------------------------
  /** Unpack the LP solution arriving from the LP process. This method is
      invoked only if the user packs the info necessary for variable
      generation by herself, i.e., she overrides the
      BCP_lp_user::pack_dual_solution() method. If that's
      the case the user has to unpack the same info she has packed in the LP
      process. */
  virtual void
  unpack_dual_solution(BCP_buffer& buf);

  //---------------------------------------------------------------------------
  /** Perform the actual variable generation. Whenever a variable is
      generated, the user should invoke the send_var() method to send
      the generated variable back to the LP process. */
  virtual void
  generate_vars(BCP_vec<BCP_cut*>& cuts, BCP_vec<double>& pi);

  //---------------------------------------------------------------------------
  /** Pack an algorithmic variable into the buffer. When the user generates a
      variable in generate_vars() and invokes send_var() to send it off, this
      method will be invoked to pack the variable into the buffer if the
      variable is algorithmic. BCP knows how to pack indexed variables. */
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);

  //---------------------------------------------------------------------------
  /** Unpack an algorithmic cut from the buffer. This method is invoked
      when the user does use algorithmic cuts and she did not override
      the unpack_dual_solution() method. */
  virtual BCP_cut_algo*
  unpack_cut_algo(BCP_buffer& buf);
};

//#############################################################################

#endif
