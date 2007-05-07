// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include <algorithm>

#include "MKC_optim.hpp"
#include "MKC_knapsack.hpp"
#include "MKC_var.hpp"

//#############################################################################

void
MKC_greedy_knapsack(const int clr[2],
		    const MKC_knapsack_entry* entries, // this knapsack prob
		    const int entry_num,
		    // for this KS:
		    // fixings, capacity, cost, index of it in the list
		    const MKC_knapsack_fixing& ksf,
		    const double capacity,
		    const double ks_cost,
		    const int ks_ind,
		    // the total number of knapsacks
		    const int ks_num,
		    // how good sol's we are looking for
		    double& cutoff,
		    // the generated vars
		    BCP_vec<BCP_var*>& new_vars,
		    // temporary vector
		    int * tmp_chosen)
{
   int i, size = 0;
   double bestposs=0.0;
   double sum = capacity + 0.000001;
   for (i = 0; i < entry_num; ++i) {
      if (entries[i].weight < sum) {
	 sum -= entries[i].weight;
	 bestposs += entries[i].cost;
	 tmp_chosen[size++] = i;
      } else {
	 if (bestposs + entries[i].ratio * sum < cutoff)
	    break;
      }
   }
   if (bestposs > cutoff) {
       new_vars.push_back(MKC_create_var(clr, entries, tmp_chosen, size,
					 ks_cost, ks_num, ks_ind, ksf));
       cutoff = bestposs;
   }
}

//#############################################################################

void
MKC_exact_knapsack(const int clr[2],
		   const MKC_knapsack_entry* entries, // this knapsack prob
		   const int entry_num,
		   // for this KS:
		   // fixings, capacity, cost, index of it in the list
		   const MKC_knapsack_fixing& ksf,
		   const double capacity,
		   const double ks_cost,
		   const int ks_ind,
		   // the total number of knapsacks
		   const int ks_num,
		   // how good sol's we are looking for
		   double& cutoff,
		   // the generated vars
		   BCP_vec<BCP_var*>& new_vars,
		   // temporary vector
		   int * tmp_chosen)
{
   double bestposs=0.0;
   /*printf("number %d cutoff %g rhs %g\n",entry_num,cutoff,rhs);*/
   double sum = capacity;
   int size = 0;
   int i = 0;

   // First do a quick LP relaxation
   for (int l = 0; l < entry_num; ++l) {
      if (entries[l].weight < sum) {
	 sum -= entries[l].weight;
	 bestposs += entries[l].cost;
      } else {
	 bestposs += entries[l].ratio * sum;
	 break;
      }
   }
   if (bestposs < cutoff)
      return;

   sum = capacity;
   bestposs=0.0;

   while (true) {
      int sequence_start = size;
      while (i < entry_num) {
	 if (entries[i].weight < sum) {
	    sum -= entries[i].weight;
	    bestposs += entries[i].cost;
	    tmp_chosen[size++] = i;
	 } else {
	    if (bestposs + entries[i].ratio * sum <= cutoff)
	       break;
	    sequence_start = size;
	 }
	 ++i;
      }
      if (bestposs > cutoff) {
	 new_vars.push_back(MKC_create_var(clr, entries, tmp_chosen, size,
					   ks_cost, ks_num, ks_ind, ksf));
	 cutoff = bestposs;
      }
      // Now everything in [ tmp_chosen[sequence_start] , entry_num ) is
      // included in the KS. The next KS to be checked is the one where the
      // sequence_start-1-st entry in tmp_chosen is forced to 0.
      if (sequence_start == 0)
	 return;
      // ok, now reset those entries in the sequence
      while (size > sequence_start) {
	 const int j = tmp_chosen[--size];
	 sum += entries[j].weight;
	 bestposs -= entries[j].cost;
      }
      // now find the last entry in tmp_chosen that can be set to 0 and still
      // the lp relax for the rest is not too bad
      while (size > 0) {
	 const int j = tmp_chosen[--size];
	 sum += entries[j].weight;
	 bestposs -= entries[j].cost;
	 i = j + 1;
	 if (bestposs + entries[i].ratio * sum > cutoff)
	    break;
      }
      if (size == 0 &&
	  bestposs + entries[i].ratio * sum <= cutoff)
	 break;
   }
}

//#############################################################################

