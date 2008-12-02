// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <algorithm>

#include "CoinSort.hpp"

#include "BCP_vector.hpp"
#include "MKC_knapsack.hpp"
#include "MKC_var.hpp"
#include "MKC_vargen.hpp"
#include "MKC_optim.hpp"

static const bool print_knapsacks = false;

//#############################################################################

double
MKC_compute_ks_upper_bound(const MKC_knapsack_set& kss,
			   const MKC_knapsack_fixing* ks_fixings,
			   BCP_vec<MKC_var*>* enumerated_ks,
			   const int max_enumerated_size,
			   const double* dual,
			   const double rc_bound,
			   const double exact_red_cost,
			   const bool print_best_dj,
			   const bool fall_back_to_exact)
{
   const int ks_num = kss.ks_num;
   double bound = 0.0;

   if (print_best_dj) {
      printf("\
MKC: Lower bounding based on checking the enumerated knapsacks, exactly\n\
MKC: solving smaller other knapsack and solving the LP relaxation for bigger\n\
MKC: other knapsacks.\n");
   }

   // First compute the bound for the enumerated knapsacks
   BCP_vec<MKC_var*> new_mkc_vars;
   MKC_check_enumerated_variables(kss, enumerated_ks, max_enumerated_size,
				  dual, DBL_MAX, -rc_bound, new_mkc_vars,
				  print_best_dj);
   const int enumerated_size = new_mkc_vars.size();
   for (int i = 0; i < enumerated_size; ++i)
      bound += new_mkc_vars[i]->dj(dual);
   purge_ptr_vector(new_mkc_vars);


   // then compute the bound for the rest
   BCP_vec<BCP_var*> new_vars;
   int what_to_do;
   for (int i = 0; i < ks_num; ++i) {
      if (kss.ks_list[i].size <= max_enumerated_size)
	 continue;
      what_to_do = fall_back_to_exact && kss.ks_list[i].size < 30 ? 3 : 2;
      const double dj = 
	 MKC_generate_vars_one_ks(ks_num, i,
				  kss.ks_list[i], ks_fixings[i],
				  dual, new_vars, exact_red_cost, what_to_do,
				  0);
      bound += dj;
      if (print_best_dj)
	 printf("MKC: for ks %i lower bounding gives %f.\n", i, dj);
   }
   purge_ptr_vector(new_vars);
   return bound;
}

//#############################################################################
// rc_bound is a positive number indicating how negative redcost variables we
// are looking for

