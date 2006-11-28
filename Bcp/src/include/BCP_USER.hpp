// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_USER_H
#define _BCP_USER_H

// This file is prepared for doxygen.

#include "BCP_error.hpp"
#include "BCP_message_single.hpp"

class BCP_buffer;

//#############################################################################

class BCP_user_data {
public:
	virtual ~BCP_user_data() {}
};

//#############################################################################

class BCP_tm_user;
class BCP_lp_user;
class BCP_vg_user;
class BCP_cg_user;
class BCP_vp_user;
class BCP_cp_user;

class BCP_tm_prob;
class BCP_lp_prob;
class BCP_vg_prob;
class BCP_cg_prob;
class BCP_vp_prob;
class BCP_cp_prob;

class BCP_proc_id;
class BCP_message_environment;

/** This class is an abstract base class for the initializer class the user
    has to provide. The user will have to return an instance of the
    initializer class when the BCP_user_init()
    function is invoked. The member methods of that instance will be invoked
    to create the various objects (well, pointers to them) that are
    used/controlled by the user during the course of a run. */
class USER_initialize {
public:
   /**@name Destructor */
   /*@{*/
   /** virtual destructor */
   virtual ~USER_initialize() {}
   /*@}*/

   //--------------------------------------------------------------------------
   /**@name Message passing environment */
   /*@{*/
   /** Create a message passing environment. Currently implemented
       environments are single and PVM, the default is single. To use PVM, the
       user has to override this method and return a pointer to a new
       <code>BCP_pvm_environment</code> object. */
   virtual BCP_message_environment * msgenv_init(int argc, char* argv[]);
   /*@}*/

   //--------------------------------------------------------------------------
   /**@name User object initialization in the processes
      These methods are invoked when the appropriate process starts. They have
      to return pointers to user objects, i.e., objects derived from the
      return value of each init method. Those objects can be used by the user
      to store information about the problem; information that will be used in
      the member methods of the user objects. */
   /*@{*/
   ///
   virtual BCP_tm_user * tm_init(BCP_tm_prob& p,
				 const int argnum,
				 const char * const * arglist) {
      throw BCP_fatal_error("USER_initialize::tm_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   ///
   virtual BCP_lp_user * lp_init(BCP_lp_prob& p) {
      throw BCP_fatal_error("USER_initialize::lp_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   ///
   virtual BCP_vg_user * vg_init(BCP_vg_prob& p) {
      throw BCP_fatal_error("USER_initialize::vg_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   ///
   virtual BCP_cg_user * cg_init(BCP_cg_prob& p) {
      throw BCP_fatal_error("USER_initialize::cg_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   ///
   virtual BCP_vp_user * vp_init(BCP_vp_prob& p) {
      throw BCP_fatal_error("USER_initialize::vp_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   ///
   virtual BCP_cp_user * cp_init(BCP_cp_prob& p) {
      throw BCP_fatal_error("USER_initialize::cp_init() missing.\n");
      return 0; // to satisfy aCC on HP-UX
   }
   /*@}*/
};

//#############################################################################

/** This is the function the user must invoke when (s)he is ready to turn
    contrl over to BCP. The arguments of the function are the command line
    arguments and a pointer to an initializer object. That object should be
    derived from the USER_initialize class. Its member methods will be invoked
    to initialize the user controlled parts of the branch-and-cut-and-price
    procedure. */
int bcp_main(int argc, char* argv[], USER_initialize* user_init);

#endif
