// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <queue>

#include "BCP_os.hpp"

#include "BCP_USER.hpp"
#include "BCP_timeout.hpp"
#include "BCP_string.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_node_change.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_functions.hpp"
#include "BCP_main_fun.hpp"

#include "BCP_tm_user.hpp"
#include "BCP_lp_user.hpp"

#include "BCP_message_single.hpp"

//#############################################################################

int main(int argc, char* argv[])
{
   USER_initialize* user_init = BCP_user_init();

   BCP_message_environment* msg_env = user_init->msgenv_init();
   {
     BCP_single_environment* single_env =
       dynamic_cast<BCP_single_environment*>(msg_env);
     if (single_env) {
       // this way when register_process takes over the execution the single
       // environment will have access to the command line arguments and can
       // parse it.
       single_env->set_arguments(argc, argv);
     }
   }

   BCP_proc_id* my_id = msg_env->register_process();
   if (my_id == 0) {
      // Using BCP_single_environment will return '0'.
      delete msg_env;
      throw BCP_fatal_error("Exiting.\n");
   }
   BCP_proc_id* parent = msg_env->parent_process();

   // The master process takes one command-line argument which is the name of
   // the parameter file. The slaves do not take any arguments.
   if (! parent) {
     BCP_tm_main(msg_env, user_init, my_id, argc, argv);
   } else {
     if (argc != 1) {
       throw BCP_fatal_error("The slaves do not take any argument!\n");
     }
     BCP_buffer msg_buf;
     msg_env->receive(parent, BCP_Msg_AnyMessage, msg_buf, -1);
     if (msg_buf.msgtag() != BCP_Msg_ProcessType) {
       throw BCP_fatal_error("The first message is not ProcessType!!!\n");
     }
     // got a new identity, act on it
     BCP_process_t ptype;
     msg_buf.unpack(ptype);
     switch (ptype) {
     case BCP_ProcessType_LP:
       BCP_lp_main(msg_env, user_init, my_id, parent);
       break;
     case BCP_ProcessType_CP:
       // BCP_cp_main(msg_env, user_init, my_id, parent);
       break;
     case BCP_ProcessType_VP:
       // BCP_vp_main(msg_env, user_init, my_id, parent);
       break;
     case BCP_ProcessType_CG:
       BCP_cg_main(msg_env, user_init, my_id, parent);
       break;
     case BCP_ProcessType_VG:
       BCP_vg_main(msg_env, user_init, my_id, parent);
       break;
     case BCP_ProcessType_Any:
       throw BCP_fatal_error("New process identity is BCP_ProcessType_Any!\n");
     case BCP_ProcessType_TM:
       throw BCP_fatal_error("New process identity is BCP_ProcessType_TM!\n");
     }
   }
   delete parent;
   delete my_id;
   delete msg_env;
   delete user_init;

   return 0;
}

//#############################################################################

