// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>
#include <algorithm>

#include "CoinSort.hpp"
#include "BCP_vector.hpp"
#include "BCP_matrix.hpp"

#include "MC_cut.hpp"
#include "MC_solution.hpp"
#include "MC.hpp"

//#############################################################################

void
MC_cuts_from_mst(const MC_problem& mc, const double* x, const double minviol,
		 const BCP_vec<int>& nodesign,
		 const BCP_vec<int>& parentnode,
		 const BCP_vec<int>& parentedge,
		 const int maxnewcuts,
		 BCP_vec<BCP_cut*>& new_cuts, BCP_vec<BCP_row*>& new_rows)
		 
		 
{
  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  const MC_graph_edge* edges = mc.edges;

  double* coefs = new double[n];
  int* posedges = new int[n];
  int* negedges = new int[n];
  int pos, neg;
  double sum;

  BCP_vec<int> nodepathi;
  BCP_vec<int> nodepathj;
  BCP_vec<int> edgepathi;
  BCP_vec<int> edgepathj;

  nodepathi.reserve(n+1);
  nodepathj.reserve(n+1);
  edgepathi.reserve(n);
  edgepathj.reserve(n);
  

  int ii, jj;

  // randomly reorder the edges so that if not all edges are checked then at
  // least we shuffle which ones are checked.
  int* randomorder = new int[m];
  CoinIotaN(randomorder, m, 0);
  std::random_shuffle(randomorder, randomorder + m);

  for (int l = 0; l < m; ++l) {
    const int k = randomorder[l];
    const int i = edges[k].tail;
    const int j = edges[k].head;
    // first check if it's a tree edge...
    if (parentnode[i] == j || parentnode[j] == i)
      continue;
    // Check if the cycle is likely to be violated
#if 0
    if ((nodesign[i] == nodesign[j] && x[k] < .3) ||
	(nodesign[i] != nodesign[j] && x[k] > .7))
      continue;
#endif
    // Otherwise build the paths to the root (root does get on the path, too)
    nodepathi.clear();
    nodepathj.clear();
    edgepathi.clear();
    edgepathj.clear();
    for (ii = i; ii != -1; ii = parentnode[ii]) {
      nodepathi.unchecked_push_back(ii);
      edgepathi.unchecked_push_back(parentedge[ii]);
    }
    for (jj = j; jj != -1; jj = parentnode[jj]) {
      nodepathj.unchecked_push_back(jj);
      edgepathj.unchecked_push_back(parentedge[jj]);
    }
    // find where do they intersect first (they do intersect, maybe just in
    // the root). also, if one of them is on the other's path the ii or jj
    // will be -1 after this loop ends.
    for (ii = nodepathi.size()-1, jj = nodepathj.size()-1;
	 ii >= 0 && jj >= 0; --ii, --jj) {
      if (nodepathi[ii] != nodepathj[jj])
	break;
    }

    // Compute the lhs of the inequality
    sum = 0.0;
    pos = 0;
    neg = 0;
    if (nodesign[i] == nodesign[j]) {
      sum = x[k];
      posedges[pos++] = k;
    } else {
      sum = -x[k];
      negedges[neg++] = k;
    }
    for ( ; ii >= 0; --ii) {
      const int p0 = nodepathi[ii];
      const int p1 = nodepathi[ii+1];
      const int path_ind = edgepathi[ii];
      if (nodesign[p0] == nodesign[p1]) {
	sum -= x[path_ind];
	negedges[neg++] = path_ind;
      } else {
	sum += x[path_ind];
	posedges[pos++] = path_ind;
      }
    }
    for ( ; jj >= 0; --jj) {
      const int p0 = nodepathj[jj];
      const int p1 = nodepathj[jj+1];
      const int path_ind = edgepathj[jj];
      if (nodesign[p0] == nodesign[p1]) {
	sum -= x[path_ind];
	negedges[neg++] = path_ind;
      } else {
	sum += x[path_ind];
	posedges[pos++] = path_ind;
      }
    }
    if (sum <= pos - 1.0 + minviol)
      continue;

    if ((pos % 2) != 1)
      abort();
    // Great! A sufficiently violated cycle. create the cut
    const int len = pos + neg;
    CoinDisjointCopyN(negedges, neg, posedges + pos);
    new_cuts.push_back(new MC_cycle_cut(posedges, pos, len));
    CoinFillN(coefs, pos, 1.0);
    CoinFillN(coefs + pos, neg, -1.0);
    CoinSort_2(posedges, posedges + len, coefs);
    new_rows.push_back(new BCP_row(posedges, posedges + len,
				   coefs, coefs + len, -1e40, pos - 1.0));
    if (static_cast<int>(new_cuts.size()) >= maxnewcuts)
      break;
  }

  delete[] randomorder;
  delete[] negedges;
  delete[] posedges;
  delete[] coefs;
}

//#############################################################################

MC_solution*
MC_mst_cutgen(const MC_problem& mc, const double* x, const double* w,
	      const double alpha, const double beta,
	      const MC_EdgeOrdering edge_ordering,
	      const int heurswitchround,
	      const bool do_edge_switch_heur, const int struct_switch_heur,
	      const double minviol, const int maxnewcuts,
	      BCP_vec<BCP_cut*>& new_cuts, BCP_vec<BCP_row*>& new_rows)
{
  const int n = mc.num_nodes;
  const int m = mc.num_edges;

  // will do minimum spanning tree based cut gen heuristic. The MST is done
  // in a graph where the weight of the edges is min(x, 1-x) + a perturbation
  BCP_vec<double> weights;
  BCP_vec<int> edgeorder;
  weights.reserve(m);
  edgeorder.reserve(m);

  int i;

  switch (edge_ordering) {
  case MC_MstEdgeOrderingPreferZero:
    if (w == NULL || beta == 0.0) {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back( alpha * x[i] );
    } else {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back(alpha * x[i] - beta * w[i]);
    }
    break;
  case MC_MstEdgeOrderingPreferOne:
    if (w == NULL || beta == 0.0) {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back( alpha * (1.0-x[i]) );
    } else {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back(alpha * (1.0-x[i]) - beta * w[i]);
    }
    break;
  case MC_MstEdgeOrderingPreferExtreme:
    if (w == NULL || beta == 0.0) {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back( alpha * CoinMin(x[i], 1.0-x[i]) );
    } else {
      for (i = 0; i < m; ++i)
	weights.unchecked_push_back(alpha*CoinMin(x[i], 1.0-x[i]) - beta*w[i]);
    }
    break;
  }
  for (i = 0; i < m; ++i)
    edgeorder.unchecked_push_back(i);

  CoinSort_2(weights.begin(), weights.end(), edgeorder.begin());

  BCP_vec<int> parentnode(n, -1);
  BCP_vec<int> parentedge(n, -1);
  BCP_vec<int> nodesign(n, 1);

  MC_kruskal(mc, edgeorder.begin(), x, nodesign.begin(),
	     parentnode.begin(), parentedge.begin());

  // Now if we do not yet have enough cuts then go through every edge and
  // check whether the cycle it creates is  violated.
  if (static_cast<int>(new_cuts.size()) < maxnewcuts)
    MC_cuts_from_mst(mc, x, minviol,
		     nodesign, parentnode, parentedge,
		     maxnewcuts, new_cuts, new_rows);

  // now create the heur solution
  MC_solution* sol = new MC_solution(nodesign, mc, heurswitchround,
				     do_edge_switch_heur, struct_switch_heur);
  return sol;
}
