// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>

#include "CSP_userexits.hpp"
#include "CSP_colgen.hpp"

UserData* initializeUserData(CSP_subProblem* sp,
			     OsiClpSolverInterface* solver,
			     int baseRows)
{
   UserData *userData = new UserData;
   userData->subProblem = sp;
   userData->solver = solver;
   userData->nBaseRows = baseRows;
   return userData;
}