MKC_var*
MKC_create_var(const int clr[2],
	       const MKC_knapsack_entry* entries,
	       const int * entry_ind, const int size,
	       const double ks_cost, const int ks_num, const int ks_ind,
	       const MKC_knapsack_fixing& ksf)
{
   const int newsize = size + ksf.fixed_entry_num;
   int * var_entries = new int[newsize + 1];
   *var_entries++ = ks_ind;
   double cost = ks_cost + ksf.fixed_cost;
   for (int i = 0; i < size; ++i) {
      const MKC_knapsack_entry& entry = entries[entry_ind[i]];
      *var_entries++ = entry.index + ks_num;
      cost += entry.orig_cost;
   }
   for (int i = 0; i < ksf.fixed_entry_num; ++i) {
      *var_entries++ = ksf.fixed_entries_ind[i] + ks_num;
   }
   var_entries -= newsize;
   std::sort(var_entries, var_entries + newsize);
   --var_entries;
   return new MKC_var(cost, newsize + 1, var_entries, clr);
}

//#############################################################################

void
MKC_enumerate_knapsacks(MKC_knapsack_set& kss,
			BCP_vec<MKC_var*>* enumerated_ks,
			const int max_enumerated_size)
{
   const int ks_num = kss.ks_num;
   MKC_knapsack_entry * sublist = new MKC_knapsack_entry[kss.order_num];
   double * w = new double[kss.order_num + 1];
   char * flag = new char[kss.order_num + 1];
   int enumerated_num = 0;
   int i, j, k;
   int clr[2];

   for (i = 0; i < ks_num; ++i) {
      MKC_knapsack& ks = kss.ks_list[i];
      if (ks.size > max_enumerated_size)
	 continue;

      BCP_vec<MKC_var*>& enumerated = enumerated_ks[i];
      MKC_knapsack_entry* entries = ks.entries;
      const int color_num = ks.color_num;
      const int * color_beg = ks.color_beg;
      // order the entries in increasing weight within each color
      for (j = 0; j < color_num; ++j) {
	 std::sort(entries + color_beg[j], entries + color_beg[j+1],
		   MKC_ks_entry_weight_ascend);
      }

      if (color_num == 1) {
	 clr[0] = entries[0].color;
	 clr[1] = -1;
	 // If there's only one color....
	 MKC_enumerate_one_ks(clr, entries, ks.size, enumerated,
			      ks_num, i, ks.cost, ks.capacity, flag, w);
      } else {
	 // Otherwise we have more than one colors
	 for (j = 0; j < color_num; ++j) {
	    const int cbj = color_beg[j];
	    const int cbj1 = color_beg[j+1];
	    clr[0] = entries[cbj].color;
	    for (k = 0; k < j; ++k) {
	       const int cbk = color_beg[k];
	       const int cbk1 = color_beg[k+1];
	       const int sublist_size = cbj1 - cbj + cbk1 - cbk;
	       clr[1] = entries[cbk].color;
	       std::merge(entries + cbj, entries + cbj1,
			  entries + cbk, entries + cbk1,
			  sublist, MKC_ks_entry_weight_ascend);
	       MKC_enumerate_one_ks(clr, sublist, sublist_size, enumerated,
				    ks_num, i, ks.cost, ks.capacity, flag, w);
	    } // for k
	 } // for j
      }

      printf("MKC: For knapsack %2i generated %7i variables.\n", i,
	     static_cast<int>(enumerated.size()));
      enumerated_num += enumerated.size();
   }
   delete[] flag;
   delete[] w;
   delete[] sublist;

   printf("\nMKC: Generated a total of %i enumerated variables.\n\n",
	  enumerated_num);
}

//#############################################################################

void
MKC_enumerate_one_ks(const int clr[2],
		     const MKC_knapsack_entry * entries, const int size,
		     BCP_vec<MKC_var*>& enumerated,
		     const int ks_num, const int ks_ind, const double ks_cost,
		     const double ks_capacity, char * flag, double * w)
{
   int newsize;
   int l, m;
   double cost;
   double sum = ks_capacity + 0.000001;

   memset(flag, false, (size + 1));
   for (l = 0; l < size; ++l)
      w[l] = entries[l].weight;
   w[size] = sum + 1.0;

   for (l = 0; true; ++l) {
      if (w[l] < sum) {
	 sum -= w[l];
	 flag[l] = true;
	 // create a var
	 for (cost = ks_cost, newsize = 0, m = l; m >= 0; --m) {
	    if (flag[m]) {
	       cost += entries[m].orig_cost;
	       ++newsize;
	    }
	 }
	 if (cost >= -1e-8)
	    continue;
	 int * var_entries = new int[newsize + 1];
	 *var_entries++ = ks_ind;
	 for (m = l; m >= 0; --m) {
	    if (flag[m])
	       *var_entries++ = entries[m].index + ks_num;
	 }
	 var_entries -= newsize;
	 std::sort(var_entries, var_entries + newsize);
	 --var_entries;
	 enumerated.push_back(new MKC_var(cost, newsize+1, var_entries, clr));
	 // finished creating the var
      } else {
	 while (l >=0 && !flag[l])
	    --l;
	 if (l < 0)
	    break;
	 sum += w[l];
	 flag[l] = false;
      }
   } // for l
}
