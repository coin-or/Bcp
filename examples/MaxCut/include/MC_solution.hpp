// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_SOLUTION_H
#define _MC_SOLUTION_H

#include "BCP_vector.hpp"
#include "BCP_solution.hpp"

class BCP_buffer;
class BCP_string;
class MC_problem;
class MC_graph_edge;

class MC_solution : public BCP_solution {
public:
   double cost;
   BCP_vec<int> sig;
public:
  MC_solution(const BCP_vec<int>& sign,
	      const MC_problem& mc,
	      const int heurswitchround,
	      const bool do_edge_switch_heur,
	      const int struct_switch_heur);
  MC_solution() : cost(0.0), sig() {}
  ~MC_solution() {}

  virtual double objective_value() const { return cost; }


  MC_solution& operator=(const MC_solution& sol);

  BCP_buffer& pack(BCP_buffer& buf) const;
  BCP_buffer& unpack(BCP_buffer& buf);

  void display(const BCP_string& fname) const;
  double compute_cost(const int m, const MC_graph_edge* edges);
  double switch_improve(const MC_problem& mc, const int maxiter);
  double edge_switch_improve(const MC_problem& mc, const int maxiter);
  double ising_with_external_edge_switch_improve(const MC_problem& mc,
						 const int maxiter);
  double lk_switch_improve(const MC_problem& mc, const int maxiter);
  double structure_switch_improve(const MC_problem& mc,
				  const int struct_ind, const int maxiter);
};

#endif
