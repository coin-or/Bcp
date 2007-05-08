// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_USER_H
#define _BCP_USER_H

// This file is prepared for doxygen.

#include "CoinSmartPtr.hpp"
#include "BCP_error.hpp"
#include "BCP_message_single.hpp"
#include "BCP_functions.hpp"

class BCP_var_algo;
class BCP_cut_algo;
class BCP_warmstart;

//#############################################################################

class BCP_user_data : public Coin::ReferencedObject {
public:
    virtual ~BCP_user_data() {}
};

//#############################################################################

class BCP_user_class {
public:
    virtual ~BCP_user_class() {}
};

//#############################################################################

/**@name This abstract base class contains Methods that pack/unpack warmstart,
   var_algo and cut_algo objects.

   The packing methods take an object and a buffer as an argument and the user
   is supposed to pack the object into the buffer.

   The argument of the unpacking methods is just the buffer. The user is
   supposed to return a pointer to the unpacked object.

   NOTE: within these methods the user can try to dynamic cast \c user_class
   into BCP_tm_user*, BCP_lp_user*, etc., and when the cast succeeds the user
   knows in which process the methods was called, and also, the user will have
   access to the other methods of that class.
*/

class BCP_user_pack {
public:
    /** A pointer ot the usr class of the process from which the methods of
	this class are invoked from. The user can try to dynamic cast \c
	user_class into BCP_tm_user*, BCP_lp_user*, etc., and when the cast
	succeeds the user knows in which process the methods was called, and
	also, the user will have access to the other methods of that class. */
    BCP_user_class* user_class;

public:
    virtual ~BCP_user_pack() {}

    //-------------------------------------------------------------------------
    /** Pack warmstarting information */
    virtual void
    pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf,
		   bool report_if_default = false)
    {
	if (report_if_default) {
	    printf("BCP_user_pack : default pack_warmstart() executed.\n");
	}
	BCP_pack_warmstart(ws, buf);
    }

    /** Unpack warmstarting information */
    virtual BCP_warmstart*
    unpack_warmstart(BCP_buffer& buf,
		     bool report_if_default = false)
    {
	if (report_if_default) {
	    printf("BCP_user_pack : default unpack_warmstart() executed.\n");
	}
	return BCP_unpack_warmstart(buf);
    }
    
    //-------------------------------------------------------------------------
    /** Pack an algorithmic variable */
    virtual void
    pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::pack_var_algo() invoked but not overridden!\n");
    }

    /** Unpack an algorithmic variable */
    virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::unpack_var_algo() invoked but not overridden!\n");
	return 0; // to satisfy aCC on HP-UX
    }
      
    //-------------------------------------------------------------------------
    /** Pack an algorithmic cut */
    virtual void pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::pack_cut_algo() invoked but not overridden!\n");
    }

    /** Unpack an algorithmic cut */
    virtual BCP_cut_algo* unpack_cut_algo(BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::unpack_cut_algo() invoked but not overridden!\n");
	return 0; // to satisfy aCC on HP-UX
    }

    //-------------------------------------------------------------------------
    /** Pack an user data */
    virtual void
    pack_user_data(const BCP_user_data* ud, BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::pack_user_data() invoked but not overridden!\n");
    }

    /** Unpack an user data */
    virtual BCP_user_data* unpack_user_data(BCP_buffer& buf)
    {
	throw BCP_fatal_error("\
BCP_user_pack::unpack_user_data() invoked but not overridden!\n");
    }
};

//#############################################################################

class BCP_tm_user;
class BCP_ts_user;
class BCP_lp_user;
class BCP_vg_user;
class BCP_cg_user;
class BCP_vp_user;
class BCP_cp_user;

class BCP_tm_prob;
class BCP_ts_prob;
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
   virtual BCP_ts_user * ts_init(BCP_ts_prob& p) {
      throw BCP_fatal_error("USER_initialize::ts_init() missing.\n");
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
   ///
   virtual BCP_user_pack* packer_init(BCP_user_class* p) {
       return NULL;
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