void
MKC_generate_variables(const MKC_knapsack_set& kss,
		       const MKC_knapsack_fixing* ks_fixings,
		       BCP_vec<MKC_var*>* enumerated_ks,
		       const int max_enumerated_size,
		       const double* dual,
		       const double gap,
		       BCP_vec<BCP_var*>& new_vars,
		       const double rc_bound,
		       const bool for_all_knapsack,
		       const bool all_encountered_var,
		       const bool print_best_dj,
		       const bool fall_back_to_exact,
		       const int index, const int iternum)
{
   const int ks_num = kss.ks_num;
   int i;
   double best_dj = 0.0;
   BCP_vec<MKC_var*> new_mkc_vars;

   // Always generate vars for the enumerated knapsacks
   MKC_check_enumerated_variables(kss, enumerated_ks, max_enumerated_size,
				  dual, gap, -rc_bound, new_mkc_vars,
				  print_best_dj);
   const int enumerated_size = new_mkc_vars.size();
   for (i = 0; i < enumerated_size; ++i)
      new_vars.push_back(new_mkc_vars[i]);

   size_t oldsize = new_vars.size();

   FILE* log = stdout;

   if (for_all_knapsack) {

      // First try to generate heuristically
      if (print_knapsacks) {
	 char logfile[200];
	 sprintf(logfile, "ks-%i-%i", index, iternum);
	 log = fopen(logfile, "w");
	 fprintf(log, "********** Starting column generation **********\n");
      }
      for (i = 0; i < ks_num; ++i) {
	 if (kss.ks_list[i].size <= max_enumerated_size)
	    continue;
	 MKC_generate_vars_one_ks(ks_num, i,
				  kss.ks_list[i], ks_fixings[i],
				  dual, new_vars, rc_bound, 0, log);
	 if (print_knapsacks)
	    fprintf(log, "\n");
	 if (oldsize < new_vars.size()) {
	    const double dj =
	       dynamic_cast<MKC_var*>(new_vars.back())->dj(dual);
	    if (print_best_dj)
	       printf("MKC: heur for ks %i: varnum %i, best dj %f.\n",
		      i, static_cast<int>(new_vars.size() - oldsize), dj);
	    if (!all_encountered_var) {
	       // keep only the last, as that is the best
	       purge_ptr_vector(new_vars,
				new_vars.entry(oldsize), new_vars.end()-1);
	    }
	    if (dj < best_dj)
	       best_dj = dj;
	    oldsize = new_vars.size();
	 }
      }
      if (print_knapsacks)
	 fprintf(log, "********** Finished column generation **********\n");
      const int heur_size = new_vars.size() - enumerated_size;
      if (heur_size > 0) {
	 printf("MKC: heur generated %i variables, best dj: %f.\n",
		heur_size, best_dj);
      } else {
	 printf("MKC: heur generated 0 variables.\n");
      }

      // heuristic didn't succeed, go for exact...
      if (fall_back_to_exact) {
	 for (i = 0; i < ks_num; ++i) {
	    if (kss.ks_list[i].size <= max_enumerated_size)
	       continue;
	    MKC_generate_vars_one_ks(ks_num, i,
				     kss.ks_list[i], ks_fixings[i],
				     dual, new_vars, rc_bound, 1, 0);
	    if (oldsize < new_vars.size()) {
	       const double dj =
		  dynamic_cast<MKC_var*>(new_vars.back())->dj(dual);
	       if (print_best_dj)
		  printf("MKC: exact for ks %i: varnum %i, best dj %f\n",
			 i, static_cast<int>(new_vars.size() - oldsize), dj);
	       if (!all_encountered_var) {
		  // keep only the last, as that is the best
		  purge_ptr_vector(new_vars,
				   new_vars.entry(oldsize), new_vars.end()-1);
	       }
	       if (dj < best_dj)
		  best_dj = dj;
	       oldsize = new_vars.size();
	    }
	 }
	 const int exact_size = new_vars.size() - enumerated_size;
	 if (exact_size > 0) {
	    printf("MKC: exact generated %i variables, best dj: %f.\n",
		   exact_size, best_dj);
	 } else {
	    printf("MKC: exact generated 0 variables.\n");
	 }
      }

   } else {

      // in a call to this function generate vars only for one ks, and in each
      // call for a different ks.

      static int ks_ind = -1;

      // First try to generate heuristically
      for (i = (ks_ind + 1) % ks_num; i != ks_ind; i = (i + 1) % ks_num) {
	 if (kss.ks_list[i].size <= max_enumerated_size)
	    continue;
	 MKC_generate_vars_one_ks(ks_num, i,
				  kss.ks_list[i], ks_fixings[i],
				  dual, new_vars, rc_bound, 0, 0);
	 if (0 < new_vars.size()) {
	    if (print_best_dj)
	       printf("MKC: heur for ks %i: varnum %i, best dj %f.\n",
		      i, static_cast<int>(new_vars.size()),
		      dynamic_cast<MKC_var*>(new_vars.back())->dj(dual));
	    if (!all_encountered_var) {
	       // keep only the last, as that is the best
	       purge_ptr_vector(new_vars,
				new_vars.entry(oldsize), new_vars.end()-1);
	    }
	    ks_ind = i;
	    return;
	 }
      }
      if (fall_back_to_exact) {
	 for (i = (ks_ind + 1) % ks_num; i != ks_ind; i = (i + 1) % ks_num) {
	    if (kss.ks_list[i].size <= max_enumerated_size)
	       continue;
	    MKC_generate_vars_one_ks(ks_num, i,
				     kss.ks_list[i], ks_fixings[i],
				     dual, new_vars, rc_bound, 1, 0);
	    if (0 < new_vars.size()) {
	       if (print_best_dj)
		  printf("MKC: exact for ks %i: varnum %i, best dj %f.\n",
			 i, static_cast<int>(new_vars.size()),
			 dynamic_cast<MKC_var*>(new_vars.back())->dj(dual));
	       if (!all_encountered_var) {
		  // keep only the last, as that is the best
		  purge_ptr_vector(new_vars,
				   new_vars.entry(oldsize), new_vars.end()-1);
	       }
	       ks_ind = i;
	       return;
	    }
	 }
      }
   }
}

