// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_SOLVER
#define _BCP_LP_SOLVER

#include "BCP_vector.hpp"
#include "BCP_matrix.hpp"

/** LP termination codes */
enum BCP_termcode{
   /** */
   BCP_NotExecuted  = -2,
   /** */
   BCP_Abandoned    = -1,
   /** */
   BCP_Optimal      = 0,
   /** */
   BCP_PrimalInf    = 1,
   /** */
   BCP_PrimalUnb    = 2,
   /** */
   BCP_Itlim        = 3,
   /** */
   BCP_DualObjlim   = 4,
   /** */
   BCP_PrimalApproxDualFeas = 5
};

/** LP feasibility */
enum BCP_lp_feasibility{
   /** */
   BCP_PrimalFeasible   = 0x00,
   /** */
   BCP_DualFeasible     = 0x00,
   /** */
   BCP_PrimalInfeasible = 0x01,
   /** */
   BCP_PrimalUnknown    = 0x02,
   /** */
   BCP_DualInfeasible   = 0x04,
   /** */
   BCP_DualUnknown      = 0x08,
   /** */
   BCP_PrimalFeasible_DualFeasible = BCP_PrimalFeasible | BCP_DualFeasible,
   /** */
   BCP_PrimalFeasible_DualInfeasible = BCP_PrimalFeasible | BCP_DualInfeasible,
   /** */
   BCP_PrimalInfeasible_DualFeasible = BCP_PrimalInfeasible | BCP_DualFeasible,
   /** */
   BCP_PrimalInfeasible_DualInfeasible = BCP_PrimalInfeasible | BCP_DualInfeasible,
   /** */
   BCP_PrimalFeasible_DualUnknown = BCP_PrimalFeasible | BCP_DualUnknown,
   /** */
   BCP_PrimalInfeasible_DualUnknown = BCP_PrimalInfeasible | BCP_DualUnknown,
   /** */
   BCP_PrimalUnknown_DualFeasible = BCP_PrimalUnknown | BCP_DualFeasible,
   /** */
   BCP_PrimalUnknown_DualInfeasible = BCP_PrimalUnknown | BCP_DualInfeasible,
   /** */
   BCP_PrimalUnknown_DualUnknown = BCP_PrimalUnknown | BCP_DualUnknown
};

//-----------------------------------------------------------------------------

class BCP_row;
class BCP_row_set;
class BCP_col;
class BCP_col_set;
class BCP_buffer;
class BCP_warmstart;

/** NO DESC IN OLD DOC

    All members are virtual... */

class BCP_lp_solver {
public:
   /**@name Destructor */
   /*@{*/
   /** */
   virtual ~BCP_lp_solver() {}
   /*@}*/

   /**@name Tolerances */
   /*@{*/
   /** Return primal infeasibility tolerance. */
   virtual double primalTolerance() const = 0;
   /** Return dual infeasibility tolerance. */
   virtual double dualTolerance() const = 0;
   /*@}*/

   /**@name Infinity */
   /*@{*/
   /** */
   virtual double infinity() const = 0;
   /*@}*/

   /**@name Load, unload (and resize??) */
   /*@{*/
   // load the description of an LP into the lp solver.
   // *****
   // ?? The lp solver must copy out the data!!!
   // LpMatrix might be empty !!! in that case just reserve the space.
   // *****
   /** */
   virtual void load_lp(const BCP_lp_relax& LpMatrix, // the problem description
                        const int Scaling,          // how to scale
			// non-0 : likely sizes
			const int MaxRowNum=0, const int MaxColNum=0,
			const int Maxnonzeros=0) = 0;
   // forget the problem description. Need not necessarily free INTERNAL
   // space; that can be done in the destructor. However, GET RID of LpMatrix.
   /** */
   virtual void unload_lp() = 0;
//    virtual void resize_lp(const int MaxRowNum, const int MaxColNum,
// 			  const int Maxnonzeros) = 0;
   /*@}*/

   /**@name Optimize */
   /*@{*/
   // optimize the problem
   /** */
   virtual void optimize(const bool need_exact_opt) = 0;
   /** */
   virtual void refactor_at_termination(const bool ifRefactor) = 0;
   /** */
   virtual bool refactor_at_termination() const = 0;
   /*@}*/

   /**@name Warmstarting */
   // functions related to warmstarting
   /*@{*/
   /** */
   virtual void set_warmstart(const BCP_warmstart* warmstart,
			      const int bvarnum, const int bcutnum) = 0;
   /** */
   virtual BCP_warmstart* get_warmstart(const int bvarnum,
					const int bcutnum) = 0;
   /*@}*/

   /**@name Get the solution */
   /*@{*/
   // what is the lp feasibility status after solving the last lp problem
   /** */
   virtual BCP_lp_feasibility get_lp_feasibility() = 0;

   // get the appropriate vector/value
   // -------------------------------------------------------------------------
   // *** NOTE ***
   // ?? If a value is within the appropriate 0-tolerance then set it to 0.0
   // -------------------------------------------------------------------------
   /** */
   virtual double           lower_bound() = 0; // lb on the objval

   /** */
   virtual BCP_termcode     termcode() = 0;
   /** */
   virtual int              iternum() = 0;
   /** */
   virtual double           objval() = 0;
   /** */
   virtual BCP_vec<double>* x() = 0;
   /** */
   virtual BCP_vec<double>* pi() = 0;
   /** */
   virtual BCP_vec<double>* dj() = 0;
   /** */
   virtual BCP_vec<double>* slack() = 0;

