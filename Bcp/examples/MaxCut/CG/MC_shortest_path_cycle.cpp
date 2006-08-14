// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <queue>
#include <vector>

#include "CoinSort.hpp"

#include "MC.hpp"
#include "MC_cut.hpp"

#include "BCP_matrix.hpp"

//#############################################################################

void
MC_create_shortest_path_cut(const int n, const int sp_tree_root,
			    const MC_graph_edge* edges,
			    const MC_path_node* bp_nodes,
			    BCP_vec<BCP_cut*>& new_cuts,
			    BCP_vec<BCP_row*>& new_rows,
			    int * itmp, double * dtmp)
{
  int pos = 0, neg = 0;
  int* posedges = itmp;
  int* negedges = itmp + n;

  int i = sp_tree_root + n;
  int parent;
  do {
    parent = bp_nodes[i].parent;
    if ((parent < n && i < n) || (parent >= n && i >= n)) {
      // side edge thus not in F
      negedges[neg++] = bp_nodes[i].edge_to_parent;
    } else {
      // crossing edge thus in F
      posedges[pos++] = bp_nodes[i].edge_to_parent;
    }
    i = parent;
  } while (parent != sp_tree_root);

  if ((pos % 2) != 1)
    abort();
  const int len = pos + neg;
  CoinDisjointCopyN(negedges, neg, posedges + pos);
  new_cuts.push_back(new MC_cycle_cut(posedges, pos, len));
  CoinFillN(dtmp, pos, 1.0);
  CoinFillN(dtmp + pos, neg, -1.0);
  CoinSort_2(posedges, posedges + len, dtmp);
  new_rows.push_back(new BCP_row(posedges, posedges + len,
				 dtmp, dtmp + len, -1e40, pos - 1.0));
}

//#############################################################################

int
MC_find_components(const int nodenum, const MC_path_node* bp_nodes,
		   int * component)
{
  int* node_stack = new int[nodenum];
  int stack_size = 0;
  int comp_num = 0;
  int i, j;
  for (i = 0; i < nodenum; ++i) {
    if (component[i] >= 0) // it's already encountered
      continue;
    node_stack[stack_size++] = i;
    ++comp_num;
    while (stack_size > 0) {
      const int node = node_stack[--stack_size];
      component[node] = i;
      const MC_path_adj_entry* neighbors = bp_nodes[node].adj_list;
      for (j = bp_nodes[node].degree - 1; j >= 0; --j) {
	const int other = neighbors[j].neighbor;
	if (component[other] == -1) {
	  component[other] = i;
	  node_stack[stack_size++] = other;
	}
      }
    }
  }
  delete[] node_stack;
  return comp_num;
}

//#############################################################################

