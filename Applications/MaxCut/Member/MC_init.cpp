// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <fstream>
#include <algorithm>

#include "BCP_tm.hpp"

#include "MC_init.hpp"
#include "MC_cut.hpp"
#include "MC_tm.hpp"
#include "MC_lp.hpp"

//#############################################################################

USER_initialize * BCP_user_init()
{
   return new MC_initialize;
}

//#############################################################################

void MC_read_parameters(MC_tm& tm, const char * paramfile);
void MC_readproblem(MC_tm& tm);

//-----------------------------------------------------------------------------

BCP_tm_user *
MC_initialize::tm_init(BCP_tm_prob& p,
		       const int argnum, const char * const * arglist)
{
   MC_tm* tm = new MC_tm;

   MC_read_parameters(*tm, arglist[1]);
   MC_readproblem(*tm);

   MC_problem &mc = tm->mc;
   if (mc.ising_problem) {
      const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
      const int num_grid_nodes = grid * grid;
      const bool has_extra_node = (mc.num_nodes != num_grid_nodes);
      const double granularity = has_extra_node ? .99 : 1.99;
      p.par.set_entry(BCP_tm_par::Granularity, granularity);
      p.slave_pars.lp.set_entry(BCP_lp_par::Granularity, granularity);
   }

   return tm;
}

//-----------------------------------------------------------------------------

BCP_lp_user *
MC_initialize::lp_init(BCP_lp_prob& p)
{
   MC_lp* lp = new MC_lp;
   return lp;
}

//#############################################################################

void MC_read_parameters(MC_tm& tm, const char * paramfile)
{
   tm.tm_par.read_from_file(paramfile);
   tm.lp_par.read_from_file(paramfile);
}

//#############################################################################

int
MC_components(const int n, const int m, const MC_graph_edge* edges,
	      int* component)
{
  // See the MC_kruskal.cpp file.
  // This is the same algorithm, just here we don't need all those things that
  // are computed there
  int * first_on_chain = component;
  int * last_on_chain = new int[n];
  int * next_on_chain = new int[n];
  int * size_of_chain = new int[n];

  CoinIotaN(first_on_chain, n, 0);
  CoinIotaN(last_on_chain, n, 0);
  CoinFillN(next_on_chain, n, -1);
  CoinFillN(size_of_chain, n, 1);

  int tree_size = 0;

  int label_s = -1; // shorter chain head
  int label_l = -1; // longer chain head
  for (int k = 0; k < m; ++k) {
    const int i = edges[k].tail;
    const int j = edges[k].head;
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
    ++tree_size;
    for (int l = label_s; l != -1; l = next_on_chain[l]) {
      first_on_chain[l] = label_l;
    }
    next_on_chain[last_on_chain[label_l]] = label_s;
    last_on_chain[label_l] = last_on_chain[label_s];
    size_of_chain[label_l] += size_of_chain[label_s];
  }
  
  delete[] last_on_chain;
  delete[] next_on_chain;
  delete[] size_of_chain;

  return tree_size;
}

//#############################################################################

static void
MC_fill_structure(const MC_problem& mc, MC_switch_structure& swstruct,
		  const int num_nodes, const int * nodes)
{
  const int * nodes_end = nodes + num_nodes;
  // count the edges going to real neighbors
  int outedges = 0;
  int i, j;
  for (i = 0; i < num_nodes; ++i) {
    MC_adjacency_entry* adj_list = mc.nodes[nodes[i]].adj_list;
    for (j = mc.nodes[nodes[i]].degree - 1; j >= 0; --j) {
      if (std::find(nodes, nodes_end, adj_list[j].neighbor) == nodes_end)
	++outedges;
    }
  }
  swstruct.num_nodes = num_nodes;
  swstruct.num_neighbors = outedges;
  swstruct.nodes = new int[num_nodes + outedges];
  swstruct.neighbors = swstruct.nodes + num_nodes;
  CoinDisjointCopyN(nodes, num_nodes, swstruct.nodes);
  outedges = 0;
  for (i = 0; i < num_nodes; ++i) {
    MC_adjacency_entry* adj_list = mc.nodes[nodes[i]].adj_list;
    for (j = mc.nodes[nodes[i]].degree - 1; j >= 0; --j) {
      if (std::find(nodes, nodes_end, adj_list[j].neighbor) == nodes_end)
	swstruct.neighbors[outedges++] = adj_list[j].index;
    }
  }
}

//#############################################################################

typedef std::pair<int,int> intpair;

static inline bool
operator<(const intpair& ip0, const intpair& ip1)
{
  return ((ip0.first < ip1.first) ||
	  ((ip0.first == ip1.first) && (ip0.second < ip1.second)));
}

