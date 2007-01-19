// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MATRIX_H
#define _BCP_MATRIX_H

// This file is fully docified.

// #include <cmath>
// #include <cfloat>

#include "CoinPackedVector.hpp"
#include "CoinPackedMatrix.hpp"

#include "BCP_math.hpp"
#include "BCP_vector.hpp"

//#############################################################################

class BCP_buffer;

//#############################################################################

/** This class holds a column in a compressed form. That is, it is a packed
    vector with an objective coefficient, lower and upper bound. */

class BCP_col : public CoinPackedVector {
   
protected:
   /**@name Data members */
   /*@{*/
   /** The objective function coefficient corresponding to the column. */   
   double _Objective;
   /** The lower bound corresponding to the column. */   
   double _LowerBound;
   /** The upper bound corresponding to the column. */   
   double _UpperBound;
   /*@}*/
   //--------------------------------------------------------------------------

public:
   /**@name Query methods */
   /*@{*/
   /** Return the objective coefficient. */
   inline double Objective()  const { return _Objective; }
   /** Return the lower bound. */
   inline double LowerBound() const { return _LowerBound; }
   /** Return the upper bound. */
   inline double UpperBound() const { return _UpperBound; }
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name General modifying methods */
   /*@{*/
   /** Set the objective coefficient to the given value. */
   inline void Objective(const double obj) { _Objective = obj; }
   /** Set the lower bound to the given value. */
   inline void LowerBound(const double lb) { _LowerBound = lb; }
   /** Set the upper bound to the given value. */
   inline void UpperBound(const double ub) { _UpperBound = ub; }

