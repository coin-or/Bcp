// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_POOL_H
#define _BCP_LP_POOL_H

#include "BCP_error.hpp"
#include "BCP_vector.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_matrix.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"

//#############################################################################

class BCP_lp_waiting_row {
private:
   BCP_lp_waiting_row(const BCP_lp_waiting_row&);
   BCP_lp_waiting_row& operator=(const BCP_lp_waiting_row&);
private:
   BCP_cut* _cut;
   BCP_row* _row;
   double _violation;
public:
   BCP_lp_waiting_row(BCP_cut* cut, BCP_row* row=0, double viol=-1) :
      _cut(cut), _row(row), _violation(viol) {}
   ~BCP_lp_waiting_row() {
      delete _row;
      delete _cut;
   }

   inline BCP_cut* cut() { return _cut; }
   inline BCP_row* row() { return _row; }
   inline const BCP_cut* cut() const { return _cut; }
   inline const BCP_row* row() const { return _row; }

   inline void clear_cut() { _cut = 0; }
   inline void delete_row() { delete _row;  _row = 0; _violation = -1;}
   inline void set_row(BCP_row*& row) { _row = row; row = 0; }

   inline double violation() const { return _violation; }
   inline void set_violation(double v) { _violation = v; }
   void compute_violation(const BCP_lp_result& lpres);
};

//-----------------------------------------------------------------------------

class BCP_lp_cut_pool : public BCP_vec<BCP_lp_waiting_row*> {
private:
   static bool _rows_are_valid;
   // disable the default copy constructor and assignment operator
   BCP_lp_cut_pool(const BCP_lp_cut_pool&);
   BCP_lp_cut_pool& operator=(const BCP_lp_cut_pool&);
public:
   BCP_lp_cut_pool() {}
   ~BCP_lp_cut_pool() { 
      purge_ptr_vector(dynamic_cast< BCP_vec<BCP_lp_waiting_row*>& >(*this)); 
   }

   inline bool rows_are_valid() const { return _rows_are_valid; }
   inline void rows_are_valid(bool status) { _rows_are_valid = status; }

   inline void compute_violations(const BCP_lp_result& lpres,
				  BCP_lp_cut_pool::iterator first, 
				  BCP_lp_cut_pool::iterator last) {
      if (! _rows_are_valid)
	 throw BCP_fatal_error("\
BCP_lp_cut_pool::compute_violations() : rows are not valid\n");
      while (first != last) {
	 (*first)->compute_violation(lpres);
	 ++first;
      }
   }
   int remove_nonviolated(const double etol);
};

//#############################################################################

class BCP_lp_waiting_col {
private:
   BCP_lp_waiting_col(const BCP_lp_waiting_col&);
   BCP_lp_waiting_col& operator=(const BCP_lp_waiting_col&);
private:
   BCP_var* _var;
   BCP_col* _col;
   double _red_cost;
public:
   BCP_lp_waiting_col(BCP_var* var, BCP_col* col=0, double rc=0) :
      _var(var), _col(col), _red_cost(rc) {}
   ~BCP_lp_waiting_col() {
      delete _col;
      delete _var;
   }

   inline BCP_var* var() { return _var; }
   inline BCP_col* col() { return _col; }
   inline const BCP_var* var() const { return _var; }
   inline const BCP_col* col() const { return _col; }

   inline void clear_var() { _var = 0; }
   inline void delete_col() { delete _col;   _col = 0; _red_cost = 0; }
   inline void set_col(BCP_col*& col) { _col = col; col = 0; }

   inline double red_cost() const { return _red_cost; }
   void compute_red_cost(const BCP_lp_result& lpres);
};

//-----------------------------------------------------------------------------

class BCP_lp_var_pool : public BCP_vec<BCP_lp_waiting_col*> {
private:
   static bool _cols_are_valid;
   // disable the default copy constructor and assignment operator
   BCP_lp_var_pool(const BCP_lp_var_pool&);
   BCP_lp_var_pool& operator=(const BCP_lp_var_pool&);
public:
   BCP_lp_var_pool() {}
   ~BCP_lp_var_pool() { 
      purge_ptr_vector(*(dynamic_cast<BCP_vec<BCP_lp_waiting_col*>*>(this))); 
   }

   inline bool cols_are_valid() const { return _cols_are_valid; }
   inline void cols_are_valid(bool status) { _cols_are_valid = status; }

   inline void compute_red_costs(const BCP_lp_result& lpres,
				 BCP_lp_var_pool::iterator first, 
				 BCP_lp_var_pool::iterator last) {
      if (! _cols_are_valid)
	 throw BCP_fatal_error("\
BCP_lp_var_pool::compute_red_costs() : cols are not valid\n");
      while (first != last) {
	 (*first)->compute_red_cost(lpres);
	 ++first;
      }
   }
   int remove_positives(const double etol);
};

#endif
