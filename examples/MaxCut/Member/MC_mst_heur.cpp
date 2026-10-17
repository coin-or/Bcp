// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "CoinHelperFunctions.hpp"
#include "CoinSort.hpp"

#include "BCP_vector.hpp"
#include "MC.hpp"
#include "MC_cut.hpp"
#include "MC_solution.hpp"

MC_solution*
MC_mst_heur(const MC_problem & mc, const double * x, const double * w,
	    const double alpha, const double beta,
	    const MC_EdgeOrdering edge_ordering,
	    const int heurswitchround,
	    const bool do_edge_switch_heur, const int struct_switch_heur)
{
  MC_solution* sol;
  
  const int m = mc.num_edges;
  const int n = mc.num_nodes;

  BCP_vec<int> nodesign(n, 1);
  int * edgeorder = new int[m];
  int * edges_in_tree = new int[n-1];
  double * composite_weight = new double[m];

  int j;
  switch (edge_ordering) {
  case MC_MstEdgeOrderingPreferZero:
    if (w == NULL || beta == 0.0) {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha * x[j];
    } else {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha * x[j] - beta * w[j];
    }
    break;
  case MC_MstEdgeOrderingPreferOne:
    if (w == NULL || beta == 0.0) {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha * (1.0-x[j]);
    } else {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha * (1.0-x[j]) - beta * w[j];
    }
    break;
  case MC_MstEdgeOrderingPreferExtreme:
    if (w == NULL || beta == 0.0) {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha * CoinMin(x[j], 1.0-x[j]);
    } else {
      for (j = 0; j < m; ++j)
	composite_weight[j] = alpha*CoinMin(x[j], 1.0-x[j]) - beta*w[j];
    }
    break;
  }
  CoinIotaN(edgeorder, m, 0);

  CoinSort_2(composite_weight, composite_weight + m, edgeorder);

  MC_kruskal(mc, edgeorder, x, nodesign.begin(), edges_in_tree);

  delete[] composite_weight;
  delete[] edges_in_tree;
  delete[] edgeorder;

  sol = new MC_solution(nodesign, mc, heurswitchround,
			do_edge_switch_heur, struct_switch_heur);
  return sol;
}

