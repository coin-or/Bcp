// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include <CoinHelperFunctions.hpp>
#include <CoinShallowPackedVector.hpp>

#include <OsiClpSolverInterface.hpp>

#ifdef CSP_UseCpx
#include <OsiCpxSolverInterface.hpp>
#endif

#ifdef CSP_UseSbb
#include <SbbModel.hpp>
#endif

#include <CglOddHole.hpp>
#include <CglGomory.hpp>
#include <CglKnapsackCover.hpp>

#include "CSP_colgen.hpp"
#include "CSP_userexits.hpp"
#include "KS.hpp"

using std::min;
using std::max;

void
CSP_colgen::setCsp(const CSPROBLEM* a, double perturb_factor, int perturb_num)
{
   int i, j, k;
   gutsOfDestructor();

   csproblem_ = a;

  // put an empty CSP_subProblem on the vector of subproblems
   // doing a pushback involves copying if it needs to reallocation.
   // therefore it requires an assignment operator..
   // but we really don't want to copies,
   // so we'll switch to pointers
   subProblems.push_back(new CSP_subProblem());

   // get the first subproblem..and the only one for now
   CSP_subProblem& sp = *subProblems[0];

   sp.perturb_factor = perturb_factor;
   sp.perturb_num = perturb_num;

   // create binary representation, where m is the number of items
   // and w_i is the width of the i-th item
   const int m = csproblem_->getM();
   const int knives_ = csproblem_->getS();
   sp.numBits = new int[m];
   sp.numCols = 0;

   // Ugly math calculates the max number of bits, b_i, needed to 
   // represent a_i in binary -- formula courtesy of JL.
   // Let l   be the stock roll length
   //     w_i be the demanded width
   // then 
   //     b_i= ceil(log_2(1+floor(l/w_i)))
   // However, we were too lazy to look up how to do log base 2
   // in C/C++, so we resorted to fact that 
   //     log_a(x) = log_b(x)/log_b(a)
   // and came up with
   //     b_i= ceil(log(1+floor(l/w_i))/log(2))

   const int l_ = csproblem_->getL();
   const int* w_ = csproblem_->getW();

   for (i = 0; i < m; ++i) {
      // l_/w[i] is an int! No need to take floor()
     sp.numBits[i]= (int)ceil(log(1+l_/w_[i])/log(2)) ;
     sp.numCols += sp.numBits[i];
   }
   sp.maxNumBits = * std::max_element(sp.numBits, sp.numBits + m);
 
   if (maxCols_ < sp.numCols) maxCols_=sp.numCols;
   if (maxNumBits_ < sp.maxNumBits) maxNumBits_ = sp.maxNumBits;

   OsiClpSolverInterface& solver=sp.solver;

   // fill out lower and upper bounds of the columns
   // for the 0/1 subproblem
   sp.lowerBounds = new double[sp.numCols];
   sp.upperBounds = new double[sp.numCols];
   CoinFill(sp.lowerBounds,sp.lowerBounds+sp.numCols,0.0);
   CoinFill(sp.upperBounds,sp.upperBounds+sp.numCols,1.0);

   // assume s <= -1 if no knife constraint is wanted
   if (knives_>-1){
     sp.nBaseRows=2;
   }
   else{
     sp.nBaseRows=1;
   }

   sp.csproblem=csproblem_;

   if (knives_ <= -1) {
     // Fill out "matrix", which has to be column major ordered
     // The base matrix consists of one row
     int * starts = new int[sp.numCols+1];
     int * indices =  new int[sp.numCols];
     double * elements = new double[sp.numCols];
     CoinIotaN(starts, sp.numCols+1,0);
     CoinFillN(indices, sp.numCols, 0);
     
     int colIndexCtr = 0;
     for (j = 0; j < m; ++j) {
       for (k=0; k<sp.numBits[j];++k){
	  elements[colIndexCtr++] = w_[j]*(1<<k);
       }
     }
     
     double * obj = new double [sp.numCols];
     double * rowLowers = new double [1];
     double * rowUppers = new double [1];
     
     CoinFillN(obj, sp.numCols, solver.getInfinity());
     rowLowers[0]= -solver.getInfinity();
     rowUppers[0]= l_;
     
     solver.loadProblem(sp.numCols, sp.nBaseRows, 
			starts, indices, elements, 
			sp.lowerBounds, sp.upperBounds,obj, 
			rowLowers,rowUppers);

     delete [] starts;
     delete [] indices;
     delete [] elements;
     delete [] obj;
     delete [] rowLowers;
     delete [] rowUppers;

   } else { // we have 2 rows, the second is for the knives. 
      // Fill out "matrix", which has to be column major ordered
      // The base matrix consists of one row
      int * starts = new int[sp.numCols+1];
      int * indices =  new int[2*sp.numCols];
      double * elements = new double[2*sp.numCols];
   
      // every column has 2 entries
      for (i = 0; i< sp.numCols+1; ++i){
	 starts[i]=2*i;
      }

      // every column has one entry in the zero row 
      // and one entry in the 1st row
      for (i = 0; i< sp.numCols; i++){
	 indices[2*i]=0;
	 indices[2*i+1]=1;
      }
       
      int colIndexCtr = 0;
      for (j = 0; j < m; ++j) {
	 int twoPower = 1;
	 for (k=0; k<sp.numBits[j];++k){
	   int value = w_[j]*twoPower;
	   int knifevalue = (l_ - w_[j])*twoPower;
	   elements[2*colIndexCtr]=value;
	   elements[2*colIndexCtr+1]=knifevalue;
	   twoPower*=2;
	   colIndexCtr++;
	 }
      }
       
      double * obj = new double [sp.numCols];
      double * rowLowers = new double [2];
      double * rowUppers = new double [2];
      
      CoinFillN(obj, sp.numCols, solver.getInfinity());
      rowLowers[0]= -solver.getInfinity();
      rowLowers[1]= 0;
      rowUppers[0]= l_;
      rowUppers[1]= knives_ * l_;
       
      solver.loadProblem(sp.numCols, sp.nBaseRows, 
			 starts, indices, elements, 
			 sp.lowerBounds, sp.upperBounds,obj, 
			 rowLowers,rowUppers);

      delete [] starts;
      delete [] indices;
      delete [] elements;
      delete [] obj;
      delete [] rowLowers;
      delete [] rowUppers;
   }

   int* integerCols = new int[sp.numCols];
   CoinIotaN(integerCols, sp.numCols, 0);
   solver.setInteger(integerCols, sp.numCols);
   delete[] integerCols;

   sp.user = initializeUserData(&sp, &solver, sp.nBaseRows);
}
	 
