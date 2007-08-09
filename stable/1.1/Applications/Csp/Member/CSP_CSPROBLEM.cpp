// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <iostream>
#include "CoinTime.hpp"
#include "CSP.hpp"

#define CSP_VERBOSE_INPUT 0

// what are you doing here? DOn't look at this.
// But...replace this with the reading in the data for CSP


CSPROBLEM::CSPROBLEM() :
  l_(0), demand_(0), w_(0), m_(0), s_(0), start_time_(0)
{
}

CSPROBLEM::CSPROBLEM(std::istream &is) :
  l_(0), demand_(0), w_(0), m_(0), s_(0), start_time_(0)
{

  is >> l_ >> s_ >> m_ ;
  
  //check that m >= 0)

  demand_=new int[m_];
  w_=new int[m_];
  for (int i=0; i<m_; ++i){
    is >> demand_[i] >> w_[i] ;
  }
  
   start_time_ = CoinCpuTime();
   
}

//#############################################################################

double
PATTERN::dj(const double* pi) const
{
   const double* elem = widths_.getElements();
   const int* ind = widths_.getIndices();
   double rc = 0;
   for (int i = widths_.getSize() - 1; i >= 0; --i) {
      rc += pi[ind[i]] * elem[i];
   }
   return cost_ - rc;
}
