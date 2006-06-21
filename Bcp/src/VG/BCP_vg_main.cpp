// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>
#include <cerrno>


#include "BCP_USER.hpp"

#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_problem_core.hpp"

#include "BCP_main_fun.hpp"

#include "BCP_vg_user.hpp"
#include "BCP_vg.hpp"

//#############################################################################

void BCP_vg_main(BCP_message_environment* msg_env, USER_initialize* user_init,
		 BCP_proc_id* my_id, BCP_proc_id* parent)
{
   BCP_vg_prob p;
   p.msg_env = msg_env;
   p.tree_manager = parent;

   // wait for the message with the parameters and unpack it
   p.msg_buf.clear();
   msg_env->receive(p.tree_manager, BCP_Msg_ProcessParameters, p.msg_buf, -1);
   p.par.unpack(p.msg_buf);

   // Let us be nice
   setpriority(PRIO_PROCESS, 0, p.par.entry(BCP_vg_par::NiceLevel));

   FILE* logfile = 0;

   const BCP_string& log = p.par.entry(BCP_vg_par::LogFileName);
   if (! (p.par.entry(BCP_vg_par::LogFileName) == "")) {
      int len = log.length();
      char *logname = new char[len + 300];
      memcpy(logname, log.c_str(), len);
      memcpy(logname + len, "-vg-", 4);
      len += 4;
      gethostname(logname + len, 255);
      len = strlen(logname);
      logname[len++] = '-';
      sprintf(logname + len, "%i", static_cast<int>(getpid()));
      logfile = freopen(logname, "a", stdout);
      if (logfile == 0) {
	 fprintf(stderr, "Error while redirecting stdout: %i\n", errno);
	 abort();
      }
      setvbuf(logfile, NULL, _IOLBF, 0); // make it line buffered
      delete[] logname;
   } else {
      setvbuf(stdout, NULL, _IOLBF, 0); // make it line buffered
   }

   // now create the user universe
   p.user = user_init->vg_init(p);
   p.user->setVgProblemPointer(&p);

   // wait for the core description and process it
   p.msg_buf.clear();
   p.msg_env->receive(p.tree_manager, BCP_Msg_CoreDescription, p.msg_buf, -1);
   p.core->unpack(p.msg_buf);

   // wait for the user info
   p.msg_buf.clear();
   msg_env->receive(p.tree_manager, BCP_Msg_InitialUserInfo, p.msg_buf, -1);
   p.user->unpack_module_data(p.msg_buf);

   // ok, we're all geared up to generate vars
   // wait for messages and process them...
   BCP_message_tag msgtag;
   while (true) {
      p.msg_buf.clear();
      msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage, p.msg_buf, 15);
      msgtag = p.msg_buf.msgtag();
      if (msgtag == BCP_Msg_NoMessage) {
	 // test if the TM is still alive
	 if (! p.msg_env->alive(p.tree_manager))
	    throw BCP_fatal_error("VG:   The TM has died -- VG exiting\n");
      } else {
	 if (BCP_vg_process_message(p, p.msg_buf)) {
	    // BCP_Msg_FinishedBCP arrived
	    break;
	 }
      }
   }
   if (logfile)
      fclose(logfile);
}

//#############################################################################

bool
BCP_vg_process_message(BCP_vg_prob& p, BCP_buffer& buf)
{
   p.process_message();
   return (p.msg_buf.msgtag() == BCP_Msg_FinishedBCP);
}

void
BCP_vg_prob::process_message()
{
   while (true) {
      const BCP_message_tag msgtag = msg_buf.msgtag();
      switch (msgtag) {
       case BCP_Msg_ForVG_DualNonzeros:
       case BCP_Msg_ForVG_DualFull:
       case BCP_Msg_ForVG_User:
	 msg_buf.unpack(node_level).unpack(node_index).unpack(node_iteration);
	 sender = msg_buf.sender()->clone();
	 user->unpack_dual_solution(msg_buf);
	 break;

       case BCP_Msg_UpperBound:
	 double new_ub;
	 msg_buf.unpack(new_ub);
	 if (new_ub < upper_bound)
	    upper_bound = new_ub;
	 break;

       case BCP_Msg_NextPhaseStarts:
	 phase++;
	 break;

       case BCP_Msg_FinishedBCP:
	 return;

       default:
	 // a bogus message
	 printf("Unknown message type arrived to VG: %i\n", msg_buf.msgtag());
      }
      msg_buf.clear();

      if (probe_messages()) {
	 // if there's something interesting in the queue that overrides
	 // the pervious message then just continue to get the next message
	 continue;
      }
      // if there's nothing interesting then msgtag has the message tag of
      // the last unpacked message. We got to do something only if the
      // message is an lp solution, everything else has already been taken
      // care of. 
      if (msgtag == BCP_Msg_ForVG_DualNonzeros  ||
	  msgtag == BCP_Msg_ForVG_DualFull      ||
	  msgtag == BCP_Msg_ForVG_User) {
	 user->generate_vars(cuts, pi);
	 // upon return send a no more vars message
	 double timing = 0.0; // *FIXME*
	 msg_buf.clear();
	 msg_buf.pack(node_index).pack(node_iteration).pack(timing);
	 msg_env->send(sender, BCP_Msg_NoMoreVars, msg_buf);
      }
      break;
   }
}