//#############################################################################
// possible values for what_to_do:
// 0 - do heuristic
// 1 - do exact
// 2 - do lp relaxation
// 3 - do lp relaxation with fallback to exact.
//
// Return value:
// if what_to_do is 0/1 : rc_bound  or the best red cost found
// if what_to_do is 2/3 : 0  or the best red cost found
//
// Note: if what_to_do is 0/1 then we are looking for vars with rc better than
// rc_bound (i.e., red cost < -rc_bound). If what_to_do is 2/3 then we do the
// lp relaxation and if it is very bad ( < -rc_bound ), then in case of 3 we
// do exact KS.

double
MKC_generate_vars_one_ks(const int ks_num, const int ks_ind,
			 MKC_knapsack& ks_orig,
			 const MKC_knapsack_fixing& ksf,
			 const double* dual,
			 BCP_vec<BCP_var*>& new_vars,
			 const double rc_bound,
			 const int what_to_do,
			 FILE* log)
{
   MKC_knapsack_entry* entries_orig = ks_orig.entries;

   int j, k;

   // First compute the cost and ratio for every entry in the ks
   const double* order_dual = dual + ks_num;
   if (print_knapsacks && log)
      fprintf(log, "\nKnapsack %i : size %i , capacity %f\n",
	      ks_ind, ks_orig.size, ks_orig.capacity);
   for (j = ks_orig.size - 1; j >= 0; --j) {
      MKC_knapsack_entry &current = entries_orig[j];
      current.cost = order_dual[current.index] - current.orig_cost;
      current.ratio = current.cost > 0 ? current.cost / current.weight : 0;
      if (print_knapsacks && log) {
	 fprintf(log, "color: %3i  weight: %7.3f   orig_cost: %7.3f",
		 current.color, current.weight, current.orig_cost);
	 fprintf(log, "   dual: %10.3f   cost: %7.3f   ratio: %6.3f\n",
		 order_dual[current.index], current.cost, current.ratio);
      }
   }

   // Preprocess the KS
   double rc_adjust = 0;

   int color0_cnt = -1;
   int color1_cnt = -1;

   int clr[2] = {-1, -1};

   MKC_knapsack_entry * color0_list = 0;
   MKC_knapsack_entry * color1_list = 0;

   const int fixed_num = ksf.fixed_entry_num;
   if (fixed_num > 0) {
      for (j = 0; j < fixed_num; ++j) {
	 const int ind = ksf.fixed_entries[j];
	 rc_adjust += entries_orig[ind].cost;
	 k = entries_orig[ind].color;
	 if (clr[0] == -1) {
	    clr[0] = k;
	    continue;
	 }
	 if (clr[0] == k)
	    continue;

	 if (clr[1] == -1) {
	    clr[1] = k;
	    continue;
	 }
	 if (clr[1] == k)
	    continue;

	 abort();
      }

      if (clr[0] == -1) {
	 abort();
      } else {
	 const int first = ks_orig.color_beg[clr[0]];
	 const int last = ks_orig.color_beg[clr[0]+1];
	 color0_list = new MKC_knapsack_entry[last - first];
	 int k = 0;
	 for (j = first; j < last; ++j) {
	    if (ksf.fixing[entries_orig[j].index] == 1)
	       color0_list[k++] = entries_orig[j];
	 }
	 color0_cnt = k;
	 // order the entries in the color by their ratios (decreasing)
	 if (k > 0)
	    std::sort(color0_list, color0_list+k, MKC_ks_entry_ratio_descend);
      }

      if (clr[1] != -1) {
	 const int first = ks_orig.color_beg[clr[1]];
	 const int last = ks_orig.color_beg[clr[1]+1];
	 color1_list = new MKC_knapsack_entry[last - first];
	 int k = 0;
	 for (j = first; j < last; ++j) {
	    if (ksf.fixing[entries_orig[j].index] == 1)
	       color1_list[k++] = entries_orig[j];
	 }
	 color1_cnt = k;
	 // order the entries in the color by their ratios (decreasing)
	 if (k > 1)
	    std::sort(color1_list, color1_list+k, MKC_ks_entry_ratio_descend);
      }
   }

   // Create a new knapsack by throwing out the entries that cannot be in the
   // KS. Also throw out those that are listed in the color[01]_list.
   MKC_knapsack ks;
   ks.cost = ks_orig.cost;
   ks.capacity = ks_orig.capacity - ksf.fixed_weight;
   ks.entries = new MKC_knapsack_entry[ks_orig.size];
   MKC_knapsack_entry * entries = ks.entries;
   ks.size = 0;

   for (j = 0; j < ks_orig.size; ++j) {
      const int index = entries_orig[j].index;
      if (ksf.fixing[index] == 0 ||
	  entries_orig[j].ratio <= 0.000001 ||
	  entries_orig[j].weight > ks.capacity)
	 continue;

      if (clr[0] != -1)
	 if (entries_orig[j].color == clr[0])
	    continue;
      if (clr[1] != -1)
	 if (entries_orig[j].color == clr[1])
	    continue;
      entries[ks.size++] = entries_orig[j];
   }

   if (ks.size > 0) {
      ks.color_beg = new int[ks_orig.color_num + 1];
      ks.color_beg[0] = 0;
      for (k = 0, j = 1; j < ks.size; ++j) {
	 if (entries[j - 1].color != entries[j].color)
	    ks.color_beg[++k] = j;
      }
      ks.color_beg[++k] = ks.size;
      ks.color_num = k;
      // order the entries in each color by their ratios (decreasing)
      for (j = 0; j < ks.color_num; ++j) {
	 std::sort(entries + ks.color_beg[j], entries + ks.color_beg[j+1],
		   MKC_ks_entry_ratio_descend);
      }
   } else {
      ks.color_beg = 0;
      ks.color_num = 0;
   }
   const int color_num = ks.color_num;
   const int *color_beg = ks.color_beg;
   

   // throw out the heavy ones from the two color_list's
   if (clr[0] != -1) {
      for (k = 0, j = 0; j < color0_cnt; ++j) {
	 if (color0_list[j].weight <= ks.capacity)
	    color0_list[k++] = color0_list[j];
      }
      color0_cnt = k;
   }
   if (clr[1] != -1) {
      for (k = 0, j = 0; j < color1_cnt; ++j) {
	 if (color1_list[j].weight <= ks.capacity)
	    color1_list[k++] = color1_list[j];
      }
      color1_cnt = k;
   }

   // Oh boy... Now the nonfixed entries with colors different than those of
   // the fixed entries are listed in the new knapsack, and the entries with
   // colors same as the fixed entries are lised in color[01]_list

   MKC_knapsack_entry* sublist = new MKC_knapsack_entry[ks_orig.size + 1];
   int* tmp_chosen = new int[ks_orig.size];

   // somthing to store the colors and their best ratio in.
   BCP_vec<int> color_order;
   BCP_vec<double> color_ratio;
   color_order.reserve(ks_orig.size); // an upper bound on the num of colors
   color_ratio.reserve(ks_orig.size);

   // Now do a heuristic for the KS and see if we find anything
   double cutoff =
      ks.cost - rc_adjust - dual[ks_ind] + (what_to_do < 2 ? rc_bound : 0);

   // Depending on how many fixed colors we have for the knapsack we
   // handle it differently:
   if (clr[1] != -1) {
      // We got both colors fixed!
      const int sublist_size = color0_cnt + color1_cnt;
      // this'll work even if sublist_size is 0.
      std::merge(color0_list, color0_list + color0_cnt,
		 color1_list, color1_list + color1_cnt,
		 sublist, MKC_ks_entry_ratio_descend);
      MKC_do_the_knapsack(clr, sublist, sublist_size, ks, ksf, ks_ind,
			  ks_num, cutoff,
			  new_vars, tmp_chosen, what_to_do);
   } else {
      
      if (color_num > 1) {
	 color_order.clear();
	 color_ratio.clear();
	 // order the colors in increasing order of their best ratios
	 for (j = 0; j < color_num; ++j) {
	    color_order.unchecked_push_back(j);
	    color_ratio.unchecked_push_back(entries[color_beg[j]].ratio);
	 }
	 CoinSort_2(color_ratio.begin(), color_ratio.end(),
		    color_order.begin());
      }

      if (clr[0] != -1) {
	 // We've got one color fixed

	 if (color_num == 0) {
	    // First handle the case when that color is the ONLY color
	    MKC_do_the_knapsack(clr, color0_list, color0_cnt, ks, ksf, ks_ind,
				ks_num, cutoff,
				new_vars, tmp_chosen, what_to_do);
	 } else {
	    // There is another color
	    int j = 0;
	    if (color0_list[0].ratio * ks.capacity < cutoff) {
	       for ( ; j < color_num; ++j)
		  if (color_ratio[j] * ks.capacity >= cutoff)
		     break;
	    }
	    for (; j < color_num; ++j) {
	       const int cbj = color_beg[color_order[j]];
	       const int cbj1 = color_beg[color_order[j]+1];
	       const int sublist_size = color0_cnt + cbj1 - cbj;
	       std::merge(color0_list, color0_list + color0_cnt,
			  entries + cbj, entries + cbj1,
			  sublist, MKC_ks_entry_ratio_descend);
	       MKC_do_the_knapsack(clr, sublist, sublist_size, ks, ksf, ks_ind,
				   ks_num, cutoff,
				   new_vars, tmp_chosen, what_to_do);
	    }
	 }

      } else {
	 // We don't have any colors fixed.

	 if (color_num == 1) {
	    // Again, first check if there's only ONE color
	    MKC_do_the_knapsack(clr, entries, ks.size, ks, ksf, ks_ind,
				ks_num, cutoff,
				new_vars, tmp_chosen, what_to_do);
	 } else {
	    // We've got at least two colors
	    // Now for every pair of colors that has a chance do a greedy
	    // heuristic
	    for (j = 0; j < color_num; ++j)
	       if (color_ratio[j] * ks.capacity >= cutoff)
		  break;
	    for (; j < color_num; ++j) {
	       for (k = 0; k < j; ++k) {
		  const int cbj = color_beg[color_order[j]];
		  const int cbj1 = color_beg[color_order[j]+1];
		  const int cbk = color_beg[color_order[k]];
		  const int cbk1 = color_beg[color_order[k]+1];
		  const int sublist_size = cbj1 - cbj + cbk1 - cbk;
		  std::merge(entries + cbj, entries + cbj1,
			     entries + cbk, entries + cbk1,
			     sublist, MKC_ks_entry_ratio_descend);
		  MKC_do_the_knapsack(clr,
				      sublist, sublist_size, ks, ksf, ks_ind,
				      ks_num, cutoff,
				      new_vars, tmp_chosen, what_to_do);
	       }
	    }
	 }
      }
   }

   // clean up
   delete[] tmp_chosen;
   delete[] sublist;
   delete[] color0_list;
   delete[] color1_list;

   return ks.cost - rc_adjust - dual[ks_ind] - cutoff;
}