void
BCP_tm_main(BCP_message_environment* msg_env,
	    USER_initialize* user_init,
	    BCP_proc_id* my_id,
	    const int argnum, const char* const * arglist)
{
   // Start to create the universe... (we don't have a user universe yet).
   BCP_tm_prob p;

   // this also reads in the parameters from a file
   BCP_tm_parse_command_line(p, argnum, arglist);
   
   BCP_buffer msg_buf;
   p.msg_env = msg_env;
   p.start_time = BCP_time_since_epoch();

   FILE* logfile = 0;

   const BCP_string& log = p.par.entry(BCP_tm_par::LogFileName);
   if (! (p.par.entry(BCP_tm_par::LogFileName) == "")) {
      int len = log.length();
      char *logname = new char[len + 300];
      memcpy(logname, log.c_str(), len);
      memcpy(logname + len, "-tm-", 4);
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

   // BCP_tm_user_init() returns a BCP_tm_user* and that will be part of p.
   // Also, it should take care of every I/O, heuristic startup, etc. the user
   // wants to do. Wreak havoc in p if (s)he wants.
   // BUT: once this function returns, the processes designated to be part of
   // BCP must be idle and waiting for a message. See the
   // BCP_slave_process_stub() function below. 
   p.user = user_init->tm_init(p, argnum, arglist);
   p.user->setTmProblemPointer(&p);

   if (!p.param(BCP_tm_par::DoBranchAndCut)){
      printf("\n**********************************************\n");
      printf("* Heuristics Finished!!!!!!!                 *\n");
      printf("* Now displaying stats and best solution.... *\n");
      printf("**********************************************\n\n");
      // *FIXME-NOW*
      // Print out timing and solution

      delete p.user;   p.user = 0;
      // delete parent; -- not needed: TM has no parent, it's a 0 pointer
      delete my_id;   my_id = 0;
      delete msg_env;   msg_env = 0;
      exit(0);
   }

   // Fire up the LP/CG/CP/VG/VP processes
   // Actually, this is firing up enough copies of self.
   BCP_tm_start_processes(p);

   // Notify the LP/CG/CP/VG/VP processes about their identity. Also, send out
   // their parameters.
   BCP_tm_notify_processes(p);

   // Initialize the number of leaves assigned to CP's and VP's as 0
   if (p.param(BCP_tm_par::CpProcessNum) > 0) {
      for (int i = p.slaves.cp->size() - 1; i >= 0; --i)
	 p.leaves_per_cp.
	    push_back( std::make_pair(p.slaves.cp->procs()[i]->clone(), 0) );
   }
   if (p.param(BCP_tm_par::VpProcessNum) > 0) {
      for (int i = p.slaves.vp->size() - 1; i >= 0; --i)
	 p.leaves_per_vp.
	    push_back( std::make_pair(p.slaves.vp->procs()[i]->clone(), 0) );
   }

   // Set the core (variables & cuts)
   p.core = BCP_tm_create_core(p);
   p.core_as_change = new BCP_problem_core_change;
   *p.core_as_change = *p.core;
   // Send the core description to every process
   BCP_tm_distribute_core(p);

   BCP_tm_distribute_user_info(p);

   // Initialize the root of the search tree (can't invoke directly
   // p.user->create_root(), b/c the root might contain extra vars/cuts and
   // it's better if we take care of inserting them into the appropriate data
   // structures.
   BCP_tm_node* root = BCP_tm_create_root(p);

   p.next_phase_nodes.push_back(root);
   p.search_tree.insert(root);

   BCP_sanity_checks(p);

   //--------------------------------------------------------------------------
   // The main loop
   //--------------------------------------------------------------------------
   bool something_died = false;
   for ( p.phase = 0; true ; ++p.phase) {
      // insert the nodes in next_phase_nodes into candidates, print out some
      // statistics about the previous phase (if there was one) and do some
      // other stuff, too.
      BCP_tm_tasks_before_new_phase(p);
      // do one phase (return true/false depending on success)
      something_died = BCP_tm_do_one_phase(p);
      // If nothing is left for the next phase or if something has died then
      // quit the infinite loop.
      if (p.next_phase_nodes.size() == 0 || something_died)
	 break;
   }

   //--------------------------------------------------------------------------
   // Everything is done
   //--------------------------------------------------------------------------
   // first let the processes know that they're not needed in BCP any more,
   // they can start idling (this will be used when we'll loop around in TM).
   // The processes will respond by sending statistics.
   BCP_tm_idle_processes(p);

   BCP_tm_wrapup(&p, 0, 0, 0, true);

   // Finally stop all the processes.
   BCP_tm_stop_processes(p);

   if (logfile)
      fclose(logfile);

   delete p.user;   p.user = 0;
   // delete parent; -- not needed: TM has no parent, it's a 0 pointer
   delete my_id;   my_id = 0;
   delete msg_env;  msg_env = 0;
}

//#############################################################################

bool BCP_tm_do_one_phase(BCP_tm_prob& p)
{
   BCP_buffer& buf = p.msg_buf;
   // While there are nodes waiting to be processed (or being processed) we
   // don't go to the next phase
   while (!p.candidates.empty() > 0 || p.slaves.lp->busy_num() > 0){
      // Fill up as many free LP processes as we can
      if (BCP_tm_start_new_nodes(p) == BCP_NodeStart_Error)
	 // Error indicates that something has died
	 return true;
      // Process incoming messages. If there are no active nodes left then
      // timeout is set to 0, so we just check the queue, but not wait.
      buf.clear();
      p.msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage, buf,
			 p.slaves.lp->busy_num() == 0 ?
			 0 : p.param(BCP_tm_par::TmTimeout));
      p.process_message();
   }
   return false;
}

//#############################################################################

