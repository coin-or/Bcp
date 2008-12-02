// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_COLGEN_H
#define _CSP_COLGEN_H

#include <OsiClpSolverInterface.hpp>

#include "CSP.hpp"

class UserData;

// CSP_subProblem was a private class of the CSP_colgen class
class CSP_subProblem {
public:
   // The csproblem for which we'll generate columns
   const CSPROBLEM* csproblem;
   UserData* user;

   double perturb_factor;
   int perturb_num;
   
   // number of base rows
   int nBaseRows;
   
   // array of the number of bits needed per item 
   // only the colgen class needs to know this data
   // which is the same for every subproblem
   // and calculated in setCsp method
   int * numBits;
   
   // The largest entry in the numBits array
   int maxNumBits;
   
   // may want to have a pool of knapsack cover cuts in the future
   
   // Subproblem may only differ at this point by their bounds
   // We may expand in the future
   double* lowerBounds;
   double* upperBounds;
   
   // number of cols in all the subproblems
   int numCols;
   
   OsiClpSolverInterface solver;
   
   // Default constructor
   CSP_subProblem(){}
   
   // Destructor 
   ~CSP_subProblem(){
      delete [] lowerBounds;
      delete [] upperBounds;
      delete [] numBits;
   }
   
   // Copy Constructor
   // Declaring only effectively disables these methods so 
   // the code doesn't use the C++ default copy and assignment.
   CSP_subProblem(const CSP_subProblem&);
   
   // Assignment
   CSP_subProblem& operator=(const CSP_subProblem&);
};

//#############################################################################

class CSP_colgen {
private:

  //max number of columns in any of the subproblems
  int maxCols_;

  // max number of bits for any item in any subproblem
  int maxNumBits_;

  // to get the number use subProblems.size();
  std::vector<CSP_subProblem*> subProblems;
  

  // Whether the members set by the setXxx() methods will have to be deleted
  // upon destructing the object
  // hack to keep track of which objects need to get cleaned up on exit
  // CSP doesn't need  
  const bool ownSetMembers;

  // The cutting stock problem instance for which we'll generate columns
  const CSPROBLEM* csproblem_;
  
  // private methods
private:
  void resetColBounds(OsiSolverInterface& si, const int numCols, 
		      const double* colLBs, const double* colUBs){
    int i;
    for (i=0; i<numCols; ++i){
      si.setColLower(i,colLBs[i]);
      si.setColUpper(i,colUBs[i]);
    }
  }
  
  void gutsOfDestructor() {
    if (ownSetMembers) {
      delete csproblem_;
    }
    for(size_t i=0; i<subProblems.size(); ++i){
      delete subProblems[i];
    }
  }

  // public methods  
public:
  // constructor. 
  // (Ask LL: The bool variable is to indicate whether to 
  //  to delete pointers that are passed via a setXxx() method (?)
  CSP_colgen(const bool own) : maxCols_(0),
			       maxNumBits_(0),
			       subProblems(),
			       ownSetMembers(own){}

  ~CSP_colgen() {
    gutsOfDestructor();
  }
  
  // invoked once in the beginning of BCP
  // sets up 1-row matrix (knapsack)
  // binary expansion stuff
  // has to set up mapping - these binvars correspon to which item
  // setCSP - e.g., calculate space needed for binary expansions
  void setCsp(const CSPROBLEM* a, double perturb_factor, int perturb_num);
  
  // this routine applies the exclusion contraints to the knapsack problem
  // invoked exactly once for every searchtree node when we enter the node
  // adds extra constraints for exclusions
  void applyExclusions(const std::vector<const PATTERN*> &excl_patterns );
  
  // invoked every time you solve an lp relaxation
  std::vector<PATTERN*>
  generateColumns(const double* pi, const double detol, const bool feasible);
};

#endif
