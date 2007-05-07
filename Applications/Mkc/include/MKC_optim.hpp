// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_OPTIM_H
#define _MKC_OPTIM_H

#include "BCP_vector.hpp"

class MKC_knapsack_entry;
class MKC_knapsack_fixing;
class MKC_knapsack_set;
class BCP_var;
class MKC_var;


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
		    int * tmp_chosen);

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
		   int * tmp_chosen);

MKC_var*
MKC_create_var(const int clr[2],
	       const MKC_knapsack_entry* entries,
	       const int * entry_ind, const int size,
	       const double ks_cost, const int ks_num, const int ks_ind,
	       const MKC_knapsack_fixing& ksf);

void
MKC_enumerate_knapsacks(MKC_knapsack_set& kss,
			BCP_vec<MKC_var*>* enumerated_ks,
			const int max_enumerated_size);

void
MKC_enumerate_one_ks(const int clr[2],
		     const MKC_knapsack_entry * entries, const int size,
		     BCP_vec<MKC_var*>& enumerated,
		     const int ks_num, const int ks_ind, const double ks_cost,
		     const double ks_capacity, char * flag, double * w);

#endif
