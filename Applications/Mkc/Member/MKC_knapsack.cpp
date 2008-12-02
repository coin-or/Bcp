// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_buffer.hpp"
#include "MKC_knapsack.hpp"

//#############################################################################

void
MKC_knapsack_entry::pack(BCP_buffer& buf)
{
   buf.pack(index)
      .pack(color)
      .pack(weight)
      .pack(orig_cost);
   // the rest are local info
}

//#############################################################################

void
MKC_knapsack_entry::unpack(BCP_buffer& buf)
{
   buf.unpack(index)
      .unpack(color)
      .unpack(weight)
      .unpack(orig_cost);
   // the rest are local info
}

//#############################################################################

void
MKC_knapsack::pack(BCP_buffer& buf)
{
   buf.pack(cost)
      .pack(capacity)
      .pack(size)
      .pack(color_beg, color_num + 1);
   for (int i = 0; i < size; ++i)
      entries[i].pack(buf);
}

//#############################################################################

void
MKC_knapsack::unpack(BCP_buffer& buf)
{
   buf.unpack(cost)
      .unpack(capacity)
      .unpack(size)
      .unpack(color_beg, color_num);
   --color_num;
   entries = new MKC_knapsack_entry[size];
   for (int i = 0; i < size; ++i)
      entries[i].unpack(buf);
}

//#############################################################################

void
MKC_knapsack_set::pack(BCP_buffer& buf)
{
   buf.pack(ks_num)
      .pack(order_num);
   for (int i = 0; i < ks_num; ++i)
      ks_list[i].pack(buf);
   // no orig_var_num/orig_name_list; they are used only in the TM, no need to
   // pack/unpack it.
}

//#############################################################################

void
MKC_knapsack_set::unpack(BCP_buffer& buf)
{
   buf.unpack(ks_num)
      .unpack(order_num);
   delete[] ks_list;
   ks_list = new MKC_knapsack[ks_num];
   for (int i = 0; i < ks_num; ++i)
      ks_list[i].unpack(buf);
   // no orig_var_num/orig_name_list; they are used only in the TM, no need to
   // pack/unpack it.
}
