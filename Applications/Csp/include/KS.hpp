// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _KS_HPP
#define _KS_HPP

#include <utility>
#include <algorithm>
#include <cfloat>

using std::fill;

// A goto-less implementation of the Horowitz-Sahni exact solution 
// procedure for solving knapsack problem.
//
// Reference: Martello and Toth, Knapsack Problems, Wiley, 1990, p30-31.
//
// ToDo: Implement a dynamic programming approach for case
//       of knapsacks with integral coefficients
//-------------------------------------------------------------------

// The knapsack problem is to find:

// max {sum(j=1,n) p_j*x_j st. sum (j=1,n)w_j*x_j <= c, x binary}

// Notation:
//     xhat : current solution vector
//     zhat : current solution value = sum (j=1,n) p_j*xhat_j
//     chat : current residual capacity = c - sum (j=1,n) w_j*xhat_j
//     x    : best solution so far, n-vector.
//     z    : value of the best solution so far =  sum (j=1,n) p_j*x_j

// Input: n, the number of variables; 
//        c, the rhs;
//        p, n-vector of objective func. coefficients;
//        w, n-vector of the row coeff.

// Output: z, the optimal objective function value;
//         x, the optimal (binary) solution n-vector;

struct triplet {
   double c;
   double w;
   int i;
};

inline bool
ratioComp(const triplet& t0, const triplet& t1) {
   return (t0.c/t0.w > t1.c/t1.w ?
	   true : (t0.c/t0.w < t1.c/t1.w ? false : (t0.i > t1.i)));
}

class Knapsack {
private:
   inline void
   sortItems(const double* c, const double* w) {
      delete perm_;
      double* ww = new double[n_+1];
      double* cc = new double[n_+1];
      perm_ = new int[n_];
      triplet* items = new triplet[n_];
      for (int i = n_ - 1; i >= 0; --i) {
	 items[i].c = c[i];
	 items[i].w = w[i];
	 items[i].i = i;
      }
      std::sort(items, items+n_, ratioComp);
      for (int i = n_ - 1; i >= 0; --i) {
	 cc[i] = items[i].c;
	 ww[i] = items[i].w;
	 perm_[i] = items[i].i;
      }
      delete[] items;
      delete[] weight_;
      delete[] cost_;
      ww[n_] = DBL_MAX;
      cc[n_] = 0.0;
      weight_ = ww;
      cost_ = cc;
   }

   /** The capacity of the knapsack */
   double cap_;
   /** The number of items */
   int n_;
   /** The permutation corresponding to the ordering (element perm[i] is the
       i-th in the original order */
   int* perm_;
   /** The cost of each item (the order is after doing the cost/weight
       ordering) */
   const double* cost_;
   /** The weight of the items (the order is after doing the cost/weight
       ordering) */
   const double* weight_;

   /** The optimal solution in terms of the input order. */
   int* x;
   /** The optimal solution value */
   double z;

public:
   Knapsack(int n, double cap, const double* c, const double* w) :
      cap_(cap), n_(n), perm_(0), cost_(0), weight_(0), x(new int[n])
   {
      sortItems(c, w);
   }
   ~Knapsack() {
      delete[] perm_;
      delete[] x;
      delete[] weight_;
      delete[] cost_;
   }

   void setCosts(const double* c) {
      sortItems(c, weight_);
   }
   void setCapacity(double cap) { cap_ = cap; }

   /** Return true/false depending on whether the problem is feasible */
   void optimize(double lb = 0.0, double minimp = 1e-6);

   const int* getBestSol() const { return x; }
   const double getBestVal() const { return z; }
};

//#############################################################################

void
Knapsack::optimize(double lb, double minimp)
{
   z = lb;
   fill(x, x+n_, 0);
   int * xhat = new int[n_+1];
   fill(xhat, xhat+(n_+1), 0);
   int j;

   // set up: adding the extra element and
   // accounting for the FORTRAN vs C difference in indexing arrays.
   double * p = new double[n_+2];
   double * w = new double[n_+2];
   int ii;
   for (ii = 1; ii < n_+1; ii++){
      p[ii]=cost_[ii-1];
      w[ii]=weight_[ii-1];
   }

   // 1. initialize 
   double zhat = 0.0;
   z = 0.0;
   double caphat = cap_;
   p[n_+1] = 0.0;
   w[n_+1] = DBL_MAX;
   j = 1;

   while (true) {
      // 2. compute upper bound u
      // "find r = min {i: sum k=j,i w_k>chat};"
      ii = j;
      double wSemiSum = w[j];
      double pSemiSum = p[j];
      while (wSemiSum <= caphat && ii < n_+2){
	 ii++;
	 wSemiSum += w[ii];
	 pSemiSum += p[ii];
      }
      if (ii == n_+2) {
	 printf("Exceeded iterator limit. Aborting...\n");
	 abort();
      }
      // r = ii at this point 
      wSemiSum -= w[ii];
      pSemiSum -= p[ii];
      // *FIXME* : why floor
      // double u = pSemiSum + floor((caphat - wSemiSum)*p[ii]/w[ii]);
      double u = pSemiSum + (caphat - wSemiSum)*p[ii]/w[ii];
      
      // "if (z >= zhat + u) goto 5: backtrack;"
      if (z + minimp < zhat + u) {
	 do {
	    // 3. perform a forward step 
	    while (w[j] <= caphat){
	       caphat = caphat - w[j];
	       zhat = zhat + p[j];
	       xhat[j] = 1;
	       ++j;
	    }
	    if (j <= n_) {
	       xhat[j] = 0;
	       ++j;
	    }
	 } while (j == n_); 
	 
	 // "if (j<n) goto 2: compute_ub;"
	 if (j < n_)
	    continue;
      
	 // 4. up date the best solution so far 
	 if (zhat > z) {
	    z = zhat;
	    int k;
	    for (k = 0; k < n_; ++k){
	       x[perm_[k]] = xhat[k+1];
	    }
	 }
	 j = n_;
	 if (xhat[n_] == 1){
	    caphat = caphat + w[n_];
	    zhat = zhat - p[n_];
	    xhat[n_] = 0;
	 }
      }
      // 5. backtrack 
      // "find i=max{k<j:xhat[k]=1};"
      int i = j-1; 
      while (!(xhat[i]==1) && i>0) {
	 i--;
      }
      
      // "if (no such i exists) return;"
      if (i == 0) {
	 delete [] p;
	 delete [] w;
	 delete [] xhat;
	 break;
      }

      caphat = caphat + w[i];
      zhat = zhat - p[i];
      xhat[i] = 0;
      j = i+1;
      // "goto 2: compute_ub;"
   }
}

#endif