BCP_problem_core* BCP_tm_create_core(BCP_tm_prob& p)
{
   BCP_vec<BCP_var_core*> bvars;
   BCP_vec<BCP_cut_core*> bcuts;
   BCP_lp_relax* matrix = 0;

   p.user->initialize_core(bvars, bcuts, matrix);

   const int bvarnum = bvars.size();
   if (bvarnum > 0) {
      BCP_IndexType i = p.next_var_index_set_start;
      for (i = 0; i < bvarnum; ++i) {
	 BCP_var_core* var = bvars[i];
	 // make certain that the bounds of integer vars is integer...
	 if (var->var_type() != BCP_ContinuousVar) {
	    var->set_lb(floor(var->lb()+1e-8));
	    var->set_ub(ceil(var->ub()-1e-8));
	 }
	 var->set_bcpind(i);
	 p.vars[i] = new BCP_var_core(*var);
      }
      p.next_var_index_set_start = i;
   }

   const int bcutnum = bcuts.size();
   if (bcutnum > 0) {
      BCP_IndexType i = p.next_cut_index_set_start;
      for (i = 0; i < bcutnum; ++i) {
	 BCP_cut_core* cut = bcuts[i];
	 cut->set_bcpind(i);
	 p.cuts[i] = new BCP_cut_core(*cut);
      }
      p.next_cut_index_set_start = i;
   }

   if (!matrix)
      matrix = new BCP_lp_relax;

   return new BCP_problem_core(bvars, bcuts, matrix);
}
   
//#############################################################################

static inline BCP_cut*
BCP_tm_unpack_root_cut(BCP_tm_prob& tm)
{
  BCP_cut* cut;
  BCP_buffer& buf = tm.msg_buf;
  BCP_object_t obj_t;
  int index;
  double lb, ub;
  BCP_obj_status stat;
  buf.unpack(obj_t).unpack(stat).unpack(lb).unpack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    cut = new BCP_cut_core(lb, ub);
    break;
  case BCP_IndexedObj:
    buf.unpack(index);
    cut = new BCP_cut_indexed(index, lb, ub);
    break;
  case BCP_AlgoObj:
    cut = tm.user->unpack_cut_algo(buf);
    cut->change_bounds(lb, ub);
    break;
  default:
    throw BCP_fatal_error("BCP_tm_unpack_root_cut: unexpected obj_t.\n");
  }
  cut->set_bcpind(0);
  cut->set_status(stat);

  return cut;
}

//#############################################################################

BCP_tm_node* BCP_tm_create_root(BCP_tm_prob& p)
{
   BCP_vec<BCP_var*> added_vars;
   BCP_vec<BCP_cut*> added_cuts;
   BCP_user_data* user_data = 0;
   BCP_pricing_status pricing_status = BCP_PriceNothing;

   // If the root cuts are saved then read them in
   const BCP_string& cutfile = p.param(BCP_tm_par::ReadRootCutsFrom);
   if (cutfile.length() > 0) {
      BCP_buffer& buf = p.msg_buf;
      buf.clear();
      FILE* f = fopen(cutfile.c_str(), "r");
      size_t size;
      if (fread(&size, 1, sizeof(size), f) != sizeof(size))
	 throw BCP_fatal_error("ReadRootCutsFrom read error.\n");
      char * data = new char[size];
      if (fread(data, 1, size, f) != size)
	 throw BCP_fatal_error("ReadRootCutsFrom read error.\n");
      fclose(f);
      buf.set_content(data, size, 0, BCP_Msg_NoMessage);
      delete[] data;
      
      int num;
      buf.unpack(num);
      added_cuts.reserve(added_cuts.size() + num);
      for (int i = 0; i < num; ++i) {
	 added_cuts.unchecked_push_back(BCP_tm_unpack_root_cut(p));
      }
   }

   p.user->create_root(added_vars, added_cuts, user_data, pricing_status);

   BCP_node_change* root_changes = new BCP_node_change;
   root_changes->core_change._storage = BCP_Storage_WrtCore;
   root_changes->indexed_pricing.set_status(pricing_status);

   if (added_vars.size() > 0) {
      BCP_vec<BCP_var*>::iterator vi = added_vars.begin();
      const BCP_vec<BCP_var*>::iterator lastvi = added_vars.end();
      root_changes->var_change._change.reserve(added_vars.size());
      BCP_IndexType i = p.next_var_index_set_start;
      while (vi != lastvi) {
	 BCP_var* var = *vi;
	 p.vars[i] = var;
	 var->set_bcpind(i++);
	 // make certain that the bounds of integer vars is integer...
	 if (var->var_type() != BCP_ContinuousVar) {
	    var->set_lb(floor(var->lb()+1e-8));
	    var->set_ub(ceil(var->ub()-1e-8));
	 }
	 root_changes->var_change._change.
	    unchecked_push_back(BCP_obj_change(var->lb(), var->ub(),
					       var->status()));
	 ++vi;
      }
      p.next_var_index_set_start = i;
      root_changes->var_change._new_vars.swap(added_vars);
   }

   if (added_cuts.size() > 0) {
      BCP_vec<BCP_cut*>::iterator ci = added_cuts.begin();
      const BCP_vec<BCP_cut*>::iterator lastci = added_cuts.end();
      root_changes->cut_change._change.reserve(added_cuts.size());
      BCP_IndexType i = p.next_cut_index_set_start;
      while (ci != lastci) {
	 BCP_cut* cut = *ci;
	 p.cuts[i] = cut;
	 cut->set_bcpind(i++);
	 root_changes->cut_change._change.
	    unchecked_push_back(BCP_obj_change(cut->lb(), cut->ub(),
					       cut->status()));
	 ++ci;
      }
      p.next_cut_index_set_start = i;
      root_changes->cut_change._new_cuts.swap(added_cuts);
   }

   BCP_tm_node* root = new BCP_tm_node(0, root_changes);

   root->_user_data = user_data;

   return root;
}

