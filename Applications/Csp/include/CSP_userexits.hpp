// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef CSP_USEREXITS_H
#define CSP_USEREXITS_H

#include <algorithm>
#include <vector>

#include <OsiClpSolverInterface.hpp>

class CSP_subProblem;

class UserData {
public:
   CSP_subProblem* subProblem;
   OsiClpSolverInterface* solver;
   int cutMode;
   int nBaseRows;
   std::vector<const double*> denseSols;
   std::vector<double> objval;
public:
   void clear() {
      for (int i = denseSols.size() - 1; i >= 0; --i)
	 delete[] denseSols[i];
      denseSols.clear();
      objval.clear();
   }
   bool addSol(const double* sol, double val) {
      const int size = solver->getNumCols();
      for (int i = denseSols.size() - 1; i >= 0; --i) {
	 if (fabs(objval[i] - val) > 1e-3)
	    continue;
	 if (std::equal(sol, sol + size, denseSols[i])) {
	    delete[] sol;
	    return false;
	 }
      }
      denseSols.push_back(sol);
      objval.push_back(val);
      return true;
   }
   int numSolutions() {
      return denseSols.size();
   }
   const double* solution(int i) {
      return denseSols[i];
   }

   UserData() : subProblem(0), solver(0), cutMode(0) {}
   ~UserData() { clear(); }
};

UserData* initializeUserData(CSP_subProblem* sp, OsiClpSolverInterface* solver,
			     int baseRows);

#endif
