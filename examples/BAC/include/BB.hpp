// Last edit: 2/10/05
//
// Name:     BB.hpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BB_H
#define _BB_H

class CoinPackedMatrix;
class BCP_buffer;

/** Class holding data for describing the problem */

class BB_prob {
private:
   BB_prob(const BB_prob&);
   BB_prob& operator=(const BB_prob&);

public:

  /// Numerical precision for double arithmetic
  double EPSILON; 

  /// Number of rows in the input file
  int rownum;

  /// Number of columns in the input file
  int colnum;

  /**@name Column informations */
  //@{
  /// Integrality information for structural variables
  bool* integer;    
  
  /// Lower bounds for structural variables
  double* clb;
  
  /// Upper bounds for stuctural variables
  double* cub;      

  /// Objective coefficients
  double* obj;      
  //@}

  /**@name Row informations */
  //@{
  /// Lower bounds for core constraints
  double* rlb_core;      
  
  /// Upper bounds for core constraints
  double* rub_core;      

  /// Lower bounds for indexed constraints
  double* rlb_indexed;   

  /// Upper bounds for indexed constraints
  double* rub_indexed;   
  
  /// Holds the coefficients of the core rows
  CoinPackedMatrix* core;
  
  // Holds the coeffcients of the indexed rows
  CoinPackedMatrix* indexed;
  //@}
  
public:

  /**@name Constructors and destructors */
  //@{
  /// Default constructor 
  BB_prob();

  /// Default destructor 
  ~BB_prob();
  //@}
};

#endif