   /** */
   virtual BCP_termcode     termcode_approx() { return BCP_NotExecuted; }
   /** */
   virtual int              iternum_approx() { return 0; }
   /** */
   virtual double           objval_approx() { return 1e-31; }
   /** */
   virtual BCP_vec<double>* x_approx() { return 0; }
   /** */
   virtual BCP_vec<double>* pi_approx() { return 0; }
   /** */
   virtual BCP_vec<double>* dj_approx() { return 0; }
   /** */
   virtual BCP_vec<double>* slack_approx() { return 0; }
   /*@}*/

   /**@name Add and delete columns and rows */
   /*@{*/
   // problem modifying functions. throw an exception if doesn't fit!
   /** */
   virtual void add_cols(const BCP_vec<BCP_col*>& Cols) = 0;
   /** */
   virtual void add_cols(const BCP_col_set& Cols) = 0;
   /** */
   virtual void del_cols(const BCP_vec<int>& ColIndices) = 0;

   /** */
   virtual void add_rows(const BCP_vec<BCP_row*>& Rows) = 0;
   /** */
   virtual void add_rows(const BCP_row_set& Rows) = 0;
   /** */
   virtual void del_rows(const BCP_vec<int>& RowIndices) = 0;
   /*@}*/

   /**@name Mark and reset free rows */
   /*@{*/
   /** Mark the rows free for speed, but don't delete them from the matrix. */
   virtual void free_rows(BCP_vec<int>::const_iterator FirstRowInd,
			  BCP_vec<int>::const_iterator LastRowInd) = 0;
   /** Reset the rows to be constrained as they were before making them free.
    */ 
   virtual void constrain_rows(BCP_vec<int>::const_iterator FirstRowInd,
			       BCP_vec<int>::const_iterator LastRowInd) = 0;
   /*@}*/

   /**@name Change bounds */
   /*@{*/
   // in the next four functions Indices contains the col(row) indices of the
   // cols(rows) whose bounds must be set/get, and in/into
   // [FirstBound,LastBound) the lb/ub pairs are listed/written one after the
   // other.

   /** */
   virtual void set_col_set_bd(BCP_vec<int>::const_iterator FirstIndex,
			       BCP_vec<int>::const_iterator LastIndex,
			       BCP_vec<double>::const_iterator FirstBound,
			       BCP_vec<double>::const_iterator LastBound) = 0;
   /** */
   virtual void set_row_set_bd(BCP_vec<int>::const_iterator FirstIndex,
			       BCP_vec<int>::const_iterator LastIndex,
			       BCP_vec<double>::const_iterator FirstBound,
			       BCP_vec<double>::const_iterator LastBound) = 0;
   /** */
   virtual void get_col_set_bd(BCP_vec<int>::const_iterator FirstIndex,
			       BCP_vec<int>::const_iterator LastIndex,
			       BCP_vec<double>::iterator FirstBound,
			       BCP_vec<double>::iterator LastBound) = 0;
   /** */
   virtual void get_row_set_bd(BCP_vec<int>::const_iterator FirstIndex,
			       BCP_vec<int>::const_iterator LastIndex,
			       BCP_vec<double>::iterator FirstBound,
			       BCP_vec<double>::iterator LastBound) = 0;
   /*@}*/

   /**@name Fix variables to zero */
   /*@{*/
   /** */
   virtual void fix_vars_to_zero(BCP_vec<int>::const_iterator FirstColInd,
				 BCP_vec<int>::const_iterator LastColInd) = 0;
   /*@}*/

   /**@name Objective limit set/get */
   /*@{*/
   /** Set the upper limit on the objective value. */
   virtual void obj_upper_limit(const double ObjectiveUpperLimit) = 0;
   /** Get an upper limit on the objective value. */
   virtual double obj_upper_limit() = 0;
   /*@}*/

   /**@name Strong branching */
   /*@{*/
   /** Prepare for strong branching. The matrix is guaranteed not to change
       between <code>prepare_for_sb</code> and <code>finished_sb</code>.
       (Bounds and freeness can all change.) */
   virtual void prepare_for_sb(const int IterationLimit) = 0;
   /** */
   virtual void optimize_in_sb(const bool need_exact_opt) = 0;
   /** */
   virtual void finished_sb() = 0;
   /*@}*/

   /**@name Reduced cost */
   /*@{*/
   /** Compute the reduced cost of a <code>BCP_col</code>. */
   virtual double reduced_cost(const BCP_col& Col) = 0;
   /** Return the objective coefficient of the i-th variable. */
   /*@}*/
   virtual double cost(const int i) const = 0;

   /**@name Restore feasibility */
   /*@{*/
   // functions related to fixing infeasibility
   /** */
   virtual void prepare_for_feasibility_restoration() = 0;
   /** */
   virtual bool check_feasibility_restoration(const BCP_col& Col) = 0;
   /** */
   virtual void finished_feasibility_restoration() = 0;
   /*@}*/

   /**@name Dump matrix */
   /*@{*/
   /** Save the problem (or problem matrix only?) into a file in ??? format. */
   virtual void dump_matrix(const char * filename) = 0;
   /*@}*/
};

#endif
