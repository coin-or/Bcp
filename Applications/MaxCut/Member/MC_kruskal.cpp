// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cassert>
#include <algorithm>
#include <numeric>

#include "CoinHelperFunctions.hpp"

#include "MC_cut.hpp"
#include "MC.hpp"

//#############################################################################

static void
MC_label_neighbors(const int this_node, const int* degree,
		   const int* neighbornode, const int* neighboredge,
		   int * parentnode, int * parentedge)
{
  for (int i = degree[this_node]; i < degree[this_node+1]; ++i) {
    const int child = neighbornode[i];
    if (parentnode[child] == -2) {
      parentnode[child] = this_node;
      parentedge[child] = neighboredge[i];
      MC_label_neighbors(child, degree, neighbornode, neighboredge,
			 parentnode, parentedge);
    }
  }
}

//#############################################################################

void
MC_kruskal(const MC_problem& mc, const int* edgeorder, const double* x,
	   int * nodesign, int * edges_in_tree)
{
  //--------------------------------------------------------------------------
  // Kruskal's algo is used (the version when we add edges starting from the
  // cheapest and never creating a cycle). Avoiding cycles is ensured by
  // keeping the components in chains (first each in its own chain), later
  // an edge is added iff its two endpoints have different chain heads.
  //
  // NOTE: the weights on the edges are not needed to find the MST, it's
  // enough that the edges are ordered in increasing weight.
  // HOWEVER, to fill out nodesign we need the weights (or it might be a
  // different vector...).
  //--------------------------------------------------------------------------

  // Also, create the nodesign array: mark the tree nodes with +/- 1 according
  // to the following rule: if the x value of the edge is closer to 0 then the
  // two nodes are on the same side, if it's closer to 1 then they are on the
  // opposite sides. This is done by assigning +1 to each node to start with,
  // and when two chains are merged (the shorted appended to the longer) the
  // nodesigns of all nodes on the shorter chain are reversed if necessary.
  // nodesign is assumed to be exactly mc.num_nodes long.

  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  int k;

  CoinFillN(nodesign, n, 1);

  // the first/last node on the chain containing the i-th node
  // first_on_chain is accurate for every i,
  // last_on_chain is accurate only for chain heads!
  int * first_on_chain = new int[n];
  int * last_on_chain  = new int[n];
  // the next node in the same chain, or -1 if there aren't more. ACCURATE
  int * next_on_chain  = new int[n];
  // size of the chain containing i. Accurate only for chain heads!
  int * size_of_chain  = new int[n];

  CoinIotaN(first_on_chain, n, 0);
  CoinIotaN(last_on_chain, n, 0);
  CoinFillN(next_on_chain, n, -1);
  CoinFillN(size_of_chain, n, 1);

  int tree_size = 0;

  const MC_graph_edge* edges = mc.edges;

  int label_s = -1; // shorter chain head
  int label_l = -1; // longer chain head
  for (k = 0; k < m; ++k) {
    const int kth_edge = edgeorder[k];
    const int i = edges[kth_edge].tail;
    const int j = edges[kth_edge].head;
    const int label_i = first_on_chain[i];
    const int label_j = first_on_chain[j];
    if (label_i == label_j)
      continue;
    if (size_of_chain[label_i] > size_of_chain[label_j]) {
      label_s = label_j;
      label_l = label_i;
    } else {
      label_s = label_i;
      label_l = label_j;
    }
    edges_in_tree[tree_size++] = kth_edge;
    // relabel those in the chain headed by label_s and if necessary, reverse
    // the nodesign of all of them 
    for (int l = label_s; l != -1; l = next_on_chain[l]) {
      first_on_chain[l] = label_l;
    }
    if ((x[kth_edge] > .5 && (nodesign[i] == nodesign[j])) ||
	(x[kth_edge] < .5 && (nodesign[i] != nodesign[j])) ) {
      for (int l = label_s; l != -1; l = next_on_chain[l]) {
	nodesign[l] = -nodesign[l];
      }
    }
    next_on_chain[last_on_chain[label_l]] = label_s;
    last_on_chain[label_l] = last_on_chain[label_s];
    size_of_chain[label_l] += size_of_chain[label_s];
  }

  if (tree_size != n-1) {
    printf("MC: The MST has less than n-1 edges!\n");
    abort();
  }

  delete[] size_of_chain;
  delete[] next_on_chain;
  delete[] last_on_chain;
  delete[] first_on_chain;
}

//#############################################################################

void
MC_kruskal(const MC_problem& mc, const int * edgeorder, const double* x,
	   int * nodesign, int * parentnode, int * parentedge)
{
  const int n = mc.num_nodes;

  int * edges_in_tree = new int[n-1];

  MC_kruskal(mc, edgeorder, x, nodesign, edges_in_tree);

  // Create the rooted tree

  int * neighbornode   = new int[2*n];
  int * neighboredge   = new int[2*n];
  int * degree         = new int[n+1];

  int k;
  const int tree_size = n - 1;
  const MC_graph_edge* edges = mc.edges;

  CoinFillN(degree, n + 1, 0);
  for (k = 0; k < tree_size; ++k) {
    const int l = edges_in_tree[k];
    ++degree[edges[l].tail];
    ++degree[edges[l].head];
  }
  const int maxdeg_node = std::max_element(degree, degree + n) - degree;

  std::partial_sum(degree, degree + n, degree);
  degree[n] = 0;
  std::rotate(degree, degree + n, degree + (n+1));

  for (k = 0; k < tree_size; ++k) {
    const int l = edges_in_tree[k];
    const int i = edges[l].tail;
    const int j = edges[l].head;
    neighbornode[degree[i]] = j;
    neighbornode[degree[j]] = i;
    neighboredge[degree[i]++] = l;
    neighboredge[degree[j]++] = l;
  }
  std::rotate(degree, degree + n, degree + (n+1));
  degree[0] = 0;

  CoinFillN(parentnode, n, -2);
  CoinFillN(parentedge, n, -1);
  parentnode[maxdeg_node] = -1;
  MC_label_neighbors(maxdeg_node, degree, neighbornode, neighboredge,
		     parentnode, parentedge);

  delete[] degree;
  delete[] neighboredge;
  delete[] neighbornode;
  delete[] edges_in_tree;
}
