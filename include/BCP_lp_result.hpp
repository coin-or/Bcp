// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_RESULT_H
#define _BCP_LP_RESULT_H

// This file is fully docified.

#include <cfloat>

#include "BCP_error.hpp"
#include "BCP_vector.hpp"
#include "OsiSolverInterface.hpp"

/** LP termination codes. Each possible termination is a bit. */
enum BCP_termcode {
  /** */
  BCP_Abandoned           = 0x01,
  /** */
  BCP_ProvenOptimal       = 0x02,
  /** */
  BCP_ProvenPrimalInf     = 0x04,
  /** */
  BCP_ProvenDualInf       = 0x08,
  /** */
  BCP_PrimalObjLimReached = 0x10,
  /** */
  BCP_DualObjLimReached   = 0x20,
  /** */
  BCP_IterationLimit      = 0x40,
  /** */
  BCP_TimeLimit           = 0x80
};

/** This class holds the results after solving an LP relaxation. There may be
    an exact and/or an approximate solution. */

class BCP_lp_result {
private:
  /**@name Disabled methods */
  /*@{*/
    /** The copy constructor is declared but not defined to disable it. */
    BCP_lp_result(const BCP_lp_result&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_lp_result& operator=(const BCP_lp_result&);
  /*@}*/

private:
  /**@name Data members storing general information about the solver. */
  /*@{*/
    double _lower_bound;
    /** The zero-tolerance used by the LP solver for the primal solution. */
    double _primal_tolerance;
    /** The zero-tolerance used by the LP solver for the dual solution. */
    double _dual_tolerance;
    /** True if the primal and dual solution vectors, reduced costs and lhss
	are <em>owned</em> by this structure. Otherwise these vectors are
	members of some other structure, e.g., the LP solver. In the second
	case the data members that are pointers just point into the other
	structure. This is much faster, and most of the time sufficient. The
	flag is needed so that when deleting such an object we shouldn't delete
	these pointers. */
    bool   _private_arrays;
  /*@}*/

  /**@name Data members holding information about the LP solution. */
  /*@{*/
    /** The termination code of the algorithm. */
    BCP_termcode _termcode;
    /** The number of iterations the algorithm took (however the
        algorithm used interprets "iteration"). */
    int     _iternum;
    /** The solution value. Depending on whether the solver solves to
	optimality or just delivers an approximate solution this value is
	either the optimum or a lower bound on the optimum. */
    double  _objval;
    /** The primal solution. */
    double* _x;
    /** The dual solution. */
    double* _pi;
    /** The reduced costs. */
    double* _dj;
    /** The left hand sides. */
    double* _lhs;
  /*@}*/

public:
  /**@name Constructor and destructor */
  /*@{*/
    /** The default constructor initializes an empty solution, i.e., one 
        which holds neither an exact nor an approximate solution. */
    BCP_lp_result() :
      _lower_bound(-DBL_MAX),
      _primal_tolerance(0), _dual_tolerance(0), _private_arrays(false),
      _termcode(BCP_ProvenOptimal), _iternum(0), _objval(0),
      _x(0), _pi(0), _dj(0), _lhs(0)
    {}
    /** The destructor deletes the data members if they are private copies. */
    ~BCP_lp_result() {
      if (_private_arrays) {
	delete[] _x;
	delete[] _pi;
	delete[] _dj;
	delete[] _lhs;
      }
    }
  /*@}*/

  //--------------------------------------------------------------------------
  /**@name Query methods for the solution. These methods (except for
     the first) just return the value of the queried member (in case of the
     vector members a reference to the vector is returned instead of the
     pointer. */
  /*@{*/
    ///
    BCP_termcode  termcode() const { return _termcode; }
    ///
    int           iternum()  const { return _iternum; }
    ///		  
    double        objval()   const { return _objval; }
    ///
    const double* x()        const { return _x; }
    ///	  
    const double* pi()       const { return _pi; }
    ///	  
    const double* dj()       const { return _dj; }
    ///	  
    const double* lhs()      const { return _lhs; }
  /*@}*/

  /**@name Query methods for general solver information. */
  /*@{*/
    /** Return the primal tolerance of the solver. */
    double primalTolerance() const { return _primal_tolerance; }
    /** Return the dual tolerance of the solver. */
    double dualTolerance()   const { return _dual_tolerance; }
  /*@}*/

  /**@name Modifying methods */
  /*@{*/
    /** Get the result from the LP solver. Non-vector members will get their
	values from the LP solver. Vector members are copied over if
	<code>copy_out</code> is true, otherwise they will point to the
	corresponding vectors in the LP solver. <code>_private_arrays</code> is
	set to the same value as <code>copy_out</code>. */
    void get_results(OsiSolverInterface& lp_solver, bool copy_out = true);
    /** Set the lower bound and the exact and approximate objective values to
	the value given in the argument. */ 
    void fake_objective_value(const double val) {
      _objval = val;
    }
  /*@}*/
};

#endif
