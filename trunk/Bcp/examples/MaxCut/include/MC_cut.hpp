// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_CUT_H
#define _MC_CUT_H

#include <algorithm>

#include "CoinHelperFunctions.hpp"

#include "BCP_math.hpp"
#include "BCP_mempool.hpp"
#include "BCP_cut.hpp"

class BCP_buffer;
class BCP_row;
class MC_problem;
class MC_solution;

//#############################################################################

enum MC_cut_t {
  MC_cut_t__cycle,
  MC_cut_t__explicit_dense
};

enum MC_EdgeOrdering {
  MC_MstEdgeOrderingPreferZero,
  MC_MstEdgeOrderingPreferOne,
  MC_MstEdgeOrderingPreferExtreme
};

//#############################################################################

class MC_cycle_cut : public BCP_cut_algo {
private:
   MC_cycle_cut(const MC_cycle_cut&);
   MC_cycle_cut& operator=(const MC_cycle_cut&);
private:
   static BCP_MemPool memPool;
public:
   static inline void * operator new(size_t size) {
      return memPool.alloc(size);
   }
   static inline void operator delete(void *p, size_t size) {
      memPool.free(p, size);
   }
public:
   int  cycle_len; // length of the cycle
   int  pos_edges; // number of edges with positive coefficient
   int* edges;
public:
   MC_cycle_cut(const int* ei, const int pos, const int len) :
      BCP_cut_algo(-BCP_DBL_MAX, pos-1.0),
      cycle_len(len), pos_edges(pos), edges(0)
   {
      if (len > 0) {
	 edges = new int[len];
	 CoinDisjointCopyN(ei, len, edges);
      }
   }
   MC_cycle_cut(BCP_buffer& buf);
   ~MC_cycle_cut() {
      delete[] edges;
   }

   void pack(BCP_buffer& buf) const;
};

//#############################################################################

class MC_explicit_dense_cut : public BCP_cut_algo {
private:
   MC_explicit_dense_cut(const MC_explicit_dense_cut&);
   MC_explicit_dense_cut& operator=(const MC_explicit_dense_cut&);
private:
   static BCP_MemPool memPool;
public:
   static inline void * operator new(size_t size) {
      return memPool.alloc(size);
   }
   static inline void operator delete(void *p, size_t size) {
      memPool.free(p, size);
   }
public:
   double rhs;
   double * coeffs;
   int  varnum;
public:
   MC_explicit_dense_cut(const double ub, const int num,
			 const double * elements) :
      BCP_cut_algo(-BCP_DBL_MAX, ub),
      rhs(ub), coeffs(new double[num]), varnum(num)
   {
      CoinDisjointCopyN(elements, num, coeffs);
   }
   MC_explicit_dense_cut(BCP_buffer& buf);
   ~MC_explicit_dense_cut() {
      delete[] coeffs;
   }

   void pack(BCP_buffer& buf) const;
};

//#############################################################################

inline bool
MC_cycle_cut_less(const BCP_cut* bc0, const BCP_cut* bc1) {
   const MC_cycle_cut* c0 = dynamic_cast<const MC_cycle_cut*>(bc0);
   const MC_cycle_cut* c1 = dynamic_cast<const MC_cycle_cut*>(bc1);
   if (c0->cycle_len < c1->cycle_len)
      return true;
   if (c1->cycle_len < c0->cycle_len)
      return false;
   if (c0->pos_edges < c1->pos_edges)
      return true;
   if (c1->pos_edges < c0->pos_edges)
      return false;
   return memcmp(c0->edges, c1->edges, c0->cycle_len * sizeof(int)) < 0;
}

inline bool
MC_cycle_cut_equal(const BCP_cut* bc0, const BCP_cut* bc1) {
   const MC_cycle_cut* c0 = dynamic_cast<const MC_cycle_cut*>(bc0);
   const MC_cycle_cut* c1 = dynamic_cast<const MC_cycle_cut*>(bc1);
   return (c0->cycle_len == c1->cycle_len &&
	   c0->pos_edges == c1->pos_edges &&
	   memcmp(c0->edges, c1->edges, c0->cycle_len * sizeof(int)) == 0);
}

//#############################################################################

MC_solution*
MC_mst_heur(const MC_problem & mc, const double * x, const double * w,
	    const double alpha, const double beta,
	    const MC_EdgeOrdering edge_ordering,
	    const int heurswitchround,
	    const bool do_edge_switch_heur, const int struct_switch_heur);

//#############################################################################

MC_solution*
MC_mst_cutgen(const MC_problem& mc, const double* x, const double* w,
	      const double alpha, const double beta,
	      const MC_EdgeOrdering edge_ordering,
	      const int heurswitchround,
	      const bool do_edge_switch_heur, const int struct_switch_heur,
	      const double minviol, const int maxnewcuts,
	      BCP_vec<BCP_cut*>& new_cuts, BCP_vec<BCP_row*>& new_rows);

//-----------------------------------------------------------------------------

void
MC_kruskal(const MC_problem& mc, const int * edgeorder, const double* x,
	   int * nodesign, int * edges_in_tree);

//-----------------------------------------------------------------------------

void
MC_kruskal(const MC_problem& mc, const int * edgeorder, const double* x,
	   int * nodesign, int * parentnode, int * parentedge);

//#############################################################################

void
MC_generate_shortest_path_cycles(const MC_problem& mc, const double* x,
				 const bool generate_all_cuts,
				 const double minviol,
				 BCP_vec<BCP_cut*>& new_cuts,
				 BCP_vec<BCP_row*>& new_rows);

//#############################################################################

void
MC_test_ising_four_cycles(const int n, const int* cycles,
			  const double* x, const double minviol,
			  BCP_vec<BCP_cut*>& cuts, BCP_vec<BCP_row*>& rows);

void
MC_test_ising_triangles(const int n, const int* triangles,
			const double* x, const double minviol,
			BCP_vec<BCP_cut*>& cuts, BCP_vec<BCP_row*>& rows);

#endif