void
MC_generate_shortest_path_cycles(const MC_problem& mc, const double* x,
				 const bool generate_all_cuts,
				 const double minviol,
				 BCP_vec<BCP_cut*>& new_cuts,
				 BCP_vec<BCP_row*>& new_rows)
{
  const double minviol1 = 1.0 - minviol;

  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  double* dtmp = new double[n];
  int* itmp = new int[4 * n + m];
  // the first 2n entries will be used in MC_generate_shortest_path_cycles as
  // temporaries
  int* component = itmp + (2 * n);
  int* edges_to_create = itmp + (4 * n);
  const MC_graph_edge* edges = mc.edges;

  MC_path_node* bp_nodes = new MC_path_node[2 * n];
  MC_path_adj_entry* bp_all_adj_list = new MC_path_adj_entry[8 * m];

  int i, j;

  for (i = 0; i < n; ++i)
    bp_nodes[i].degree = 0;

  // Build the bipartite graph
  for (i = 0; i < m; ++i) {
    const double xi = x[i];
    const int t = edges[i].tail;
    const int h = edges[i].head;
    edges_to_create[i] = 0;
    if (xi >= minviol) {
      // the length of the crossing edges is < 1 - minviol, so there can be a
      // violated cycle through them, so they exist
      edges_to_create[i] |= 1;
      ++bp_nodes[t].degree;
      ++bp_nodes[h].degree;
    }
    if (xi < minviol1) {
      // the length of the side edges is < 1 - minviol, so there can be a
      // violated cycle through them, so they exist
      edges_to_create[i] |= 2;
      ++bp_nodes[t].degree;
      ++bp_nodes[h].degree;
    }
  }

  int total_degree = 0;
  for (i = 0; i < n; ++i) {
    bp_nodes[i].adj_list = bp_all_adj_list + total_degree;
    total_degree += bp_nodes[i].degree;
    bp_nodes[i + n].adj_list = bp_all_adj_list + total_degree;
    total_degree += bp_nodes[i].degree;
  }

  for (i = 0; i < n; ++i)
    bp_nodes[i].degree = 0;

  for (i = 0; i < m; ++i) {
    const double xi = CoinMax(x[i], 0.0);
    const double xi1 = CoinMax(1 - xi, 0.0);
    const int t = edges[i].tail;
    const int h = edges[i].head;
    const int nt = n + t;
    const int nh = n + h;
    if ((edges_to_create[i] & 1) != 0) { // create the crossing edges
      const int degt = bp_nodes[t].degree;
      const int degh = bp_nodes[h].degree;
      MC_path_adj_entry& t_entry = bp_nodes[t].adj_list[degt];
      MC_path_adj_entry& h_entry = bp_nodes[h].adj_list[degh];
      MC_path_adj_entry& nt_entry = bp_nodes[nt].adj_list[degt];
      MC_path_adj_entry& nh_entry = bp_nodes[nh].adj_list[degh];
      
      t_entry.neighbor = nh;
      h_entry.neighbor = nt;
      nt_entry.neighbor = h;
      nh_entry.neighbor = t;

      t_entry.orig_edge = i;
      h_entry.orig_edge = i;
      nt_entry.orig_edge = i;
      nh_entry.orig_edge = i;

      t_entry.cost = xi1;
      h_entry.cost = xi1;
      nt_entry.cost = xi1;
      nh_entry.cost = xi1;

      ++bp_nodes[t].degree;
      ++bp_nodes[h].degree;
    }
    if ((edges_to_create[i] & 2) != 0) { // create the side edges
      const int degt = bp_nodes[t].degree;
      const int degh = bp_nodes[h].degree;
      MC_path_adj_entry& t_entry = bp_nodes[t].adj_list[degt];
      MC_path_adj_entry& h_entry = bp_nodes[h].adj_list[degh];
      MC_path_adj_entry& nt_entry = bp_nodes[nt].adj_list[degt];
      MC_path_adj_entry& nh_entry = bp_nodes[nh].adj_list[degh];

      t_entry.neighbor = h;
      h_entry.neighbor = t;
      nt_entry.neighbor = nh;
      nh_entry.neighbor = nt;

      t_entry.orig_edge = i;
      h_entry.orig_edge = i;
      nt_entry.orig_edge = i;
      nh_entry.orig_edge = i;

      t_entry.cost = xi;
      h_entry.cost = xi;
      nt_entry.cost = xi;
      nh_entry.cost = xi;

      ++bp_nodes[t].degree;
      ++bp_nodes[h].degree;
    }
  }
  for (i = 0; i < n; ++i)
    bp_nodes[n+i].degree = bp_nodes[i].degree;

  // Create a priority queue that will hold the shortest path tree
  std::priority_queue<MC_path_node*,
                      std::vector<MC_path_node*>,
                      MC_path_node_ptr_compare> pqueue;

  CoinFillN(component, 2 * n, -1);

  const int comp_num = MC_find_components(2*n, bp_nodes, component);
  printf("MC: sp based exact: there are %i components in the graph\n",
	 comp_num);

  /* Check the nodes where we already have a sufficiently violated cycle
     through the node. If such a node and it's pair is in the same component
     then set the component id of one to be -1 so we skip the shortest path
     computation for them. */
  double viol;
  int skip = 0;
  for (i = new_cuts.size() - 1; i >= 0; --i) {
    const MC_cycle_cut* cut = dynamic_cast<const MC_cycle_cut*>(new_cuts[i]);
    if (! cut)
      continue;
    viol = - (cut->pos_edges - 1);
    for (j = 0; j < cut->pos_edges; ++j) {
      viol += x[cut->edges[j]];
    }
    for ( ; j < cut->cycle_len; ++j) {
      viol -= x[cut->edges[j]];
    }
    if (viol <= minviol)
      continue;
    for (j = 0; j < cut->cycle_len; ++j) {
      const int t = edges[cut->edges[j]].tail;
      const int h = edges[cut->edges[j]].head;
      if (component[t] == component[n + t]) {
	++skip;
	component[t] = -2;
      }
      if (component[h] == component[n + h]) {
	++skip;
	component[h] = -2;
      }
    }
  }
  printf("MC: sp based exact: skipping %i sp computations\n", skip);
  printf("                    because of already found cycles\n");

  int split_num = 0;
  for (i = 0; i < n; ++i) {
    if (component[i] != component[n + i]) {
      ++split_num;
      continue;
    }
    // do the shortest path
    double limit = minviol1;

    // set the original distance of everything to 1.0 (that's larger than the
    // longest edge, and we will anyway terminate every path that's longer
    // than limit). Note that a node can be in the priority queue more than
    // once, but it's going to be processed only once. All this is necessary
    // because the std priority queue installation cannot "relocate" a node
    // when it's value changes.
    for (j = 2*n - 1; j >= 0; --j) {
      bp_nodes[j].distance = 1.0;
      bp_nodes[j].processed = false;
    }
    bp_nodes[i].distance = 0.0;

    pqueue.push(bp_nodes + i);

    while (! pqueue.empty()) {
      MC_path_node* this_node_nonconst = pqueue.top();
      const MC_path_node* this_node = this_node_nonconst;
      pqueue.pop();
      if (this_node->processed)
	continue;
      const int this_node_index = this_node - bp_nodes;
      if (this_node_index == n+i) {
	// got the shortest path to the pair, need not go further
	if (! generate_all_cuts) {
	  // wee needed only the best cut for each node, here it is
	  MC_create_shortest_path_cut(n, this_node_index, edges, bp_nodes,
				      new_cuts, new_rows, itmp, dtmp);
	}
	break;
      }
      this_node_nonconst->processed = true;
      const double this_dist = this_node->distance;
      const MC_path_adj_entry* this_alist = this_node->adj_list;
      for (j = this_node->degree - 1; j >= 0; --j) {
	const double neighborcost = this_dist + this_alist[j].cost;
	if (neighborcost >= limit)
	  continue;
	const int neighbor = this_alist[j].neighbor;
	if (bp_nodes[neighbor].distance > neighborcost) {
	  MC_path_node* neighbor_node = bp_nodes + neighbor;
	  pqueue.push(neighbor_node);
	  neighbor_node->distance = neighborcost;
	  neighbor_node->parent = this_node_index;
	  neighbor_node->edge_to_parent = this_alist[j].orig_edge;
	  if (neighbor == n+i) {
	    // found a new violated cycle...
	    limit = neighborcost;
	    if (generate_all_cuts) {
	      MC_create_shortest_path_cut(n, i, edges, bp_nodes,
					  new_cuts, new_rows, itmp, dtmp);
	    }
	  }
	}
      }
    }
  }

  printf("MC: sp based exact: computed %i sp out of %i\n", n - split_num, n);

  delete[] bp_all_adj_list;
  delete[] bp_nodes;
  delete[] itmp;
  delete[] dtmp;
}
