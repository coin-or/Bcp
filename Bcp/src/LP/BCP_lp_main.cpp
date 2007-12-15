// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include <cerrno>
#ifdef _MSC_VER
#include <process.h>
#endif 

#include "CoinTime.hpp"

#include "BcpConfig.h"
#include "BCP_os.hpp"

#include "BCP_USER.hpp"
#include "BCP_buffer.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp.hpp"
#include "BCP_main_fun.hpp"
#include "BCP_lp_functions.hpp"
#include "BCP_lp_user.hpp"

#include "BCP_message.hpp"

//#############################################################################

void
BCP_lp_process_core(BCP_lp_prob& p, BCP_buffer& buf)
{
   p.core->unpack(buf);

   // create the core_as_change from core
   *p.core_as_change = *p.core;

   // create the core part of p.node
   const BCP_vec<BCP_var_core*>& bvars = p.core->vars;
   const int bvarnum = bvars.size();
   p.node->vars.reserve(std::max<int>(1000, 3 * bvarnum));
   if (bvarnum > 0) {
      BCP_vec<BCP_var*>& vars = p.node->vars;
      for (int i = 0; i < bvarnum; ++i) {
	 vars.unchecked_push_back(new BCP_var_core(*bvars[i]));
      }
   }

   const BCP_vec<BCP_cut_core*>& bcuts = p.core->cuts;
   const int bcutnum = bcuts.size();
   p.node->cuts.reserve(std::max<int>(1000, 3 * bcutnum));
   if (bcutnum > 0) {
      BCP_vec<BCP_cut*>& cuts = p.node->cuts;
      for (int i = 0; i < bcutnum; ++i) {
	 cuts.unchecked_push_back(new BCP_cut_core(*bcuts[i]));
      }
   }
}

//#############################################################################

BCP_process_t BCP_lp_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub)
{
   BCP_lp_prob p(my_id, parent);
   p.upper_bound = ub;
   p.msg_env = msg_env;

   // wait for the message with the parameters and unpack it
   p.msg_buf.clear();
   msg_env->receive(parent /*tree_manager*/,
		    BCP_Msg_ProcessParameters, p.msg_buf, -1);
   p.par.unpack(p.msg_buf);
   double startTimeOfDay;
   p.msg_buf.unpack(startTimeOfDay);
   CoinWallclockTime(startTimeOfDay);

   // Let us be nice
   setpriority(PRIO_PROCESS, 0, p.par.entry(BCP_lp_par::NiceLevel));

   FILE* logfile = 0;

   const BCP_string& log = p.par.entry(BCP_lp_par::LogFileName);
   if (! (p.par.entry(BCP_lp_par::LogFileName) == "")) {
      int len = log.length();
      char *logname = new char[len + 300];
      memcpy(logname, log.c_str(), len);
      memcpy(logname + len, "-lp-", 4);
      len += 4;
      gethostname(logname + len, 255);
      len = strlen(logname);
      logname[len++] = '-';
      sprintf(logname + len, "%i", static_cast<int>(GETPID));
      logfile = freopen(logname, "a", stdout);
      if (logfile == 0) {
	 fprintf(stderr, "Error while redirecting stdout: %i\n", errno);
	 throw BCP_fatal_error("");
      }
      setvbuf(logfile, NULL, _IOLBF, 0); // make it line buffered
      delete[] logname;
   } else {
      setvbuf(stdout, NULL, _IOLBF, 0); // make it line buffered
   }

   // now create the user universe
   p.user = user_init->lp_init(p);
   p.user->setLpProblemPointer(&p);
   p.packer = user_init->packer_init(p.user);
   p.packer->user_class = p.user;

   // wait for the core description and process it
   p.msg_buf.clear();
   p.msg_env->receive(parent /*tree_manager*/,
		      BCP_Msg_CoreDescription, p.msg_buf, -1);
   BCP_lp_process_core(p, p.msg_buf);

   // wait for the user info
   p.msg_buf.clear();
   msg_env->receive(parent /*tree_manager*/,
		    BCP_Msg_InitialUserInfo, p.msg_buf, -1);
   p.user->unpack_module_data(p.msg_buf);

   p.master_lp = p.user->initialize_solver_interface();
   p.user->initialize_int_and_sos_list(p.intAndSosObjects);

   // ok, we're all geared up to process search tree nodes
   // wait for messages and process them...
   BCP_message_tag msgtag;
   BCP_process_t ptype = BCP_ProcessType_EndProcess;
   while (true) {
      p.msg_buf.clear();
      msg_env->receive(BCP_AnyProcess,
		       BCP_Msg_AnyMessage, p.msg_buf, -1);
      msgtag = p.msg_buf.msgtag();
      if (msgtag == BCP_Msg_ProcessType) {
	  p.msg_buf.unpack(ptype);
	  break;
      }
      p.no_more_cuts_cnt = -1; // not waiting for cuts
      p.process_message();
      if (msgtag == BCP_Msg_FinishedBCP)
	 break;
   }

   if (logfile)
      fclose(logfile);

   return ptype;
}