void
CSP_colgen::applyExclusions(const std::vector<const PATTERN*> &excl_patterns)
{
   // C++: preincrement is faster than post increment becuase 
   // doesn't require creating a temp var

   int  k, p;
   int numItems = csproblem_->getM();
   const int l_ = csproblem_->getL();
   const int* w_ = csproblem_->getW();
   const int knife_ = csproblem_->getS();

   // Expand each excluded pattern and count the ones in them
   int ** exclFullStorageElements = new int*[excl_patterns.size()];
   int * exclNumOnes = new int[excl_patterns.size()];
   for (size_t j=0; j<excl_patterns.size(); ++j){
      exclFullStorageElements[j] = new int[numItems];
      int* fullStorageElements = exclFullStorageElements[j];
      const CSP_packedVector& exPatWidths = excl_patterns[j]->getWidths();
      const double* elems = exPatWidths.getElements();
      const int* inds = exPatWidths.getIndices();
      CoinFillN(fullStorageElements, numItems, 0);
      int numOnes = 0;
      for (k = exPatWidths.getSize() - 1; k >= 0; --k) {
	 int elem = static_cast<int>(elems[k]);
	 fullStorageElements[inds[k]] = elem;
	 while (elem) {
	    numOnes += (elem & 1);
	    elem >>= 1;
	 }
      }
      exclNumOnes[j] = numOnes;
   }

   int * binIndices = new int[maxCols_];
   double * binElements = new double[maxCols_];
   
   for (size_t i = 0; i < subProblems.size(); ++i) {
     CSP_subProblem& sp = *subProblems[i];
     OsiClpSolverInterface & si = sp.solver;

     // delete the extra rows that might be there
     // put together row indices we want to delete and then delete 'em.
     int numRowsToBeDeleted = si.getNumRows()-sp.nBaseRows;
     int * rowIndicesToBeDeleted = new int[numRowsToBeDeleted];
     CoinIotaN(rowIndicesToBeDeleted, numRowsToBeDeleted, sp.nBaseRows);
     si.deleteRows(numRowsToBeDeleted, rowIndicesToBeDeleted);
     delete [] rowIndicesToBeDeleted;

     // add the exclusions constraints for the pattern to be excluded by
     // (1) expanding the compressed patterns to be excluded
     //      into full-storage patterns and then 
     // (2) converting the full-storage patterns into binary and counting
     //     the cardinality of hte set of 1's for the rhs of the exclusion 
     //     constraint.
     
     CoinIotaN(binIndices, maxCols_, 0);
     for (size_t j=0; j<excl_patterns.size(); ++j){
	// (1) 
	int usage = 0;
	int knifeUsage = 0;
	const CSP_packedVector& exPatWidths = excl_patterns[j]->getWidths();
	const double* elems = exPatWidths.getElements();
	const int* inds = exPatWidths.getIndices();
	CoinFillN(binElements, maxCols_, 0.0);
	for (k=0; k<exPatWidths.getSize(); ++k){
	   const int elem = static_cast<int>(elems[k]);
	   // used below...
	   usage += elem * w_[inds[k]];
	   // may be used below...
	   knifeUsage += elem * (l_- w_[inds[k]]);
	}
	// check if pattern is maximal - could be a parameter
	// difference =
	//    length of stock roll - 
	//    sum of (widths chosen in pattern * number selected of that width)
	// if difference < smallest widths 
	// then maximal, don't set zero bits below
	// or better yet...
	// if difference < widths * 2**l (integers stored as floats)
	// tighten coef
	 
	const int diff = l_ - usage;
	int knifeDiff = l_*knife_ - knifeUsage; 
	 
	// (2) 
	const int* fullStorageElements = exclFullStorageElements[j];
	int colIndexCtr = 0;
	int tcnt = 0;
	for (p = 0; p < numItems; ++p) {
	   // If no knive constraint then set knifeDiff to be big, so the test
	   // on knifeDiff in the loop is always true
	   if (knife_ < 0)
	      knifeDiff = l_ * (1 << sp.numBits[p]);
	   // convert fullElements[p] to binary
	   for (k=0;k<sp.numBits[p]; ++k){
	      if (fullStorageElements[p]&(1<<k)){
		 // the bit is one
		 binElements[colIndexCtr] = 1;
	      } else {
		 // if the diff is at least as big as this product,
		 // we can't tighen based on the knapsack, so set
		 // the coeff in the excl constaint.
		 // otherwise, we know based on the knapsack we
		 // can tighten, so don't set the coeff on the 
		 // excl constraint (0>-1). 
		 if ((diff >= w_[p]*(1<<k)) &&
		     (knifeDiff >= (l_-w_[p])*(1<<k)) ){
		    // the bit is zero
		    binElements[colIndexCtr] = -1;
		 } else {
		    ++tcnt;
		 }
	      }
	      colIndexCtr++;
	   }
	}
	printf("Tightened %i coefficients for excluded pattern %i.\n", tcnt,j);
	// false says don't test for identical elements
	CoinShallowPackedVector v(colIndexCtr, binIndices, binElements, false);

	// future: compute upper bound somewhere and pass in
	si.addRow(v, -si.getInfinity(), exclNumOnes[j] - 1);
     }

     if (sp.csproblem->doesCombineExclusionConstraints()) {
	// Try to combine the exclusion constraints
	for (size_t j0 = 0; j0 < excl_patterns.size(); ++j0) {
	   const int* full_0 = exclFullStorageElements[j0];
	   for (size_t j1 = j0+1; j1 < excl_patterns.size(); ++j1) {
	      if (((exclNumOnes[j0] + exclNumOnes[j1]) & 1) == 0)
		 continue;
	      const int* full_1 = exclFullStorageElements[j1];
	      int colIndexCtr = 0;
	      int cnt = 0;
	      for (p = 0; p < numItems; ++p) {
		 const int elem_0 = full_0[p];
		 const int elem_1 = full_1[p];
		 for (k = 0; k < sp.numBits[p]; ++k, ++colIndexCtr){
		    if ((elem_0 & (1<<k)) == (elem_1 & (1<<k))) {
		       binIndices[cnt] = colIndexCtr;
		       binElements[cnt++] = (elem_0 & (1<<k)) ? 1 : -1;
		    }
		 }
	      }
	      CoinShallowPackedVector v(cnt, binIndices, binElements, false);
	      si.addRow(v, -si.getInfinity(),
			double((exclNumOnes[j0] + exclNumOnes[j1]) / 2));
	   }
	}
     }

     if (sp.csproblem->doesAddKnapsackMirConstraints()) {
	CoinIotaN(binIndices, maxCols_, 0);
	for (int b = 1; b < maxNumBits_; ++b) {
	   const double twopower = 1 << b; // same as pow(2,b)
	   const double oldrhs = l_;
	   const double rhs = floor(oldrhs/twopower);
	   const double f = oldrhs/twopower - rhs;
	   if (f < 1e-3)
	      continue;
	   const double mult = 1 / (1 - f);
	   int colIndexCtr = 0;
	   for (p = 0; p < numItems; ++p) {
	      const int last = min(b, sp.numBits[p]);
	      for (k = 0; k < last; ++k) {
		 const double val = w_[p] * (1 << k) / twopower;
		 binElements[colIndexCtr++] =
		    floor(val) + std::max(0.0, (val - floor(val) - f) * mult);
	      }
	      for (k = last; k < sp.numBits[p]; ++k){
		 binElements[colIndexCtr++] = w_[p] * (1 << k) / twopower;
	      }
	   }
	   CoinShallowPackedVector v(colIndexCtr, binIndices, binElements,
				     false);
	   si.addRow(v, -si.getInfinity(), rhs);
	}
     }
     
     const int knives = csproblem_->getS();
     if (knives > 0 && sp.csproblem->doesAddKnifeMirConstraints()) {
	CoinIotaN(binIndices, maxCols_, 0);
	for (int b = 1; b < maxNumBits_; ++b) {
	   const double twopower = 1 << b; // same as pow(2,b)
	   const double oldrhs = knives * l_;
	   const double rhs = floor(oldrhs/twopower);
	   const double f = oldrhs/twopower - rhs;
	   if (f < 1e-3)
	      continue;
	   const double mult = 1 / (1 - f);
	   int colIndexCtr = 0;
	   for (p = 0; p < numItems; ++p) {
	      const int last = min(b, sp.numBits[p]);
	      for (k = 0; k < last; ++k) {
		 const double val = (l_-w_[p]) * (1 << k) / twopower;
		 binElements[colIndexCtr++] =
		    floor(val) + std::max(0.0, (val - floor(val) - f) * mult);
	      }
	      for (k = last; k < sp.numBits[p]; ++k){
		 binElements[colIndexCtr++] = (l_-w_[p]) * (1 << k) / twopower;
	      }
	   }
	   CoinShallowPackedVector v(colIndexCtr, binIndices, binElements,
				     false);
	   si.addRow(v, -si.getInfinity(), rhs);
	}
     }
   }

   for (size_t j = 0; j < excl_patterns.size(); ++j) {
      delete [] exclFullStorageElements[j];
   }
   delete [] exclNumOnes;
   delete [] exclFullStorageElements;
   delete [] binIndices;
   delete [] binElements;
}

