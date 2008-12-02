// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_SOLUTION_H
#define _MKC_SOLUTION_H

#include "BCP_vector.hpp"
#include "BCP_solution.hpp"

class MKC_var;
class BCP_buffer;

//#############################################################################

class MKC_solution : public BCP_solution {
public:
  double _objective;
  BCP_vec<MKC_var*> _vars;
public:
  MKC_solution(BCP_vec<MKC_var*>& vars, const double obj);
  MKC_solution(BCP_buffer& buf);
  ~MKC_solution();

   inline double objective_value() const { return _objective; }

   void pack(BCP_buffer& buf) const;
};



#endif
