// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_KNAPSACK_H
#define _MKC_KNAPSACK_H

#include "BCP_vector.hpp"

class BCP_buffer;

//#############################################################################

// This structure corresponds to an order
struct MKC_knapsack_entry {
   int    orig_index; // it's index in the original list of vars
   int    index;      // the index of the order
   int    color;
   double weight;
   double orig_cost;  // the original cost of the order
   double cost;       // the costof the entry in the knapsack subproblems
   double ratio;      // the cost/weight ratio in the knapsack subproblems
public:
   MKC_knapsack_entry() : orig_index(-2), index(-2), color(-2),
      weight(0.0), orig_cost(0.0), cost(0.0), ratio(0.0) {}
   ~MKC_knapsack_entry() {}
   void pack(BCP_buffer& buf);
   void unpack(BCP_buffer& buf);
};

//#############################################################################

inline bool MKC_ks_entry_color_ascend(const MKC_knapsack_entry& order0,
				      const MKC_knapsack_entry& order1)
{
   return order0.color < order1.color;
}

inline bool MKC_ks_entry_ratio_descend(const MKC_knapsack_entry& order0,
				       const MKC_knapsack_entry& order1)
{
   return order0.ratio > order1.ratio;
}

inline bool MKC_ks_entry_weight_ascend(const MKC_knapsack_entry& order0,
				       const MKC_knapsack_entry& order1)
{
   return order0.weight < order1.weight;
}

inline bool MKC_ks_entry_weight_descend(const MKC_knapsack_entry& order0,
					const MKC_knapsack_entry& order1)
{
   return order0.weight < order1.weight;
}

//#############################################################################

class MKC_knapsack_fixing {
public:
   // An entry in the char vector is
   // 0: if the order can't be in the corresponding knapsack
   // 1: if there are no restrictions for that order
   // Note: those that must be in the KS are already listed in the fixed
   // entries, thus they will have a 0 in fixing.
   BCP_vec<char> fixing; //
   int fixed_entry_num;
   int * fixed_entries; // index in the KS
   int * fixed_entries_ind; // index of the order
   double fixed_cost;
   double fixed_weight;
public:
   MKC_knapsack_fixing() :
      fixed_entry_num(0), fixed_entries(0), fixed_entries_ind(0),
      fixed_cost(0.0), fixed_weight(0.0) {}
   ~MKC_knapsack_fixing() {
      delete[] fixed_entries_ind;
      delete[] fixed_entries;
   }
};

//#############################################################################

// This structure corresponds to a slab
class MKC_knapsack {
public:
   double cost;     // the cost of using this slab at all
   double capacity; 
   int size;        // the numbor of KS entries (orders) that can go into this
                    // KS (slab) at all.
   MKC_knapsack_entry * entries; // the orders that can go into this slab
                    // These entries are ordered by color
   int color_num;   // the number of different colors among the entries
   int * color_beg; // length: color_num + 1; the positions where the colors
                    // start in entries (remember, it's ordered by color)
   int orig_index; // the index of the corresponding z variable in the
		   // original list of vars
public:
   MKC_knapsack() :
      cost(0.0), capacity(0.0), size(0), entries(0),
      color_num(0), color_beg(0), orig_index(-1) {}
   ~MKC_knapsack() {
      delete[] entries;
      delete[] color_beg;
   }

   void pack(BCP_buffer& buf);
   void unpack(BCP_buffer& buf);
};

//#############################################################################

class MKC_knapsack_set {
public:
  int order_num;         // the number of orders (KS entries)
  int ks_num;            // the number of slabs (knapsacks)
  MKC_knapsack* ks_list; // the description of the KSs
  int orig_var_num;      
  char **orig_name_list;
  double * orig_row_0;
  double * orig_row_1;
public:
   MKC_knapsack_set() :
      order_num(0), ks_num(0), ks_list(0),
      orig_var_num(0), orig_name_list(0), orig_row_0(0), orig_row_1(0) {}
   ~MKC_knapsack_set() {
      delete[] ks_list;
      if (orig_name_list) {
	 for (int i = 0; i < orig_var_num; ++i)
	    delete[] orig_name_list[i];
	 delete[] orig_name_list;
      }
      delete[] orig_row_0;
      delete[] orig_row_1;
   }

   void pack(BCP_buffer& buf);
   void unpack(BCP_buffer& buf);
};

#endif
