// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>

#include "BCP_vg.hpp"
#include "BCP_cg.hpp"
#include "BCP_lp.hpp"
#include "BCP_tm.hpp"
#include "BCP_timeout.hpp"
#include "BCP_solution.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_node_change.hpp"

static inline void
BCP_tm_pack_root_cut(BCP_tm_prob* tm, const BCP_cut& cut)
{
  BCP_buffer& buf = tm->msg_buf;
  const BCP_object_t obj_t = cut.obj_type();
  const BCP_obj_status stat = cut.status();
  const double lb = cut.lb();
  const double ub = cut.ub();
  buf.pack(obj_t).pack(stat).pack(lb).pack(ub);
  switch (obj_t) {
  case BCP_CoreObj:
    break;
  case BCP_IndexedObj:
    {
      const int index = (dynamic_cast<const BCP_cut_indexed&>(cut)).index();
      buf.pack(index);
    }
    break;
  case BCP_AlgoObj:
    tm->user->pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), buf);
    break;
  default:
    throw BCP_fatal_error("BCP_tm_pack_root_cut: unexpected obj_t.\n");
  }
}

//#############################################################################

void
BCP_tm_save_root_cuts(BCP_tm_prob* tm)
{
  const BCP_string& cutfile = tm->param(BCP_tm_par::SaveRootCutsTo);
  if (cutfile.length() > 0 &&
      tm->phase == 0) {
    BCP_buffer& buf = tm->msg_buf;
    buf.clear();
    const BCP_cut_set_change& ch = tm->search_tree.root()->_desc->cut_change;
    if (ch.storage() != BCP_Storage_Explicit)
      throw BCP_fatal_error("Non-explicit cut storage in root!\n");
    const int num = ch._new_cuts.size();
    buf.pack(num);
    for (int i = 0; i < num; ++i) {
      BCP_tm_pack_root_cut(tm, *ch._new_cuts[i]);
    }
    FILE* f = fopen(cutfile.c_str(), "w");
    size_t size = buf.size();
    if (fwrite(&size, 1, sizeof(size), f) != sizeof(size))
      throw BCP_fatal_error("SaveRootCutsTo write error.\n");
    if (fwrite(buf.data(), 1, size, f) != size)
      throw BCP_fatal_error("SaveRootCutsTo write error.\n");
    fclose(f);
  }
}

//#############################################################################

void
BCP_tm_wrapup(BCP_tm_prob* tm, BCP_lp_prob* lp,
	      BCP_cg_prob* cg, BCP_vg_prob* vg, bool final_stat)
{
  printf("TM: Total running time: %.3f\n",
	 BCP_time_since_epoch() - tm->start_time);

  BCP_tm_save_root_cuts(tm);

  const bool bval = tm->param(BCP_tm_par::TmVerb_BestFeasibleSolutionValue);
  const bool bsol = tm->param(BCP_tm_par::TmVerb_BestFeasibleSolution);
  if (bval || bsol) {
     if (! tm->feas_sol) {
	printf("TM: No feasible solution is found\n");
     } else {
	printf("TM: The best solution found has value %f\n",
	       tm->feas_sol->objective_value());
     }
  }
  if (bsol && tm->feas_sol)
    tm->user->display_feasible_solution(tm->feas_sol);

  if (lp) {
    lp->stat.display();
  } else {
   // Collect the statistics and print it out
    int i;
    if (!tm->lp_stat)
      tm->lp_stat = new BCP_lp_statistics;

    // Now ask every process
    const int num_lp = tm->slaves.lp->size();
    BCP_lp_statistics this_lp_stat;

    for (i = 0; i < num_lp; ++i) {
      while (true) {
	tm->msg_env->receive(tm->slaves.lp->process(i), BCP_Msg_LpStatistics,
			     tm->msg_buf, 10);
	BCP_message_tag msgtag = tm->msg_buf.msgtag();
	if (msgtag == BCP_Msg_NoMessage) {
	  // test if the LP is still alive
	  if (! tm->msg_env->alive(tm->slaves.lp->process(i)))
	    break;
	} else {
	  break;
	}
      }
      this_lp_stat.unpack(tm->msg_buf);
      tm->lp_stat->add(this_lp_stat);
    }

    tm->lp_stat->display();
  }

  printf("\n\nBCP finished!!!\n\n");
}