//#############################################################################

void
MKC_do_the_knapsack(const int clr[2],
		    const MKC_knapsack_entry * sublist,
		    const int sublist_size,
		    const MKC_knapsack& ks,
		    const MKC_knapsack_fixing& ksf,
		    const int ks_ind,
		    const int ks_num,
		    double& cutoff,
		    BCP_vec<BCP_var*>& new_vars,
		    int* tmp_chosen,
		    const int what_to_do)
{
   switch (what_to_do) {
    case 2:
    case 3:
      {
	 double sum = ks.capacity;
	 double bestposs = 0.0;
	 for (int l = 0; l < sublist_size; ++l) {
	    if (sublist[l].weight < sum) {
	       sum -= sublist[l].weight;
      		bestposs += sublist[l].cost;
	    } else {
	       bestposs += sublist[l].ratio * sum;
	       break;
	    }
	 }
	 if (cutoff < bestposs)
	    if (what_to_do == 3)
	       MKC_exact_knapsack(clr, sublist, sublist_size,
				  ksf, ks.capacity, ks.cost, ks_ind, ks_num,
				  cutoff, new_vars, tmp_chosen);
	    else
	       cutoff = bestposs;
      }
      break;
    case 1:
      MKC_exact_knapsack(clr, sublist, sublist_size,
			 ksf, ks.capacity, ks.cost, ks_ind, ks_num,
			 cutoff, new_vars, tmp_chosen);
      break;
    case 0:
      MKC_greedy_knapsack(clr, sublist, sublist_size,
			  ksf, ks.capacity, ks.cost, ks_ind, ks_num,
			  cutoff, new_vars, tmp_chosen);
      break;
   }
}

