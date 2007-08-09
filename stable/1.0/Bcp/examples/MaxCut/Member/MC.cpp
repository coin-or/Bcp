// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_buffer.hpp"

#include "MC.hpp"
#include <cmath>

BCP_buffer&
MC_problem::pack(BCP_buffer& buf)
{
  int i, j;
  buf.pack(num_nodes).pack(num_edges);
  for (i = 0; i < num_edges; ++i) {
    buf.pack(edges[i].tail).pack(edges[i].head).pack(edges[i].cost);
  }

  const int grid = static_cast<int>(sqrt(num_nodes + 1.0));
  const int num_grid_nodes = grid * grid;

  buf.pack(ising_problem);
  const bool has_four_cycles = ising_four_cycles != NULL;
  buf.pack(has_four_cycles);
  if (has_four_cycles)
    buf.pack(ising_four_cycles, 4 * num_grid_nodes);

  const bool has_triangles = ising_triangles != NULL;
  buf.pack(has_triangles);
  if (has_triangles)
    buf.pack(ising_triangles, 6 * num_grid_nodes);

  buf.pack(num_structure_type);
  if (num_structure_type > 0) {
    buf.pack(num_switch_structures, num_structure_type);
    for (i = 0; i < num_structure_type; ++i) {
      const MC_switch_structure* sw_structs = switch_structures[i];
      for (j = 0; j < num_switch_structures[i]; ++j) {
	const int num_nodes_j = sw_structs[j].num_nodes;
	const int num_neighbors_j = sw_structs[j].num_neighbors;
	buf.pack(num_nodes_j).pack(num_neighbors_j);
	buf.pack(sw_structs[j].nodes, num_nodes_j + num_neighbors_j);
      }
    }
  }

  const bool has_feas_sol = feas_sol != 0;
  buf.pack(has_feas_sol);
  if (has_feas_sol) {
     buf.pack(feas_sol->sign, feas_sol->num_nodes)
	.pack(feas_sol->value, feas_sol->num_edges)
	.pack(feas_sol->cost);
  }

  return buf;
}

BCP_buffer&
MC_problem::unpack(BCP_buffer& buf)
{
  int i, j;
  buf.unpack(num_nodes).unpack(num_edges);
  edges = new MC_graph_edge[num_edges];
  for (i = 0; i < num_edges; ++i) {
    buf.unpack(edges[i].tail).unpack(edges[i].head).unpack(edges[i].cost);
  }

  int itmp;

  buf.unpack(ising_problem);
  bool has_four_cycles;
  buf.unpack(has_four_cycles);
  if (has_four_cycles)
    buf.unpack(ising_four_cycles, itmp, true /* allocate */);

  bool has_triangles;
  buf.unpack(has_triangles);
  if (has_triangles)
    buf.unpack(ising_triangles, itmp, true /* allocate */);

  buf.unpack(num_structure_type);
  if (num_structure_type > 0) {
    buf.unpack(num_switch_structures, itmp, true /* allocate */);
    switch_structures = new MC_switch_structure*[num_structure_type];
    for (i = 0; i < num_structure_type; ++i) {
      if (num_switch_structures[i] > 0) {
	switch_structures[i] =
	  new MC_switch_structure[num_switch_structures[i]];
	int num_nodes_j;
	int num_neighbors_j;
	MC_switch_structure* sw_structs = switch_structures[i];
	for (j = 0; j < num_switch_structures[i]; ++j) {
	  buf.unpack(num_nodes_j).unpack(num_neighbors_j);
	  sw_structs[j].num_nodes = num_nodes_j;
	  sw_structs[j].num_neighbors = num_neighbors_j;
	  buf.unpack(sw_structs[j].nodes, itmp, true /* allocate */);
	  sw_structs[j].neighbors = sw_structs[j].nodes + num_nodes_j;
	}
      }
    }
  }


  bool has_feas_sol;
  buf.unpack(has_feas_sol);
  if (has_feas_sol) {
     int n, m, *s;
     double c, *v;
     buf.unpack(s, n).unpack(v, m).unpack(c);
     feas_sol = new MC_feas_sol(c, n, s, m, v);
  }
  return buf;
}

void
MC_problem::create_adj_lists()
{
  int i;

  nodes = new MC_graph_node[num_nodes];
  for (i = 0; i < num_nodes; ++i)
    nodes[i].degree = 0;
  for (i = 0; i < num_edges; ++i) {
    ++nodes[edges[i].tail].degree;
    ++nodes[edges[i].head].degree;
  }

  all_adj_list = new MC_adjacency_entry[2 * num_edges];

  int total_degree = 0;
  for (i = 0; i < num_nodes; ++i) {
    nodes[i].adj_list = all_adj_list + total_degree;
    total_degree += nodes[i].degree;
  }
  
  for (i = 0; i < num_nodes; ++i)
    nodes[i].degree = 0;

  for (i = 0; i < num_edges; ++i) {
    const int t = edges[i].tail;
    const int h = edges[i].head;
    const double c = edges[i].cost;
    const int degt = nodes[t].degree;
    const int degh = nodes[h].degree;
    nodes[t].adj_list[degt].neighbor = h;
    nodes[h].adj_list[degh].neighbor = t;
    nodes[t].adj_list[degt].cost = c;
    nodes[h].adj_list[degh].cost = c;
    nodes[t].adj_list[degt].index = i;
    nodes[h].adj_list[degh].index = i;
    ++nodes[t].degree;
    ++nodes[h].degree;
  }
}
