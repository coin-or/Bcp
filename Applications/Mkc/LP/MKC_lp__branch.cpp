// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include "OsiSolverInterface.hpp"

#include "CoinTime.hpp"
#include "BCP_lp.hpp"

#include "MKC_lp.hpp"

BCP_branching_decision
MKC_lp::select_branching_candidates(const BCP_lp_result& lpres,
				    const BCP_vec<BCP_var*>& vars,
				    const BCP_vec<BCP_cut*>& cuts,
				    const BCP_lp_var_pool& local_var_pool,
				    const BCP_lp_cut_pool& local_cut_pool,
				    BCP_vec<BCP_lp_branching_object*>& cands,
				    bool force_branch)
{
  bool branch = force_branch;

   // First decide if we want to branch at all.
   if (local_var_pool.size() == 0)
      branch = true;

   const int iter = current_iteration();

   const int tailoff_length = par.entry(MKC_lp_par::TailoffLength);
   const double tailoff_inc =
      par.entry(MKC_lp_par::TailoffIncrease) * (tailoff_length - 1);
   if (! branch) {
      // in this case check tailoff
      if (par.entry(MKC_lp_par::CheckForTailoff) &&
	  iter >= tailoff_length &&
	  objhist[0] - tailoff_inc <= lpres.objval())
	 // branch if did not improve an average of TailoffIncrease in the
	 // past TailoffLength - 1 iterations
	 branch = true;
   }
   if (! branch) {
      if (iter >= tailoff_length) {
	 std::rotate(objhist, objhist + 1, objhist + tailoff_length);
	 objhist[tailoff_length - 1] = lpres.objval();
      } else {
	 objhist[iter - 1] = lpres.objval();
      }
      return BCP_DoNotBranch;
   }

   printf("MKC: branching occured at %f sec.\n", CoinCpuTime() - start_time);

   // We'll generate only one branching object. It'll assign an order to a
   // bunch of slabs in each children. We'll select the order that intersects
   // the most slabs.
   const int order_num = kss.order_num;

   // For each order count the number of slabs it intersects with a
   // fractional variable.
   BCP_vec<int> cnt(order_num, 0);
   BCP_vec<char> slab_flag(kss.ks_num, false);

   // get the row and column formulation of the lp
   OsiSolverInterface* solver = getLpProblemPointer()->lp_solver;
   const CoinPackedMatrix* rowMatrix = solver->getMatrixByRow();
   const CoinPackedMatrix* colMatrix = solver->getMatrixByCol();

   const int * rowstart = rowMatrix->getVectorStarts();
   const int * rowlen   = rowMatrix->getVectorLengths();
   const int * rowind   = rowMatrix->getIndices();

   const int * colstart = colMatrix->getVectorStarts();
   // const int * collen   = colMatrix->getVectorLengths();
   const int * colind   = colMatrix->getIndices();
   
   const double* x = lpres.x();

   int i, k;
   for (i = 0; i < order_num; ++i) {
     std::fill(slab_flag.begin(), slab_flag.end(), false);
     const int * rowcur = rowind + rowstart[kss.ks_num + i];
     const int * rowend = rowcur + rowlen[kss.ks_num + i];
     int & thiscnt = cnt[i];
     for ( ; rowcur != rowend; ++rowcur) {
       if (x[*rowcur] > 0.00001) {
	 const int slab = colind[colstart[*rowcur]];
	 if (!slab_flag[slab]) {
	   slab_flag[slab] = true;
	   ++thiscnt;
	 }
       }
     }
   }
   // the biggest intersection is maxval. Find the one with maxval that also
   // the longest row among those.
   const int maxval = *std::max_element(cnt.begin(), cnt.end());
   int bestind = -1;
   int bestlen = -1;
   for (i = 0; i < order_num; ++i) {
     if (cnt[i] == maxval) {
       const int len = rowlen[kss.ks_num + i];
       if (len > bestlen) {
	 bestind = i;
	 bestlen = len;
       }
     }
   }

   const int branch_order = bestind;
   const int child_num = cnt[branch_order] + 1;

   std::fill(slab_flag.begin(), slab_flag.end(), false);
   const int * rowcur = rowind + rowstart[kss.ks_num + branch_order];
   const int * rowend = rowcur + rowlen[kss.ks_num + branch_order];
   for ( ; rowcur != rowend; ++rowcur) {
     if (x[*rowcur] > 0.00001) {
       const int slab = colind[colstart[*rowcur]];
       if (!slab_flag[slab]) {
	 slab_flag[slab] = true;
       }
     }
   }
   int * slab_list = new int[child_num];
   for (i = 0, k = 0; i < kss.ks_num; ++i) {
     if (slab_flag[i])
       slab_list[k++] = i;
   }
   slab_list[k] = -1;
   MKC_branching_var * bvar =
     new MKC_branching_var(branch_order, child_num, slab_list);

   BCP_vec<BCP_var*>* new_vars = new BCP_vec<BCP_var*>(1, bvar);
   BCP_vec<BCP_cut*>* new_cuts = 0;

   // changes forced by the branching
   BCP_vec<int>* fcp = 0;
   BCP_vec<double>* fcb = 0;
   BCP_vec<int>* fvp = new BCP_vec<int>(1, -1);
   BCP_vec<double>* fvb = new BCP_vec<double>(2 * child_num);
   for (k = 0, i = 0; i < child_num; ++i) {
     (*fvb)[k++] = i;
     (*fvb)[k++] = i;
   }

   // changes implied by the branching
   BCP_vec<int>* icp = 0;
   BCP_vec<double>* icb = 0;
   // *FIXME* !!!!
   BCP_vec<int>* ivp = new BCP_vec<int>(1, -1);
   BCP_vec<double>* ivb = new BCP_vec<double>(2 * child_num);
   for (k = 0, i = 0; i < child_num; ++i) {
     (*ivb)[k++] = i;
     (*ivb)[k++] = i;
   }
   
   cands.push_back(new BCP_lp_branching_object(child_num,
					       new_vars, new_cuts,
					       fvp, fcp, fvb, fcb,
					       ivp, icp, ivb, icb));
   return BCP_DoBranch;
}
