// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_VAR_H
#define _MKC_VAR_H

#include <cfloat>
#include "BCP_vector.hpp"
#include "BCP_var.hpp"
#include "BCP_buffer.hpp"

enum MKC_var_t {
   MKC_RegularVar,
   MKC_BranchingVar
};

class MKC_var : public BCP_var_algo {
public:
   double cost;   // the cost of this variable
   int entry_num; // 1  + the number of objects put into the KS.
   int * entries; // the first entry is the index of the KS this var belongs
		  // to, ks_index the orders put into the KS.
   int color[2];  // the two colors of the variable
public:
   MKC_var(const double c, const int num, int *& entr, const int clr[2]) :
      BCP_var_algo(BCP_BinaryVar, c, 0.0, 1.0),
      cost(c), entry_num(num), entries(entr) {
	 entr = 0;
	 color[0] = clr[0];
	 color[1] = clr[1];
   }
   MKC_var(const MKC_var& x) :
      BCP_var_algo(BCP_BinaryVar, x.cost, 0.0, 1.0),
      cost(x.cost), entry_num(x.entry_num), entries(new int[x.entry_num]) {
	 memcpy(entries, x.entries, x.entry_num * sizeof(int));
	 color[0] = x.color[0];
	 color[1] = x.color[1];
   }
   MKC_var(BCP_buffer& buf) :
     BCP_var_algo(BCP_BinaryVar, 0.0, 0.0, 1.0),
     cost(0.0), entry_num(0), entries(0) {
     buf.unpack(cost).unpack(entries, entry_num)
        .unpack(color[0]).unpack(color[1]);
     set_obj(cost);
   }

   ~MKC_var() {
      delete[] entries;
   }

public:
  double dj(const double* dual) const {
    double red_cost = cost;
    for (int i = entry_num - 1; i >= 0; --i)
      red_cost -= dual[entries[i]];
    return red_cost;
  }

  void pack(BCP_buffer& buf) const {
    buf.pack(cost).pack(entries, entry_num).pack(color[0]).pack(color[1]);
  }
};

//#############################################################################

class MKC_branching_var : public BCP_var_algo {
public:
   // the index of the order that is assigned to a list of slabs
   int order_index;
   int slab_num;
   int * slab_list;
public:
   MKC_branching_var(const int order_ind, const int num, int *& slist) :
      BCP_var_algo(BCP_ContinuousVar, 0.0, 0.0, 1e40),
      order_index(order_ind), slab_num(num), slab_list(slist) {
	 slist = 0;
   }
   MKC_branching_var(BCP_buffer& buf) :
     BCP_var_algo(BCP_ContinuousVar, 0.0, 0.0, 1e40),
     order_index(-1), slab_num(0), slab_list(0) {
     buf.unpack(order_index).unpack(slab_list, slab_num);
   }

   ~MKC_branching_var() {
      delete[] slab_list;
   }

  void pack(BCP_buffer& buf) const {
    buf.pack(order_index).pack(slab_list, slab_num);
  }
};

//#############################################################################

inline BCP_var_algo*
MKC_var_unpack(BCP_buffer& buf)
{
  int type;
  buf.unpack(type);
  switch (type) {
  case MKC_RegularVar: return new MKC_var(buf);
  case MKC_BranchingVar: return new MKC_branching_var(buf);
  }
  throw BCP_fatal_error("MKC: Unknown variable type in MKC_var_unpack().\n");
  return 0; // fake return
}

//#############################################################################

inline void
MKC_var_pack(const BCP_var_algo* var, BCP_buffer& buf)
{
  const MKC_var* regular_var = dynamic_cast<const MKC_var*>(var);
  if (regular_var) {
    const int type = MKC_RegularVar;
    buf.pack(type);
    regular_var->pack(buf);
    return;
  }

  const MKC_branching_var* b_var = dynamic_cast<const MKC_branching_var*>(var);
  if (b_var) {
    const int type = MKC_BranchingVar;
    buf.pack(type);
    b_var->pack(buf);
    return;
  }

  throw BCP_fatal_error("MKC: Unknown variable type in MKC_var_pack().\n");
}

#endif