//#############################################################################

void
CSP_scaleP(UserData* user, double* p, const double mult, const int first)
{
   const int numCols = user->solver->getNumCols();
   for (int s = user->numSolutions()-1; s >= first; --s) {
      const double* sol = user->solution(s);
      for (int j = 0; j < numCols; ++j) {
	 if (sol[j] == 1.0) {
	    p[j] *= mult;
	 }
      }
   }
}

//#############################################################################
// solver_matrix is row major ordered!

bool
CSP_knapsack(CSP_subProblem& sp, const CoinPackedMatrix& solver_matrix,
	     const double* p, const double detol, const double lb)
{
   bool success = true;
   const OsiClpSolverInterface& solver = sp.solver;
   const int numCols = solver.getNumCols();
   const int numRows = solver.getNumRows();

   Knapsack ks(numCols, solver.getRowUpper()[0], p,
	       solver_matrix.getVector(0).getElements());
   ks.optimize(lb, detol);
   if (ks.getBestVal() > 0) {
      const int* sol = ks.getBestSol();
      double* lhs = new double[numRows];
      double* dsol = new double[numCols];
      std::copy(sol, sol + numCols, dsol);
      solver_matrix.times(dsol, lhs);
      const double* rlb = solver.getRowLower();
      const double* rub = solver.getRowUpper();
      int j;
      for (j = 0; j < numRows; ++j) {
	 if (rlb[j] - 1e-7 > lhs[j] || rub[j] + 1e-7 < lhs[j])
	    break;
      }
      if (j == numRows) {
	 // it's feasible! we can return it
	 sp.user->addSol(dsol, ks.getBestVal());
      } else {
	 success = false;
      }
      delete[] lhs;
   }
   return success;
}

