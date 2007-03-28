// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_VAR_H
#define _CSP_VAR_H

#include <cfloat>
#include "BCP_vector.hpp"
#include "BCP_var.hpp"
#include "BCP_buffer.hpp"
#include "CSP.hpp"


//#############################################################################

// PATTERN defined in CSP.hpp. Csp_var uses multiple inheritance.
class CSP_var : public BCP_var_algo, public PATTERN {

public:
  //CSP: 
  // ToDo: make the ub a computed upper bound "on the fly"
  //       based on demand using existing lbs on other patterns
  // This has to go in two places
  //  (1) when we start to work on a node (initialize_new_search_tree_node)
  //  (2) when a new var is created,  vars_to_cols routine where you specify
  //      new upper bound for created var

  // Need         (i)  default constructor,
  //              (ii) a "real" constructor
  //              (iii)copy constructor,
  // (We won't have a constructor that uses a buffer reference.)
  
  // Default constructor.
  // Note: Constructor for BCP_var_algo has 3 arguments:
  //                                  BCP_IntegerVar, objCoef, lb, ub)  
  CSP_var() : BCP_var_algo(BCP_IntegerVar, 0, 0.0, DBL_MAX), PATTERN() {}
  
  // Constructor
   CSP_var(const PATTERN& pat) :
      BCP_var_algo(BCP_IntegerVar, pat.getCost(), 0.0, pat.getUb()),
      PATTERN(pat) {}

  // Copy constructor
   CSP_var(const CSP_var& x) :
      BCP_var_algo(BCP_IntegerVar, x.getCost(), x.lb(), x.ub()),
      PATTERN(x) {}

  // Destructor
  // Does the destructor of the CSP_var automatically call the destructor
  // of the PATTERN? Yes.
   ~CSP_var() {}

};

//#############################################################################

// If there were only one type of var, then the pack and unpack 
// could be member functions.  In the auction code, there were 
// two types of vars, so the pack/unpack were chosen to be written
// here so the caller could tell by the function name what was being
// unpacked.  This design is a little cleaner than having some 
// sort of indicator in the first unpacked bit, or some other scheme.
// Because we are brave and think we may expand the CSP in this 
// direction, we will do likewise.

// when "static" not used, the linkers got upset becuase including
// this file multiple places led to the function being 
// defined multiple times. 
// One solution is to put it in the .cpp file, but 
// we'd like to inline this little function, so we
// used the static keyword to work around the linker issue. 

static inline BCP_var_algo* CSP_var_unpack(BCP_buffer& buf){
  PATTERN pat(buf);
  return  new CSP_var(pat);
}

  // a CSP_var IS A pattern
static inline void CSP_var_pack(const BCP_var_algo* var, BCP_buffer& buf){
  dynamic_cast<const CSP_var *>(var)->pack(buf);
}

#endif
