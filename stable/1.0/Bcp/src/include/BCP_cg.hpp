// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_CG_H
#define _BCP_CG_H

// This file is fully docified.

#include <cfloat>

#include "BCP_math.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_cg_param.hpp"
#include "BCP_parameters.hpp"
#include "BCP_process.hpp"

class BCP_cg_user;
class BCP_message_environment;
class BCP_proc_id;
class BCP_problem_core;
class BCP_var;
class BCP_cut;

/**
   This class is the central class of the Cut Generator process. Only one
   object of this type is created and that holds all the data in the CG
   process. A reference to that object is passed to (almost) every function
   (or member method) that's invoked within the CG process. 
*/

class BCP_cg_prob : public BCP_process {
private:
   /**@name Disabled members */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_cg_prob(const BCP_cg_prob&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_cg_prob& operator=(const BCP_cg_prob&);
   /*@}*/

public:
   /**@name Data members */
   /*@{*/

   // User provided members ---------------------------------------------------
   /** The user object holding the user's data. This object
       is created by a call to the appropriate member of
       \URL[<code>USER_initialize</code>]{USER_initialize.html}. */
   BCP_cg_user* user;

   /** The message passing environment. This object
       is created by a call to the appropriate member of
       \URL[<code>USER_initialize</code>]{USER_initialize.html}. */
   BCP_message_environment* msg_env;

   /** The message buffer of the Cut Generator process. */
   BCP_buffer  msg_buf;

   /** The parameters controlling the Cut Generator process. */
   BCP_parameter_set<BCP_cg_par> par;

   /** The description of the core of the problem. */
   BCP_problem_core* core;
   
   /** The proc id of the tree manager. */
    //   BCP_proc_id* tree_manager;

   /** The best currently known upper bound. */
   double upper_bound;

   // the lp solution, its sender and at which node in which iteration the lp
   // solution was generated.
   /** Cuts are to be generated for the LP solution given by these variables
       and their values (next member). Not all variables need to be listed
       (e.g., list only those that have fractional values in current LP
       solution).
       \sa BCP_lp_user::pack_primal_solution()
   */
   BCP_vec<BCP_var*> vars;
   /** The primal values corresponding to the variables above. */
   BCP_vec<double>   x;
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
   BCP_cg_prob(BCP_proc_id* my_id, BCP_proc_id* parent);
   /** The destructor deletes everything. */
   virtual ~BCP_cg_prob();
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return true/false indicating whether any upper bound has been found. */
   inline bool has_ub() const { return upper_bound < BCP_DBL_MAX / 10; }
   /** Return the current upper bound (<code>BCP_DBL_MAX</code> if there's no
       upper bound found yet.) */
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

   /** Unpack a variable. Invoked from the built-in
       BCP_cg_user::unpack_primal_solution(). */
   BCP_var* unpack_var();
   //--------------------------------------------------------------------------
   virtual BCP_buffer& get_message_buffer() { return msg_buf; }
   virtual void process_message();
};

// This function is used only internally. 
bool
BCP_cg_process_message(BCP_cg_prob& p, BCP_buffer& buf);

#endif