//#############################################################################

void BCP_tm_tasks_before_new_phase(BCP_tm_prob& p)
{
   if (p.param(BCP_tm_par::TmVerb_NewPhaseStart)) {
      printf("############################################################\n");
      printf("TM: Starting phase %i\n", p.phase);
      printf("############################################################\n");
   }

   BCP_tm_node* root = 0;

   if (p.phase > 0) {
      // Notify the LPs about the start of the new phase and get back the
      // timing data for the previous phase
      BCP_tm_notify_about_new_phase(p);

      if (p.phase == 1 &&
	  p.search_tree.size() > 1 &&
	  p.param(BCP_tm_par::PriceInRootBeforePhase2)) {
	 // note that pricing the root won't mess up ANYTHING in the storage
	 // of the root, except that it'll send back the new indexed_pricing
	 // field correctly put together, so just unpacking it in
	 // BCP_tm_prob::process_message will create a correct new root
	 // description.
	 p.flags.root_pricing_unpacked = false;
	 p.current_phase_colgen = BCP_GenerateColumns;
	 root = p.search_tree.root();
	 root->_desc->indexed_pricing.
	   set_status(BCP_PriceAfterLastIndexedToPrice);
#ifdef BCP_DEBUG
	 if (root->lp != 0 || root->cg != 0 || root->vg != 0)
	    throw BCP_fatal_error("\
TM: At least on of lp/cg/vg of the root is non-0 (before repricing).\n");
#endif
	 // cp and vp are left what they were
	 if (! BCP_tm_assign_processes(p, root)) {
	    // *LATER* : oops, something has died. do sthing
	 }
	 // send out the root to be priced
	 BCP_tm_send_node(p, root, BCP_Msg_RootToPrice);
      }

      // print statistics about the previous phase
      BCP_tm_wrapup(&p, 0, 0, 0, false); // false refers to not being final

      // trim the tree if needed and possible
      if (p.param(BCP_tm_par::TrimTreeBeforeNewPhase) && p.has_ub())
	 BCP_tm_trim_tree_wrapper(p, true /* called between phases */);
   }

   // while the LP (maybe) working on repricing, build up candidates.
   // initialize the candidate list comparison function.
   p.current_phase_colgen = BCP_DoNotGenerateColumns_Fathom;
   p.user->init_new_phase(p.phase, p.current_phase_colgen);

   for (int i = p.next_phase_nodes.size() - 1; i >= 0; --i) {
     p.candidates.insert(p.next_phase_nodes[i]);
   }

   p.next_phase_nodes.clear();

   if (root && !p.flags.root_pricing_unpacked) {
      // if root is not 0 then we have sent out the root for pricing and
      // now we have to receive the results unless it has been already
      // unpacked (this happens in a single process environment).
      p.msg_env->receive(root->lp, BCP_Msg_PricedRoot, p.msg_buf, -1);
      BCP_tm_unpack_priced_root(p, p.msg_buf);
   }

   // Reset column generation if PriceInRootBeforePhase2
   if (p.phase == 0 && p.param(BCP_tm_par::PriceInRootBeforePhase2)) {
      if (p.current_phase_colgen != BCP_DoNotGenerateColumns_Send) {
	 printf("#########################################################\n");
	 printf("TM: *WARNING*  *WARNING*  *WARNING*  *WARNING*  *WARNING*\n");
	 printf("    PriceInRootBeforePhase2 is set yet the column \n");
	 printf("    generation strategy is set to ");
	 switch (p.current_phase_colgen) {
	  case BCP_DoNotGenerateColumns_Fathom:
	    printf("BCP_DoNotGenerateColumns_Fathom"); break;
	  case BCP_GenerateColumns:
	    printf("BCP_GenerateColumns"); break;
	  case BCP_DoNotGenerateColumns_Send:
	    break;
	 }
	 printf(" in the first phase!\n");
	 printf("   The strategy is reset to BCP_DoNotGenerateColumns_Send\n");
	 printf("#########################################################\n");
      }
   }
}

//#############################################################################


