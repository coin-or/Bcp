// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include <cerrno>

#include "BcpConfig.h"
#include "BCP_os.hpp"

#include "BCP_USER.hpp"
#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_main_fun.hpp"
#include "BCP_cg_user.hpp"
#include "BCP_cg.hpp"

//#############################################################################

void BCP_cg_main(BCP_message_environment* msg_env, USER_initialize* user_init,
		 BCP_proc_id* my_id, BCP_proc_id* parent)
{
   BCP_cg_prob p(my_id, parent);
   p.msg_env = msg_env;

   // wait for the message with the parameters and unpack it
   p.msg_buf.clear();
   msg_env->receive(parent, BCP_Msg_ProcessParameters, p.msg_buf, -1);
   p.par.unpack(p.msg_buf);

   // Let us be nice
   setpriority(PRIO_PROCESS, 0, p.par.entry(BCP_cg_par::NiceLevel));

   FILE* logfile = 0;

   const BCP_string& log = p.par.entry(BCP_cg_par::LogFileName);
   if (! (p.par.entry(BCP_cg_par::LogFileName) == "")) {
      int len = log.length();
      char *logname = new char[len + 300];
      memcpy(logname, log.c_str(), len);
      memcpy(logname + len, "-cg-", 4);
      len += 4;
      gethostname(logname + len, 255);
      len = strlen(logname);
      logname[len++] = '-';
      sprintf(logname + len, "%i", static_cast<int>(GETPID));
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
   p.user = user_init->cg_init(p);
   p.user->setCgProblemPointer(&p);

   // wait for the core description and process it
   p.msg_buf.clear();
   p.msg_env->receive(parent, BCP_Msg_CoreDescription, p.msg_buf, -1);
   p.core->unpack(p.msg_buf);

   // wait for the user info
   p.msg_buf.clear();
   msg_env->receive(parent, BCP_Msg_InitialUserInfo, p.msg_buf, -1);
   p.user->unpack_module_data(p.msg_buf);

   // ok, we're all geared up to generate cuts
   // wait for messages and process them...
   BCP_message_tag msgtag;
   while (true) {
      p.msg_buf.clear();
      msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage, p.msg_buf, 15);
      msgtag = p.msg_buf.msgtag();
      if (msgtag == BCP_Msg_NoMessage) {
	 // test if the TM is still alive
	 if (! p.msg_env->alive(parent))
	    throw BCP_fatal_error("CG:   The TM has died -- CG exiting\n");
      } else {
	 if (BCP_cg_process_message(p, p.msg_buf)) {
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
BCP_cg_process_message(BCP_cg_prob& p, BCP_buffer& buf)
{
   p.process_message();
   return (p.msg_buf.msgtag() == BCP_Msg_FinishedBCP);
}

void
BCP_cg_prob::process_message()
{
   while (true) {
      const BCP_message_tag msgtag = msg_buf.msgtag();
      switch (msgtag) {
       case BCP_Msg_ForCG_PrimalNonzeros:
       case BCP_Msg_ForCG_PrimalFractions:
       case BCP_Msg_ForCG_PrimalFull:
       case BCP_Msg_ForCG_User:
	 msg_buf.unpack(node_level).unpack(node_index).unpack(node_iteration);
	 sender = msg_buf.sender()->clone();
	 user->unpack_primal_solution(msg_buf);
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
	 printf("Unknown message type arrived to CG: %i\n", msg_buf.msgtag());
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
      if (msgtag == BCP_Msg_ForCG_PrimalNonzeros  ||
	  msgtag == BCP_Msg_ForCG_PrimalFractions ||
	  msgtag == BCP_Msg_ForCG_PrimalFull      ||
	  msgtag == BCP_Msg_ForCG_User) {
	 user->generate_cuts(vars, x);
	 // upon return send a no more cuts message
	 double timing = 0.0; // *FIXME*
	 msg_buf.clear();
	 msg_buf.pack(node_index).pack(node_iteration).pack(timing);
	 msg_env->send(sender, BCP_Msg_NoMoreCuts, msg_buf);
      }
      break;
   }
}