//#############################################################################

void
MKC_check_enumerated_variables(const MKC_knapsack_set& kss,
			       BCP_vec<MKC_var*>* enumerated_ks,
			       const int max_enumerated_size,
			       const double* dual,
			       const double gap, const double rc_bound,
			       BCP_vec<MKC_var*>& new_vars,
			       const bool print_best_dj)
{
   if (! enumerated_ks)
      return;

   const int ks_num = kss.ks_num;
   const int old_nv_size = new_vars.size();
   double global_best_dj = 0.0;

   int i, j;
   for (i = 0; i < ks_num; ++i) {
      MKC_knapsack& ks = kss.ks_list[i];
      if (ks.size > max_enumerated_size)
	 continue;

      BCP_vec<MKC_var*>& eks = enumerated_ks[i];
      const int oldsize = eks.size();
      int size = oldsize;
      double best_dj = rc_bound;
      int best_ind = -1;
      for (j = 0; j < size; ) {
	 const double dj = eks[j]->dj(dual);
	 if (dj > gap) {
	    delete eks[j];
	    eks[j] = eks.back();
	    eks.pop_back();
	    --size;
	    continue;
	 }
	 if (dj < best_dj) {
	    best_dj = dj;
	    best_ind = j;
	 }
	 ++j;
      }
      if (print_best_dj) {
	 printf("MKC: checking enumerated knapsacks...\n");
	 if (best_ind == -1)
	    printf("MKC: enumeration for ks %i: deleted %i, found nothing.\n",
		   i, oldsize - size);
	 else
	    printf("MKC: enumeration for ks %i: deleted %i, best dj %f.\n",
		   i, oldsize - size, best_dj);
      }
      if (best_ind >= 0) {
	 // make a copy of eks[j]
	 new_vars.push_back(new MKC_var(*eks[best_ind]));
	 global_best_dj = best_dj;
      }
   }

   const int new_var_num = new_vars.size() - old_nv_size;
   if (new_var_num > 0) {
      printf("MKC: enumeration found %i variables, best dj: %f.\n",
	     new_var_num, global_best_dj);
   } else {
      printf("MKC: enumeration found 0 variables.\n");
   }
}
