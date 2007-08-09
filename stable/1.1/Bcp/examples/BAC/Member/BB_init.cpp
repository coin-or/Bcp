// Last edit: 12/26/05
//
// Name:     BB_tm.cpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BB_init.hpp"
#include "BB_lp.hpp"
#include "BB_tm.hpp"

using namespace std;

/****************************************************************************/
BCP_lp_user *
BB_init::lp_init(BCP_lp_prob& p)
{
  return new BB_lp;
}

/****************************************************************************/
BCP_tm_user *
BB_init::tm_init(BCP_tm_prob& p,
		 const int argnum, const char * const * arglist)
{
  
  cout << "Compilation flags: ";
  
#ifdef HEUR_SOL
  cout << "HEUR_SOL ";
#endif
  
#ifdef CUSTOM_BRANCH
  cout << "CUSTOM_BRANCH ";
#endif
  
#ifdef USER_DATA
  cout << "USER_DATA ";
#endif

  cout << endl << endl;
  
  
  BB_tm* tm = new BB_tm;

  if(argnum > 1) {
    tm->readInput(arglist[1]);
  }
  else {
    tm->readInput(NULL);
  }
  return tm;
}