void MC_readproblem(MC_tm& tm)
{
  int i, j, k;
  MC_problem &mc = tm.mc;

  std::ifstream file(tm.tm_par.entry(MC_tm_par::InputFile).c_str());
  if (! file) {
    throw BCP_fatal_error("Can't open input file!\n");
  }

  char line[1000];

  double rescale = 1;
  for (int lose = tm.tm_par.entry(MC_tm_par::DigitsToLose); lose > 0; --lose) {
     rescale *= 10.0;
  }

  file.getline(line, 999);
  int len = strlen(line);
  if (strncmp(line, "DESC: ggrid", CoinMin(len, 11)) == 0) {
     // Ising problem generated by ggrid. Parse the info.
     mc.ising_problem = true;
     printf("Ising problem detected.\n");
     file.getline(line, 999);
     sscanf(line, "DESC: scaling: %lf", &mc.scaling_factor);
     mc.scaling_factor /= rescale;
  }
  while (strncmp(line, "DESC:", CoinMin(len, 5)) == 0) {
     file.getline(line, 999);
  }

  sscanf(line, "%i%i", &mc.num_nodes, &mc.num_edges);

  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  // this will be definitely enough, even for new edges to connect
  // disconnected components.
  mc.edges = new MC_graph_edge[m + n];

  std::map<intpair, int> edge_map;

  double cost;
  intpair ip;
  for (i = 0, k = 0; i < m; ++i) {
    file.getline(line, 999);
    sscanf(line, "%i%i%lf", &ip.first, &ip.second, &cost);
    if (ip.first < 0 || ip.first >= n || ip.second < 0 || ip.second >= n ||
	(ip.first == ip.second)) {
      char errmsg[200];
      sprintf(errmsg, " bad endnodes %i %i\n", ip.first, ip.second);
      throw BCP_fatal_error(errmsg);
    }
    if (ip.first > ip.second)
      std::swap(ip.first, ip.second);
    const std::map<intpair, int>::const_iterator em = edge_map.find(ip);
    if (em != edge_map.end()) {
      printf("Warning: edge (%i,%i) appears once again.\n",
	     ip.first, ip.second);
      printf("         Collapsing the parallel edges.\n");
      mc.edges[em->second].cost += cost;
    } else {
      edge_map[ip] = k;
      mc.edges[k].tail = ip.first;
      mc.edges[k].head = ip.second;
      mc.edges[k++].cost = cost;
    }
  }
  mc.num_edges = k;

  file.close();

  // throw out the 0 cost edges if it's not an ising problem. For ising
  // problems it's worth to keep the 0 cost edges because then the structure
  // is preserved.
  if (! mc.ising_problem) {
    for (i = 0, k = 0; i < mc.num_edges; ++i) {
      if (mc.edges[i].cost == 0.0) {
	printf("Warning: Discarded edge (%i,%i) as it has final cost 0.\n",
	       mc.edges[i].tail, mc.edges[i].head);
      } else {
	mc.edges[k].tail =  mc.edges[i].tail;
	mc.edges[k].head =  mc.edges[i].head;
	mc.edges[k++].cost =  mc.edges[i].cost;
      }
    }
    mc.num_edges = k;

    // Check connectivity
    int * components = new int[mc.num_nodes];
    const int comp_num =
      mc.num_nodes - MC_components(mc.num_nodes, mc.num_edges,
				   mc.edges, components);
    if (comp_num > 1) {
      printf("There are %i components in the graph. Adding 0 cost edges.\n",
	     comp_num);
      std::set<int> seen;
      seen.insert(components[0]);
      for (i = 0; i < mc.num_nodes; ++i) {
	if (seen.find(components[i]) == seen.end()) {
	  // not seen component. connect it
	  mc.edges[mc.num_edges].tail = 0;
	  mc.edges[mc.num_edges].head = i;
	  mc.edges[mc.num_edges++].cost = 0;
	  seen.insert(components[i]);
	}
      }
    }
    delete[] components;
  }

  // rescale the edges
  for (i = 0; i < mc.num_edges; ++i) {
     const double c = mc.edges[i].cost;
     mc.edges[i].cost = floor(c / rescale + 0.5);
  }
  

  // Negate the cost of the edges (minimizing!) and compute their sum
  mc.sum_edge_weight = 0;
  for (i = 0; i < mc.num_edges; ++i) {
    const double c = - mc.edges[i].cost;
    mc.edges[i].cost = c;
    mc.sum_edge_weight += c;
  }

  mc.create_adj_lists();

  if (mc.ising_problem) {
    const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
    const int num_grid_nodes = grid * grid;
    const int num_grid_edges = 2 * num_grid_nodes;
    const int vertical = num_grid_nodes;
    {
      // enumerate the 4-cycles
      mc.ising_four_cycles = new int[num_grid_nodes * 4];
      int * cycle = mc.ising_four_cycles;
      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  // (i,j) -> (i,j+1)
	  cycle[0] = i*grid + j;
	  // (i,j+1) -> (i+1,j+1)
	  cycle[1] = vertical + i*grid + ((j+1) % grid);
	  // (i+1,j) -> (i+1,j+1)
	  cycle[2] = ((i+1) % grid) * grid + j;
	  // (i,j) -> (i+1,j)
	  cycle[3] = vertical + i*grid + j;
	  cycle += 4;
	}
      }
    }

    const bool has_extra_node = (mc.num_nodes != num_grid_nodes);
    if (has_extra_node) {
      // enumerate the triangles
      mc.ising_triangles = new int[2 * num_grid_nodes * 3];
      int * triangle = mc.ising_triangles;
      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  // (i,j) -> (i,j+1) -> extra_node ->
	  triangle[0] = i*grid + j;
	  triangle[1] = num_grid_edges + i*grid + j;
	  triangle[2] = num_grid_edges + i*grid + ((j+1) % grid);
	  if (triangle[1] > triangle[2])
	    std::swap(triangle[1], triangle[2]);
	  triangle += 3;
	  // (i,j) -> (i+1,j) -> extra_node ->
	  triangle[0] = vertical + i*grid + j;
	  triangle[1] = num_grid_edges + i*grid + j;
	  triangle[2] = num_grid_edges + ((i+1) % grid) * grid + j;
	  if (triangle[1] > triangle[2])
	    std::swap(triangle[1], triangle[2]);
	  triangle += 3;
	}
      }
    }

    mc.num_structure_type = 5;

    mc.num_switch_structures = new int[mc.num_structure_type];
    mc.switch_structures = new MC_switch_structure*[mc.num_structure_type];

    // three-node connected structures
    {
      mc.num_switch_structures[0] = num_grid_nodes * 6;
      mc.switch_structures[0] = new MC_switch_structure[num_grid_nodes * 6];
      MC_switch_structure * next_struct = mc.switch_structures[0];

      int nodes[3];

      // enumerate the outgoing edges of three long chains
      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  //-------------------------------------------------------------------
	  // the three nodes: (i-1,j), (i,j), (i+1,j)
	  //  |
	  //  |
	  nodes[0] = ((i+grid-1)%grid) * grid + j;
	  nodes[1] = i * grid + j;
	  nodes[2] = ((i+1)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;

	  //-------------------------------------------------------------------
	  // the three nodes: (i,j-1), (i,j), (i,j+1)
	  //  --
	  nodes[0] = i * grid + ((j+grid-1)%grid);
	  nodes[1] = i * grid + j;
	  nodes[2] = i * grid + ((j+1)%grid);
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;

	  //-------------------------------------------------------------------
	  // the three nodes: (i-1,j), (i,j), (i,j+1)
	  //  |_
	  nodes[0] = ((i+grid-1)%grid) * grid + j;
	  nodes[1] = i * grid + j;
	  nodes[2] = i * grid + ((j+1)%grid);
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;

	  //-------------------------------------------------------------------
	  // (i,j+1), (i,j), (i+1,j)
	  //   _
	  //  |
	  nodes[0] = i * grid + ((j+1)%grid);
	  nodes[1] = i * grid + j;
	  nodes[2] = ((i+1)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;

	  //-------------------------------------------------------------------
	  // the three nodes: (i+1,j), (i,j), (i,j-1)
	  //  _
	  //   |
	  nodes[0] = ((i+1)%grid) * grid + j;
	  nodes[1] = i * grid + j;
	  nodes[2] = i * grid + ((j+grid-1)%grid);
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;

	  //-------------------------------------------------------------------
	  // the three nodes: (i,j-1), (i,j), (i-1,j)
	  //  _|
	  nodes[0] = i * grid + ((j+grid-1)%grid);
	  nodes[1] = i * grid + j;
	  nodes[2] = ((i+grid-1)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 3, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 3 : 0))
	    abort();
	  ++next_struct;
	}
      }
    }

    //=========================================================================

    // different four-node connected structures

    // first
    {
      mc.num_switch_structures[1] = 4 * num_grid_nodes;
      mc.switch_structures[1] = new MC_switch_structure[4 * num_grid_nodes];
      MC_switch_structure * next_struct = mc.switch_structures[1];

      int nodes[4];

      // List the outgoing edges from every square
      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  // _|_
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j-1+grid)%grid);
	  nodes[2] = ((i-1+grid)%grid) * grid + j;
	  nodes[3] = i * grid + ((j+1)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  // |_
	  // |
	  nodes[0] = i * grid + j;
	  nodes[1] = ((i-1+grid)%grid) * grid + j;
	  nodes[2] = i * grid + ((j+1)%grid);
	  nodes[3] = ((i+1)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  // _ _
	  //  |
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = ((i+1)%grid) * grid + j;
	  nodes[3] = i * grid + ((j-1+grid)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  // _|
	  //  |
	  nodes[0] = i * grid + j;
	  nodes[1] = ((i+1)%grid) * grid + j;
	  nodes[2] = i * grid + ((j-1+grid)%grid);
	  nodes[3] = ((i-1+grid)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;
	}
      }
    }
    //-------------------------------------------------------------------------
    // second
    {
      mc.num_switch_structures[2] = 4 * num_grid_nodes;
      mc.switch_structures[2] = new MC_switch_structure[4 * num_grid_nodes];
      MC_switch_structure * next_struct = mc.switch_structures[2];

      int nodes[4];

      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  //   _
	  // _|
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = ((i-1+grid)%grid) * grid + ((j+1)%grid);
	  nodes[3] = ((i-1+grid)%grid) * grid + ((j+2)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  // _
	  //  |_
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = ((i+1)%grid) * grid + ((j+1)%grid);
	  nodes[3] = ((i+1)%grid) * grid + ((j+2)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  // |_
	  //   |
	  nodes[0] = i * grid + j;
	  nodes[1] = ((i+1)%grid) * grid + j;
	  nodes[2] = ((i+1)%grid) * grid + ((j+1)%grid);
	  nodes[3] = ((i+2)%grid) * grid + ((j+1)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;

	  //  _|
	  // |
	  nodes[0] = i * grid + j;
	  nodes[1] = ((i+1)%grid) * grid + j;
	  nodes[2] = ((i+1)%grid) * grid + ((j-1+grid)%grid);
	  nodes[3] = ((i+2)%grid) * grid + ((j-1+grid)%grid);
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;
	}
      }
    }
    //-------------------------------------------------------------------------
    // third
    {
      mc.num_switch_structures[3] = num_grid_nodes;
      mc.switch_structures[3] = new MC_switch_structure[num_grid_nodes];
      MC_switch_structure * next_struct = mc.switch_structures[3];

      int nodes[4];

      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  // the square is (i,j), (i,j+1), (i+1,j+1), (i+1,j)
	  //  _
	  // |_|
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = ((i+1)%grid) * grid + ((j+1)%grid);
	  nodes[3] = ((i+1)%grid) * grid + j;
	  MC_fill_structure(mc, *next_struct, 4, nodes);
	  if (next_struct->num_neighbors != 8 + (has_extra_node ? 4 : 0))
	    abort();
	  ++next_struct;
	}
      }
    }

    //=========================================================================

    // a 6-node connected structures

    {
      mc.num_switch_structures[4] = 2 * num_grid_nodes;
      mc.switch_structures[4] = new MC_switch_structure[2 * num_grid_nodes];
      MC_switch_structure * next_struct = mc.switch_structures[4];

      int nodes[6];

      for (i = 0; i < grid; ++i) {
	for (j = 0; j < grid; ++j) {
	  //  _ _
	  // |_|_|
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = i * grid + ((j+2)%grid);
	  nodes[3] = ((i+1)%grid) * grid + j;
	  nodes[4] = ((i+1)%grid) * grid + ((j+1)%grid);
	  nodes[5] = ((i+1)%grid) * grid + ((j+2)%grid);
	  MC_fill_structure(mc, *next_struct, 6, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 6 : 0))
	    abort();
	  ++next_struct;

	  //  _ 
	  // |_|
	  // |_|
	  nodes[0] = i * grid + j;
	  nodes[1] = i * grid + ((j+1)%grid);
	  nodes[2] = ((i+1)%grid) * grid + j;
	  nodes[3] = ((i+1)%grid) * grid + ((j+1)%grid);
	  nodes[4] = ((i+2)%grid) * grid + j;
	  nodes[5] = ((i+2)%grid) * grid + ((j+1)%grid);
	  MC_fill_structure(mc, *next_struct, 6, nodes);
	  if (next_struct->num_neighbors != 10 + (has_extra_node ? 6 : 0))
	    abort();
	  ++next_struct;
	}
      }
    }
  }

  if (tm.tm_par.entry(MC_tm_par::FeasSolFile).length() > 0) {
     std::ifstream solfile(tm.tm_par.entry(MC_tm_par::FeasSolFile).c_str());
     if (! solfile) {
	throw BCP_fatal_error("Can't open feas solution file!\n");
     }
     int* s = new int[mc.num_nodes];
     for (i = 0; i < mc.num_nodes; ++i) {
	solfile.getline(line, 999);
	sscanf(line, "%i", s+i);
     }
     solfile.close();
     mc.feas_sol = new MC_feas_sol(mc.num_nodes, s, mc.num_edges, mc.edges);
     printf("\nMC: value of input solution: %.0f\n\n", mc.feas_sol->cost);
  }
}
