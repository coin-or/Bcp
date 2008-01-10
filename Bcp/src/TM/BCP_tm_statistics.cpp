// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstdio>

#include "BCP_vg.hpp"
#include "BCP_cg.hpp"
#include "BCP_lp.hpp"
#include "BCP_tm.hpp"
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
    case BCP_AlgoObj:
	tm->packer->pack_cut_algo(&dynamic_cast<const BCP_cut_algo&>(cut), buf);
	break;
    default:
	throw BCP_fatal_error("BCP_tm_pack_root_cut: unexpected obj_t.\n");
    }
}

//#############################################################################

void
BCP_tm_save_root_cuts(BCP_tm_prob* tm)
{
    if (! tm->cuts_remote.empty()) {
	throw BCP_fatal_error("\
BCP: saving root cuts does not work at the moment. Must collect cuts from \n\
the TMS processes before the root cuts could be saved. \n");
    }
    const BCP_string& cutfile = tm->param(BCP_tm_par::SaveRootCutsTo);
    if (cutfile.length() > 0 &&
	tm->phase == 0) {
	BCP_buffer& buf = tm->msg_buf;
	buf.clear();
	const BCP_obj_set_change& ch =
	    tm->search_tree.root()->_data._desc->cut_change;
	if (ch.storage() != BCP_Storage_Explicit)
	    throw BCP_fatal_error("Non-explicit cut storage in root!\n");
	const int num = ch._new_objs.size();
	buf.pack(num);
	for (int i = 0; i < num; ++i) {
	    BCP_tm_pack_root_cut(tm, *tm->cuts_local[i]);
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
    BCP_tm_save_root_cuts(tm);

    // Collect the statistics and print it out
    if (!tm->lp_stat)
	tm->lp_stat = new BCP_lp_statistics;

    if (! lp) {
	// Now ask every process
        const std::vector<int>& lps = tm->lp_procs;
        const int num_lp = lps.size();
	BCP_lp_statistics* lp_stats = new BCP_lp_statistics[num_lp];
	BCP_lp_statistics this_lp_stat;
   
	int i;
	for (i = 0; i < num_lp; ++i) {
	    while (true) {
		tm->msg_env->receive(lps[i],
				     BCP_Msg_LpStatistics,
				     tm->msg_buf, 1);
		BCP_message_tag msgtag = tm->msg_buf.msgtag();
		if (msgtag == BCP_Msg_NoMessage) {
		    // test if the LP is still alive
		    if (! tm->msg_env->alive(lps[i]))
			break;
		} else {
		    break;
		}
	    }
	    lp_stats[i].unpack(tm->msg_buf);
	    tm->lp_stat->add(lp_stats[i]);
	}
	for (i = 0; i < num_lp; ++i) {
	    printf("LP # %i : node idle: %12.6f  SB idle: %12.6f\n",
		   lps[i], tm->lp_scheduler.node_idle(lps[i]),
		   tm->lp_scheduler.sb_idle(lps[i]));
	}
	delete[] lp_stats;
    }

    tm->stat.print(true /* final stat */, 0);

    tm->user->display_final_information(lp ? lp->stat : *tm->lp_stat);
}
