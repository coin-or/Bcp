// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cstring>

#include "BCP_tm_functions.hpp"

#include "BCP_tm.hpp"

//#############################################################################

void
BCP_tm_parse_command_line(BCP_tm_prob& p,
			  const int argnum, const char* const * arglist)
{
  if (argnum == 2) {
     // Read in the parameters
     p.par.read_from_file(arglist[1]);
     p.slave_pars.lp.read_from_file(arglist[1]);
     p.slave_pars.cg.read_from_file(arglist[1]);
     p.slave_pars.vg.read_from_file(arglist[1]);
  } else if (argnum == 1) {
     // work with default parameters
  } else {
    int i = 1;
    while (i < argnum) {
      if (strncmp(arglist[i], "-par", 4) == 0) {
	p.par.read_from_file(arglist[i+1]);
	p.slave_pars.lp.read_from_file(arglist[i+1]);
	p.slave_pars.cg.read_from_file(arglist[i+1]);
	p.slave_pars.vg.read_from_file(arglist[i+1]);
      } else {
	if (i+1 == argnum) {
	  printf("***WARNING*** BCP: Parameter key \"%s without value.\n",
		 arglist[i]);
	} else {
	  printf("***WARNING*** BCP:Parameter \"%s %s\" is unrecognized.\n",
		 arglist[i], arglist[i+i]);
	}
      }
      i += 2;
    }
  }
  // check the consistency of the parameters
  BCP_check_parameters(p);
}
