// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "MKC_solution.hpp"
#include "MKC_var.hpp"

#include "BCP_buffer.hpp"

MKC_solution::MKC_solution(BCP_vec<MKC_var*>& vars, const double obj) :
   _objective(obj)
{
   _vars.swap(vars);
}

MKC_solution::MKC_solution(BCP_buffer& buf)
{
   int varnum = 0;
   buf.unpack(_objective)
      .unpack(varnum);
   if (varnum > 0) {
      _vars.reserve(varnum);
      for (int i = 0; i < varnum; ++i) {
	 _vars.unchecked_push_back(new MKC_var(buf));
      }
   }
}

MKC_solution::~MKC_solution() {
   purge_ptr_vector(_vars);
}

void
MKC_solution::pack(BCP_buffer& buf) const
{
   const int varnum = _vars.size();
   buf.pack(_objective)
      .pack(varnum);
   for (int i = 0; i < varnum; ++i)
      _vars[i]->pack(buf);
}
