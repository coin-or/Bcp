// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_solution.hpp"
#include "BCP_var.hpp"

void
BCP_solution_generic::display() const
{
  const int num = _vars.size();
  for (int i = 0; i < num; ++i) {
    _vars[i]->display(_values[i]);
  }
}