   /** Assignment operator: copy over the contents of <code>x</code>. */
   BCP_col& operator=(const BCP_col& x) {
      CoinPackedVector::operator=(x);
      _Objective = x.Objective();
      _LowerBound = x.LowerBound();
      _UpperBound = x.UpperBound();
      return *this;
   }
   /** Set the objective coefficient, lower and upper bounds to the given
       values. Also invokes the assign method of the underlying packed vector.
   */ 
   inline void
   assign(const int size, int*& ElementIndices, double*& ElementValues,
	  const double Obj, const double LB, const double UB) {
      CoinPackedVector::assignVector(size, ElementIndices, ElementValues,
				    false /* no test for duplicate index */);
      _Objective = Obj;
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /** Copy the arguments into the appropriate data members. */
   inline void
   copy(const int size, const int* ElementIndices, const double* ElementValues,
	const double Obj, const double LB, const double UB) {
      CoinPackedVector::setVector(size, ElementIndices, ElementValues,
				 false /* no test for duplicate index */);
      _Objective = Obj;
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /** Same as the other <code>copy()</code> method, except that instead of
       using vectors the indices (values) are given in
       <code>[firstind,lastind)</code> (<code>[firstval,lastval)</code>). */
   inline void
   copy(BCP_vec<int>::const_iterator firstind,
	BCP_vec<int>::const_iterator lastind,
	BCP_vec<double>::const_iterator firstval,
	BCP_vec<double>::const_iterator lastval,
	const double Obj, const double LB, const double UB) {
      CoinPackedVector::setVector(lastind - firstind, firstind, firstval,
				 false /* no test for duplicate index */);
      _Objective = Obj;
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Constructors / Destructor */
   /*@{*/
   /** The default constructor creates an empty column with 0 as objective
       coefficient, 0.0 as lower and +infinity as upper bound. */
   BCP_col() : CoinPackedVector(false /* no test for duplicate index */),
	       _Objective(0), _LowerBound(0.0), _UpperBound(BCP_DBL_MAX) {}
   /** The copy constructor makes a copy of <code>x</code>. */
   BCP_col(const BCP_col& x) :
      CoinPackedVector(x), _Objective(x.Objective()),
      _LowerBound(x.LowerBound()), _UpperBound(x.UpperBound()) {}
   /** This constructor acts exactly like the <code>copy</code> method with
       the same argument list. */
   BCP_col(BCP_vec<int>::const_iterator firstind,
	   BCP_vec<int>::const_iterator lastind,
	   BCP_vec<double>::const_iterator firstval,
	   BCP_vec<double>::const_iterator lastval,
	   const double Obj, const double LB, const double UB) :
      CoinPackedVector(lastind - firstind, firstind, firstval,
		      false /* no test for duplicate index */),
      _Objective(Obj), _LowerBound(LB), _UpperBound(UB) {}
   /** This constructor acts exactly like the <code>assign</code> method with
       the same argument list. */ 
   BCP_col(const int size, int*& ElementIndices, double*& ElementValues,
	   const double Obj, const double LB, const double UB) :
      CoinPackedVector(), _Objective(Obj), _LowerBound(LB), _UpperBound(UB) {
      CoinPackedVector::assignVector(size, ElementIndices, ElementValues,
				    false /* no test for duplicate index */);
   }
   BCP_col(const CoinPackedVectorBase& vec,
	   const double Obj, const double LB, const double UB) :
      CoinPackedVector(vec.getNumElements(),
		      vec.getIndices(), vec.getElements()),
      _Objective(Obj), _LowerBound(LB), _UpperBound(UB) {}
   /** The destructor deletes all data members. */
   ~BCP_col() {}
   /*@}*/
};

//#############################################################################

/** This class holds a row in a compressed form. That is, it is a packed
    vector with a lower and upper bound. */

class BCP_row : public CoinPackedVector {
protected:
   /**@name Data members */
   /*@{*/
   /** The lower bound corresponding to the row. */   
   double _LowerBound;
   /** The upper bound corresponding to the row. */   
   double _UpperBound;
   /*@}*/
   //--------------------------------------------------------------------------

public:
   /**@name Query methods */
   /*@{*/
   /** Return the lower bound. */
   inline double LowerBound() const { return _LowerBound; }
   /** Return the upper bound. */
   inline double UpperBound() const { return _UpperBound; }
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name General modifying methods */
   /*@{*/
   /** Set the lower bound to the given value. */
   inline void LowerBound(double lb) { _LowerBound = lb; }
   /** Set the upper bound to the given value. */
   inline void UpperBound(double ub) { _UpperBound = ub; }

   /** Assignment operator: copy over the contents of <code>x</code>. */
   BCP_row& operator=(const BCP_row& x) {
      CoinPackedVector::operator=(x);
      _LowerBound = x.LowerBound();
      _UpperBound = x.UpperBound();
      return *this;
   }

   /** Set the lower and upper bounds to the given values. Also invokes the
       assign method of the underlying packed vector. */
   void
   assign(const int size, int*& ElementIndices, double*& ElementValues,
	  const double LB, const double UB) {
      CoinPackedVector::assignVector(size, ElementIndices, ElementValues,
				    false /* no test for duplicate index */);
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /** Copy the arguments into the appropriate data members. */
   void
   copy(const int size, const int* ElementIndices, const double* ElementValues,
	const double LB, const double UB) {
      CoinPackedVector::setVector(size, ElementIndices, ElementValues,
				 false /* no test for duplicate index */);
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /** Same as the other <code>copy()</code> method, except that instead of
       using vectors the indices (values) are given in
       <code>[firstind,lastind)</code> (<code>[firstval,lastval)</code>). */
   void
   copy(BCP_vec<int>::const_iterator firstind,
	BCP_vec<int>::const_iterator lastind,
	BCP_vec<double>::const_iterator firstval,
	BCP_vec<double>::const_iterator lastval,
	const double LB, const double UB) {
      CoinPackedVector::setVector(lastind - firstind, firstind, firstval,
				 false /* no test for duplicate index */);
      _LowerBound = LB;
      _UpperBound = UB;
   }
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Constructors / Destructor */
   /*@{*/
   /** The default constructor creates an empty row with -infinity as lower
       and +infinity as upper bound. */
   BCP_row() : CoinPackedVector(false /* no test for duplicate index */),
	       _LowerBound(-BCP_DBL_MAX), _UpperBound(BCP_DBL_MAX) {}
   /** The copy constructor makes a copy of <code>x</code>. */
   BCP_row(const BCP_row& x) :
      CoinPackedVector(x),
      _LowerBound(x.LowerBound()), _UpperBound(x.UpperBound()) {}
   /** This constructor acts exactly like the <code>copy</code> method with
       the same argument list. */
   BCP_row(BCP_vec<int>::const_iterator firstind,
	   BCP_vec<int>::const_iterator lastind,
	   BCP_vec<double>::const_iterator firstval,
	   BCP_vec<double>::const_iterator lastval,
	   const double LB, const double UB) :
      CoinPackedVector(lastind - firstind, firstind, firstval,
		      false /* no test for duplicate index */),
      _LowerBound(LB), _UpperBound(UB) {}
   /** This constructor acts exactly like the <code>assign</code> method with
       the same argument list. */ 
   BCP_row(const int size, int*& ElementIndices, double*& ElementValues,
	   const double LB, const double UB) :
      CoinPackedVector(), _LowerBound(LB), _UpperBound(UB) {
      CoinPackedVector::assignVector(size, ElementIndices, ElementValues,
				    false /* no test for duplicate index */);
   }
   BCP_row(const CoinPackedVectorBase& vec, const double LB, const double UB) :
      CoinPackedVector(vec.getNumElements(),
		      vec.getIndices(), vec.getElements()),
      _LowerBound(LB), _UpperBound(UB) {}
   /** The destructor deletes all data members. */
   ~BCP_row() {}
   /*@}*/
};

//#############################################################################
//#############################################################################

/** An object of type <code>BCP_lp_relax</code> holds the description of an lp
    relaxation. The matrix, lower/upper bounds on the variables and cuts and
    objective coefficients for the variables. */
class BCP_lp_relax : public CoinPackedMatrix {
private:
   /**@name Data members */
   /*@{*/
   /** The objective coefficients of the variables. */
   BCP_vec<double> _Objective;
   /** The lower bounds on the variables. */
   BCP_vec<double> _ColLowerBound;
   /** The upper bounds on the variables. */
   BCP_vec<double> _ColUpperBound;
   /** The lower bounds on the cuts. */
   BCP_vec<double> _RowLowerBound;
   /** The upper bounds on the cuts. */
   BCP_vec<double> _RowUpperBound;
   /*@}*/
   //--------------------------------------------------------------------------

public:
   /**@name Query methods */
   /*@{*/
   /** The number of columns. */
   inline size_t colnum() const       { return _ColLowerBound.size(); }
   /** The number of rows. */
   inline size_t rownum() const       { return _RowLowerBound.size(); }
   /** A const reference to the vector of objective coefficients. */
   inline const BCP_vec<double>& Objective()     const {return _Objective;}
   /** A const reference to the vector of lower bounds on the variables. */
   inline const BCP_vec<double>& ColLowerBound() const {return _ColLowerBound;}
   /** A const reference to the vector of upper bounds on the variables. */
   inline const BCP_vec<double>& ColUpperBound() const {return _ColUpperBound;}
   /** A const reference to the vector of lower bounds on the cuts. */
   inline const BCP_vec<double>& RowLowerBound() const {return _RowLowerBound;}
   /** A const reference to the vector of upper bounds on the cuts. */
   inline const BCP_vec<double>& RowUpperBound() const {return _RowUpperBound;}
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Methods modifying the whole LP relaxation. */
   /*@{*/
   /** Copy the content of <code>x</code> into the LP relaxation. */
   BCP_lp_relax& operator=(const BCP_lp_relax& mat);
   /** Reserve space in the LP relaxation for at least <code>MaxColNum</code>
       columns, <code>MaxRowNum</code> rows and <code>MaxNonzeros</code>
       nonzero entries. This is useful when columns/rows are added to the LP
       relaxation in small chunks and to avoid a series of reallocation we
       reserve sufficient space up front. */
   void reserve(const int MaxColNum, const int MaxRowNum,
		const int MaxNonzeros);
   /** Clear the LP relaxation. */
   void clear();
   /** Set up the LP relaxation by making a copy of the arguments */
   void copyOf(const CoinPackedMatrix& m,
	       const double* OBJ, const double* CLB, const double* CUB,
	       const double* RLB, const double* RUB);
   /** Set up the LP relaxation by taking over the pointers in the arguments */
   void assign(CoinPackedMatrix& m,
	       double*& OBJ, double*& CLB, double*& CUB,
	       double*& RLB, double*& RUB);
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Methods for expanding/shrinking the LP relaxation. */
   /*@{*/
#if 0
   /** Append the columns in the argument column set to the end of this LP
       relaxation. */
   void add_col_set(const BCP_col_set& Cols);
   /** Append the rows in the argument row set to the end of this LP
       relaxation. */
   void add_row_set(const BCP_row_set& Rows);
#endif
   /** Remove the columns whose indices are listed in <code>pos</code> from
       the LP relaxation. */
   void erase_col_set(const BCP_vec<int>& pos);
   /** Remove the rows whose indices are listed in <code>pos</code> from
       the LP relaxation. */
   void erase_row_set(const BCP_vec<int>& pos);
   /*@}*/
   //--------------------------------------------------------------------------
#if 0
   /**@name Dot product methods */
   /*@{*/
   /** Compute the dot product of the index-th column with the full vector
       given in <code>col</code>. */
   double dot_product_col(const int index, const BCP_vec<double>& col) const;
   /** Compute the dot product of the index-th row with the full vector
       given in <code>row</code>. */
   double dot_product_row(const int index, const BCP_vec<double>& row) const;
   /** Compute the dot product of the index-th column with the full vector
       starting at <code>col</code>. */
   double dot_product_col(const int index, const double* col) const;
   /** Compute the dot product of the index-th row with the full vector
       starting at <code>row</code>. */
   double dot_product_row(const int index, const double* row) const;
   /*@}*/
#endif
   //--------------------------------------------------------------------------

   /**@name Packing/unpacking */
   /*@{*/
   /** Pack the LP relaxation into the buffer. */
   void pack(BCP_buffer& buf) const;
   /** Unpack the LP relaxation from the buffer. */
   void unpack(BCP_buffer& buf);
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Constructors and destructor */
   /*@{*/
   /** Create an empty LP relaxation with given ordering. */
   BCP_lp_relax(const bool colordered = true) :
      CoinPackedMatrix(colordered, 0, 0, 0, NULL, NULL, NULL, NULL),
      _Objective(), _ColLowerBound(), _ColUpperBound(),
      _RowLowerBound(), _RowUpperBound() {}

   /** The copy constructor makes a copy of the argument LP relaxation. */
   BCP_lp_relax(const BCP_lp_relax& mat);

   /** Create a row major ordered LP relaxation by assigning the content of
       the arguments to the LP relaxation. When the constructor returns the
       content of 'rows' doesn't change while that of CLB, CUB and OBJ will be
       whatever the default constructor for those arguments create. */
   BCP_lp_relax(BCP_vec<BCP_row*>& rows,
		BCP_vec<double>& CLB, BCP_vec<double>& CUB,
		BCP_vec<double>& OBJ);

   /** Same as the previous method except that this method allows extra_gap
       and extra_major to be specified. For the description of those see the
       documentation of CoinPackedMatrix. (extra_gap will be used for adding
       columns, while extra major for adding rows) */
   BCP_lp_relax(BCP_vec<BCP_row*>& rows,
		BCP_vec<double>& CLB, BCP_vec<double>& CUB,
		BCP_vec<double>& OBJ,
		double extra_gap, double extra_major);

   /** Create a column major ordered LP relaxation by assigning the content of
       the arguments to the LP relaxation. When the constructor returns the
       content of 'cols' doesn't change while that of RLB and RUB will be
       whatever the default constructor for those arguments create. */
   BCP_lp_relax(BCP_vec<BCP_col*>& cols,
		BCP_vec<double>& RLB, BCP_vec<double>& RUB);

   /** Same as the previous method except that this method allows extra_gap
       and extra_major to be specified. For the description of those see the
       documentation of CoinPackedMatrix. (extra_gap will be used for adding
       rows, while extra major for adding columns) */
   BCP_lp_relax(BCP_vec<BCP_col*>& cols,
		BCP_vec<double>& RLB, BCP_vec<double>& RUB,
		double extra_gap, double extra_major);

   /** Create an LP relaxation of the given ordering by copying the content
       of the arguments to the LP relaxation. */
   BCP_lp_relax(const bool colordered,
		const BCP_vec<int>& VB, const BCP_vec<int>& EI,
		const BCP_vec<double>& EV, const BCP_vec<double>& OBJ,
		const BCP_vec<double>& CLB, const BCP_vec<double>& CUB,
		const BCP_vec<double>& RLB, const BCP_vec<double>& RUB);
   /** Create an LP relaxation of the given ordering by assigning the content
       of the arguments to the LP relaxation. When the constructor returns the
       content of the arguments will be NULL pointers. <br>
   */
   BCP_lp_relax(const bool colordered,
		const int rownum, const int colnum, const int nznum,
		int*& VB, int*& EI, double*& EV,
		double*& OBJ, double*& CLB, double*& CUB,
		double*& RLB, double*& RUB);

   /** The destructor deletes the data members */
   ~BCP_lp_relax() {}
   /*@}*/
private:
  /**@name helper functions for the constructors */
  /*@{*/
  ///
  void BCP_createColumnOrderedMatrix(BCP_vec<BCP_row*>& rows,
				     BCP_vec<double>& CLB,
				     BCP_vec<double>& CUB,
				     BCP_vec<double>& OBJ);
  ///
  void BCP_createRowOrderedMatrix(BCP_vec<BCP_col*>& cols,
				  BCP_vec<double>& RLB,
				  BCP_vec<double>& RUB);
  /*@}*/
};

#endif
