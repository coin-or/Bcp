// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_CG_PARAM_H
#define _BCP_CG_PARAM_H

// This file is fully docified.

/** Parameters used in the Cut Generator process. These parameters can be set
    in the original parameter file by including the following line: <br>
    <code>BCP_{parameter name}  {parameter value}</code>. */

struct BCP_cg_par{
   /** Character parameters. All of these variables are used as booleans
       (true = 1, false = 0). */
   enum chr_params{
      /** Indicates whether message passing is serial (all processes are on
	  the same processor) or not. <br>
	  Values: true (1), false (0). Default: 1. */
      MessagePassingIsSerial,
      /** Print out a message when the default version of an overridable
	  method is executed. Default: 1. */
      ReportWhenDefaultIsExecuted,
      /** Just a marker for the first CgVerb */
      CgVerb_First,
      /** Just a marker for the last CgVerb */
      CgVerb_Last,
      //
      end_of_chr_params
   };

   /** Integer parameters. */
   enum int_params{
      /** The "nice" level the process should run at. On Linux and on AIX this
	  value should be between -20 and 20. The higher this value the less
	  resource the process will receive from the system. Note that
	  <ol>
	    <li> The process starts with 0 priority and it can only be
	         increased.
	    <li> If the load is low on the machine (e.g., when all other
	         processes are interactive like netscape or text editors) then
		 even with 20 priority the process will use close to 100% of
		 the cpu, and the interactive processes will be noticably more
		 responsive than they would be if this process ran with 0
		 priority.
	  </ol>
	  @see the man page of the <code>setpriority</code> system function.
      */
      NiceLevel,
      //
      end_of_int_params
   };

   /** There are no double parameters. */
   enum dbl_params{
      dbl_dummy,
      //
      end_of_dbl_params
   };

   /** String parameters. */
   enum str_params{
      /** The file where the output from this process should be logged. To
	  distinguish the output of this CG process from that of the others,
	  the string "-cg-<process_id>" is appended to the given logfile name
	  to form the real filename. */
      LogFileName,
      //
      end_of_str_params
   };
   
   /** There are no string array parameters. */
   enum str_array_params{
      str_array_dummy,
      //
      end_of_str_array_params
   };

};

#endif
