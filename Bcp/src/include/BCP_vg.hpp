// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_VG_H
#define _BCP_VG_H

// This file is fully docified.

#include <cfloat>

#include "BCP_math.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_vg_param.hpp"
#include "BCP_parameters.hpp"
#include "BCP_process.hpp"

class BCP_vg_user;
class BCP_message_environment;
class BCP_proc_id;
class BCP_problem_core;
class BCP_var;
class BCP_cut;

/**
   This class is the central class of the Variable Generator process. Only one
   object of this type is created and that holds all the data in the VG
   process. A reference to that object is passed to (almost) every function
   (or member method) that's invoked within the VG process. 
*/

class BCP_vg_prob : public BCP_process {
private:
   /**@name Disabled members */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_vg_prob(const BCP_vg_prob&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_vg_prob& operator=(const BCP_vg_prob&);
   /*@}*/

public:
   /**@name Data members */
   /*@{*/
   // User provided members--------------------------------------------------
   /** The user object holding the user's data. This object
       is created by a call to the appropriate member of
       \URL[<code>USER_initialize</code>]{USER_initialize.html}. */
   BCP_vg_user* user;
   /** The message passing environment. This object
       is created by a call to the appropriate member of
       \URL[<code>USER_initialize</code>]{USER_initialize.html}. */
   BCP_message_environment* msg_env;

   /** The message buffer of the Variable Generator process. */
   BCP_buffer  msg_buf;

   /** The parameters controlling the Variable Generator process. */
   BCP_parameter_set<BCP_vg_par> par;

   /** The description of the core of the problem. */
   BCP_problem_core* core;
   
   /** The proc id of the Tree Manager. */
    //   BCP_proc_id* tree_manager;

   /** The best currently known upper bound. */
   double upper_bound;

   // the cuts and corresponding dual values in the LP formulation that were
   // sent over to generate variables from. Also, the sender and which node
   // in which iteration do these cuts/duals belong to.
   /** Variables are to be generated for the LP solution given by these cuts
       and their values (next member). Not all cuts need to be listed (e.g.,
       list only those that have nonzero dual values in the current solution).
       \sa BCP_lp_user::pack_dual_solution()
   */
   BCP_vec<BCP_cut*> cuts;
   /** The dual values corresponding to the cuts above. */
   BCP_vec<double>   pi;
   /** The process id of the LP process that sent the solution. */
   BCP_proc_id*      sender;

   /** The phase the algorithm is in. */
   int phase;
   /** The level of search tree node where the solution was generated. */
   int node_level;
   /** The index of search tree node where the solution was generated. */
   int node_index;
   /** The iteration within the search tree node where the solution was
       generated. */
   int node_iteration;

   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** The default constructor. Initializes every data member to a natural
       state. */
   BCP_vg_prob(BCP_proc_id* my_id, BCP_proc_id* parent);
   /** The destructor deletes everything. */
   virtual ~BCP_vg_prob();
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return true/false indicating whether any upper bound has been found. */
   inline bool has_ub() const { return upper_bound < BCP_DBL_MAX / 10; }
   /** Return the current upper bound (<code>BCP_DBL_MAX/10</code> if there's
       no upper bound found yet.) */
   inline double ub() const   { return upper_bound; }
   /*@}*/

   /**@name Modifying methods */
   /*@{*/
   /** Set the upper bound equal to the argument. */
   inline void ub(const double bd) { upper_bound = bd; }

   /** Test if there is a message in the message queue waiting to be
       processed. */
   bool probe_messages();
   /*@}*/
   
   /** Unpack a cut. Invoked from the built-in
       BCP_vg_user::unpack_dual_solution(). */
   BCP_cut* unpack_cut();
   //--------------------------------------------------------------------------
   virtual BCP_buffer& get_message_buffer() { return msg_buf; }
   virtual void process_message();
};

// This function is used only internally.
bool
BCP_vg_process_message(BCP_vg_prob& p, BCP_buffer& buf);

#endif
