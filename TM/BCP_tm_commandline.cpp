// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_tm_functions.hpp"

#include "BCP_tm.hpp"

//#############################################################################

void
BCP_tm_parse_command_line(BCP_tm_prob& p,
			  const int argnum, const char* const * arglist)
{
  if (argnum != 2) {
    throw BCP_fatal_error("Usage: bcp <parameter_file_name>\n");
  }

  // Read in the parameters
  p.par.read_from_file(arglist[1]);
  p.slave_pars.lp.read_from_file(arglist[1]);
  p.slave_pars.cg.read_from_file(arglist[1]);
  p.slave_pars.vg.read_from_file(arglist[1]);

  // check the consistency of the parameters
  BCP_check_parameters(p);
}