//#############################################################################

// go thru subproblems one by one
// set up obj func info
// call b&b

std::vector<PATTERN*>
CSP_colgen::generateColumns(const double* pi, const double detol,
			    const bool feasible)
{
   int j, k;
   std::vector<PATTERN*> pats;
   const int numItems = csproblem_->getM();
   const int * demand = csproblem_->getDemand();

   // allocate arrays to store orig column bounds
   // so they can be reset after each b&b call
   double * clbs = new double[maxCols_];
   double * cubs = new double[maxCols_];

   for (size_t i = 0; i < subProblems.size(); ++i) {
      CSP_subProblem& subProb = *subProblems[i];
      OsiClpSolverInterface& solver = subProb.solver;
      int colIndexCtr = 0;
      for (j = 0; j < numItems; ++j) {
	for (k=0; k<subProb.numBits[j];++k){
	   solver.setObjCoeff(colIndexCtr++, -pi[j]*(1<<k));
	}
      }
      // Make a copy of column lower and upper bounds 
      int numCols = solver.getNumCols();
      CoinCopyN(solver.getColLower(), numCols, clbs);
      CoinCopyN(solver.getColUpper(), numCols, cubs);

      const CoinPackedMatrix& solver_matrix = *solver.getMatrixByRow();

      subProb.user->clear();
      // Firt solve a relaxation of the subproblem, that is, just with the
      // knapsack constraint, and check whether the opt solution violates any
      // of the other constraints.
      double* p = new double[numCols];
      std::transform(solver.getObjCoefficients(),
		     solver.getObjCoefficients() + numCols,
		     p, std::negate<double>());
      const bool ks_succeeded = CSP_knapsack(subProb, solver_matrix, p, detol,
					     (feasible ? 1.00001 : 0.00001));

      if (! ks_succeeded) {
	 // If we did not get a good solution the solve the IP
	 printf("CSP: Must solve IP problem in cut generation :-(.\n");
	 solver.messageHandler()->setLogLevel(0);
	 const double* sol = 0;
#ifdef CSP_UseSbb
	 SbbModel sbb(solver);
	 sbb.messageHandler()->setLogLevel(0);
	 // CglGomory cg_gomory;
	 // sbb.addCutGenerator(&cg_gomory);
	 CglKnapsackCover cg_knapsack;
	 sbb.addCutGenerator(&cg_knapsack);
	 // CglOddHole cg_oddhole;
	 // sbb.addCutGenerator(&cg_oddhole);
	 sbb.branchAndBound();
	 sol = sbb.bestSolution();
#endif
#ifdef CSP_UseCpx
	 OsiCpxSolverInterface cpx;
	 cpx.loadProblem(*solver.getMatrixByCol(),
			 solver.getColLower(), solver.getColUpper(),
			 solver.getObjCoefficients(),
			 solver.getRowLower(), solver.getRowUpper());
	 int* integerCols = new int[solver.getNumCols()];
	 CoinIotaN(integerCols, solver.getNumCols(), 0);
	 cpx.setInteger(integerCols, solver.getNumCols());
	 delete[] integerCols;
	 cpx.branchAndBound();
	 sol = cpx.getColSolution();
#endif
	 if (sol == 0) {
	    continue;
	 }
	 double* dsol = new double[numCols];
	 std::copy(sol, sol + numCols, dsol);
	 subProb.user->addSol(dsol, solver.getObjValue());
	 const double redcost = 1 + solver.getObjValue();
	 // if (no attractive new columns)
	 if (redcost > -1e-8){ 
	    continue;
	 }
      }

      // Now do the knapsack heuristics with perturbed duals a couple of times
      if (feasible && subProb.perturb_num > 0) {
	 const double mult = subProb.perturb_factor;
	 CSP_scaleP(subProb.user, p, mult, 0);
	 for (k = 0; k < subProb.perturb_num; ++k) {
	    const int numsol = subProb.user->numSolutions();
	    CSP_knapsack(subProb, solver_matrix, p, detol, -DBL_MAX);
	    if (numsol < subProb.user->numSolutions()) {
	       // scale just the new guy
	       CSP_scaleP(subProb.user, p, mult,
			  subProb.user->numSolutions()-1);
	    } else {
	       // scale everyone again
	       CSP_scaleP(subProb.user, p, mult, 0);
	    }
	 }
      }

      delete[] p;
      // ekk_free(v.index);
      // ekk_free(v.element);

      for (int s = subProb.user->numSolutions()-1; s >= 0; --s) {
	 // sol has  numCols elements (i.e.,full storage)
	 const double* sol = subProb.user->solution(s);
      
	 // construct a new pattern based on the col solution
	 // numItems = m
	 int * newWidthIndices = new int[numItems];
	 double * newWidthElements = new double[numItems];
      
	 colIndexCtr = 0;
	 int newNumElements = 0;
	 double ub = 0;
	 for (j = 0; j < numItems; ++j) {
	    int patEntry = 0;
	    for (k=0; k<subProb.numBits[j];++k){
	       if(sol[colIndexCtr++] > 1-1e-5){
		  patEntry += (1<<k);
	       }
	    }
	    if (patEntry > 0) {
	       newWidthIndices[newNumElements]=j;
	       newWidthElements[newNumElements++]=patEntry;
	       const double component_ub = ceil(demand[j]/patEntry);
	       if (component_ub > ub)
		  ub = component_ub;
	    }
	 }

	 PATTERN* newPat = new PATTERN(newNumElements, newWidthIndices, 
				       newWidthElements, ub);
	 if (newPat->dj(pi) > - detol) {
	    delete newPat;
	 } else {
	    pats.push_back(newPat);
	 }
	 // newWidthIndices & newWidthElements : newPat takes ownership
	 // sol is owned by the userData buried in model
      }
      printf("CSP: generated %i, discarded %i\n",
	     subProb.user->numSolutions(),
	     subProb.user->numSolutions() - pats.size());
   }
   
   delete [] clbs;
   delete [] cubs;
   return pats;
}
