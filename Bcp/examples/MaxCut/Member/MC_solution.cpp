// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cfloat>
#include <cstdio>
#include <fstream>

#include "CoinHelperFunctions.hpp"

#include "BCP_string.hpp"
#include "BCP_buffer.hpp"
#include "BCP_vector.hpp"

#include "MC.hpp"
#include "MC_solution.hpp"

#define MC_PRINT_ITER_COUNT 0

#define MC_TEST_COST_AFTER_EVERY_SWITCH 0

//#############################################################################

BCP_buffer&
MC_solution::pack(BCP_buffer& buf) const {
  buf.pack(cost).pack(sig);
  return buf;
}

//#############################################################################

BCP_buffer&
MC_solution::unpack(BCP_buffer& buf) {
  buf.unpack(cost).unpack(sig);
  return buf;
}

//#############################################################################

MC_solution&
MC_solution::operator=(const MC_solution& sol) {
  cost = sol.cost;
  sig = sol.sig;
  return *this;
}

//#############################################################################

double
MC_solution::compute_cost(const int m, const MC_graph_edge* edges)
{
  cost = 0.0;
  for (int i = 0; i < m; ++i) {
    const MC_graph_edge& edge = edges[i];
    if (sig[edge.tail] != sig[edge.head]) {
      cost += edge.cost;
    }
  }
  return cost;
}

//#############################################################################

static inline double
MC_switch_cost(const MC_adjacency_entry* adj_list, const int degree,
	       const int* sig, const int this_sig)
{
  double sum = 0;
  for (int j = degree - 1; j >= 0; --j) {
#if 1
    // Because of pipelined processors, this is probably much faster...
    sum += sig[adj_list[j].neighbor] * this_sig * adj_list[j].cost;
#else
    if (sig[adj_list[j].neighbor] == this_sig) {
      sum += adj_list[j].cost;
    } else {
      sum -= adj_list[j].cost;
    }
#endif
  }
  return sum;
}

//#############################################################################

double
MC_solution::switch_improve(const MC_problem& mc, const int maxiter)
{
  const int n = mc.num_nodes;
  bool flag = true;
  int i, iter;
  double newcost = cost;
  for (iter = 0; iter < maxiter && flag; ++iter) {
    flag = false;
    for (i = 0; i < n; ++i) {
      const double sum = MC_switch_cost(mc.nodes[i].adj_list,
					mc.nodes[i].degree,
					sig.begin(), sig[i]);
      if (sum < 0) {
	sig[i] *= -1;
	newcost += sum;
	flag = true;
#if MC_TEST_COST_AFTER_EVERY_SWITCH
	compute_cost(mc.num_edges, mc.edges);
	if (cost != newcost)
	  abort();
#endif
      }
    }
  }
#if MC_PRINT_ITER_COUNT
  printf("MC_solution: switch_improve: %i.\n", iter);
#endif

  compute_cost(mc.num_edges, mc.edges);
  if (cost != newcost)
    abort();

  return cost;
}


//#############################################################################
// This routine must be invoked after switch_improve(), since to speed things
// up we assume that no single node switch can help, i.e., MC_switch_cost() is
// non-negative for all nodes.

double
MC_solution::edge_switch_improve(const MC_problem& mc, const int maxiter)
{
  const int m = mc.num_edges;
  bool flag = true;
  int k, iter;
  double newcost = cost;
  const MC_graph_edge* edges = mc.edges;

  for (iter = 0; iter < maxiter && flag; ++iter) {
    flag = false;
    for (k = 0; k < m; ++k) {
      const int i = edges[k].tail;
      const int j = edges[k].head;
      const double costk = 2 * sig[i] * sig[j] * edges[k].cost;
      // we assume that no single node switch can help at this point ==>
      // The MC_switch_cost values will be non-negative ==> costk > 0 is a
      // necessary condition to find improvement.
      if (costk <= 0.0)
	continue;
      const double sum = 
	MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
		       sig.begin(), sig[i]) +
	MC_switch_cost(mc.nodes[j].adj_list, mc.nodes[j].degree,
		       sig.begin(), sig[j]) -
	costk;
      if (sum < 0) {
	sig[i] *= -1;
	sig[j] *= -1;
	newcost += sum;
	flag = true;
#if MC_TEST_COST_AFTER_EVERY_SWITCH
	compute_cost(mc.num_edges, mc.edges);
	if (cost != newcost)
	  abort();
#endif
      }
    }
  }
#if MC_PRINT_ITER_COUNT
  printf("MC_solution: edge_switch_improve: %i.\n", iter);
#endif

  compute_cost(mc.num_edges, mc.edges);
  if (cost != newcost)
    abort();

  return cost;
}

//#############################################################################
// This routine must be invoked after switch_improve(), since to speed things
// up we assume that no single node switch can help, i.e., MC_switch_cost() is
// non-negative for all nodes.

double
MC_solution::ising_with_external_edge_switch_improve(const MC_problem& mc,
						     const int maxiter)
{
  const int m = mc.num_edges;
  const int grid_edge_num = m - mc.num_nodes + 1;
  bool flag = true;
  int k, iter;
  double newcost = cost;
  const MC_graph_edge* edges = mc.edges;

  const int plus_node = mc.num_nodes - 1;
  const MC_adjacency_entry* plus_node_adj_list = mc.nodes[plus_node].adj_list;

  for (iter = 0; iter < maxiter && flag; ++iter) {
    flag = false;
    // First try to switch edges not going to the extra node
    for (k = 0; k < grid_edge_num; ++k) {
      const int i = edges[k].tail;
      const int j = edges[k].head;
      const double costk = 2 * sig[i] * sig[j] * edges[k].cost;
      // we assume that no single node switch can help at this point ==>
      // The MC_switch_cost values will be non-negative ==> costk > 0 is a
      // necessary condition to find improvement.
      if (costk <= 0.0)
	continue;
      const double sum = 
	MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
		       sig.begin(), sig[i]) +
	MC_switch_cost(mc.nodes[j].adj_list, mc.nodes[j].degree,
		       sig.begin(), sig[j]) -
	costk;
      if (sum < 0) {
	sig[i] *= -1;
	sig[j] *= -1;
	newcost += sum;
	flag = true;
#if MC_TEST_COST_AFTER_EVERY_SWITCH
	compute_cost(mc.num_edges, mc.edges);
	if (cost != newcost)
	  abort();
#endif
      }
    }

    // Now try to switch edges going to the extra node
    double switch_cost_of_plus_node =
      MC_switch_cost(plus_node_adj_list, mc.nodes[plus_node].degree,
		     sig.begin(), sig[plus_node]);
    for ( ; k < m; ++k) {
      const int i = edges[k].tail;
      const double costk = 2 * sig[i] * sig[plus_node] * edges[k].cost;
      // we assume that no single node switch can help at this point ==>
      // The MC_switch_cost values will be non-negative ==> costk > 0 is a
      // necessary condition to find improvement.
      if (costk <= 0.0)
	continue;
      const double sum = 
	MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
		       sig.begin(), sig[i]) +
	switch_cost_of_plus_node -
	costk;
      if (sum < 0) {
	sig[i] *= -1;
	sig[plus_node] *= -1;
	switch_cost_of_plus_node = - switch_cost_of_plus_node + costk;
	newcost += sum;
	flag = true;
#if MC_TEST_COST_AFTER_EVERY_SWITCH
	compute_cost(mc.num_edges, mc.edges);
	if (cost != newcost)
	  abort();
#endif
      }
    }
  }
#if MC_PRINT_ITER_COUNT
  printf("MC_solution: edge_switch_improve: %i.\n", iter);
#endif

  compute_cost(mc.num_edges, mc.edges);
  if (cost != newcost)
    abort();

  return cost;
}

//#############################################################################

double
MC_solution::structure_switch_improve(const MC_problem& mc,
				      const int struct_ind, const int maxiter)
{
  if (mc.num_switch_structures[struct_ind] == 0)
    return cost;

  const int s = mc.num_switch_structures[struct_ind];
  const MC_switch_structure* sw_structs = mc.switch_structures[struct_ind];
  bool flag = true;
  int i, k, iter;
  double sum;
  double newcost = cost;
  const MC_graph_edge* edges = mc.edges;

  for (iter = 0; iter < maxiter && flag; ++iter) {
    flag = false;
    for (k = 0; k < s; ++k) {
      const MC_switch_structure& sw_struct = sw_structs[k];
      const int * nb = sw_struct.neighbors;
      sum = 0.0;
      for (i = sw_struct.num_neighbors - 1; i >= 0; --i) {
	const MC_graph_edge& e = edges[nb[i]];
	sum += sig[e.tail] * sig[e.head] * e.cost;
      }
      if (sum < 0) {
	const int * nodes = sw_struct.nodes;
	for (i = sw_struct.num_nodes - 1; i >= 0; --i) {
	  sig[nodes[i]] *= -1;
	}
	newcost += sum;
	flag = true;
#if MC_TEST_COST_AFTER_EVERY_SWITCH
	compute_cost(mc.num_edges, mc.edges);
	if (cost != newcost)
	  abort();
#endif
      }
    }
  }
#if MC_PRINT_ITER_COUNT
  printf("MC_solution: sw_struct_improve: %i.\n", iter);
#endif

  compute_cost(mc.num_edges, mc.edges);
  if (cost != newcost)
    abort();

  return cost;
}
	
//#############################################################################

double
MC_solution::lk_switch_improve(const MC_problem& mc, const int maxiter)
{
  const int n = mc.num_nodes;
  int i, start, end, iter = 0;
  double sum;
  double best_sum;

  start = 0;
  for (iter = 0; iter < 1000 * maxiter; ++iter) {
    best_sum = 0.0;
    sum = 0.0;

    // Find the first node that does improve
    for (i = start; i < n; ++i) {
      sum =  MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
			    sig.begin(), sig[i]);
      if (sum < 0.0)
	break;
    }
    if (sum == 0.0) {
      for (i = 0; i < start; ++i) {
	sum = MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
			      sig.begin(), sig[i]);
	if (sum < 0.0)
	  break;
      }
    }

    if (sum >= 0.0) // No individual node can be switched
      break;

    sig[i] *= -1;
    start = i;
    best_sum = sum;
    end = i;
    
    // Do the Lin-Kerninghan thingy  
    for (i = start+1; i < n; ++i) {
      sum +=  MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
			     sig.begin(), sig[i]);
      if (sum >= 0)
	break;
      sig[i] *= -1;
      if (sum < best_sum) {
	best_sum = sum;
	end = i;
      }
    }
    if (sum < 0) {
      for (i = 0; i < start; ++i) {
	sum += MC_switch_cost(mc.nodes[i].adj_list, mc.nodes[i].degree,
			      sig.begin(), sig[i]);
	if (sum >= 0)
	  break;
	sig[i] *= -1;
	if (sum < best_sum) {
	  best_sum = sum;
	  end = i;
	}
      }
    }

    // Now best_sum must be < 0 and we got to switch the [start,end] part
    // only, i.e., we got to reverse the switching of (end,i).
    // Note that i cannot be start since when everything is switched then sum
    // is 0.0.
    if (end > i) {
      for (--i; i >= 0; --i)
	sig[i] *= -1;
      i = n;
    }
    for (--i; i > end; --i)
      sig[i] *= -1;

    const double newcost = cost + best_sum;
    compute_cost(mc.num_edges, mc.edges);
    if (cost != newcost)
      abort();

    start = (end + 1) %n;
  }
#if MC_PRINT_ITER_COUNT
  printf("MC_solution: lk_switch_improve: %i.\n", iter);
#endif

  return cost;
}

//#############################################################################

void
MC_solution::display(const BCP_string& fname) const
{
  const int num_nodes = sig.size();
  int objval = static_cast<int>(objective_value());

  printf("MC_solution:\n");
  printf("%i\n", objval);
  for (int i = 0; i < num_nodes; ++i) {
    printf("%2i\n", sig[i]);
  }
  printf("\n");

  if (fname.length() != 0) {
    std::ofstream ofile(fname.c_str());
    ofile << "MC_solution:\n";
    ofile << objval << "\n";
    for (int i = 0; i < num_nodes; ++i) {
      ofile << sig[i] << "\n";
    }
  }
}

//#############################################################################

MC_solution::MC_solution(const BCP_vec<int>& sign,
			 const MC_problem& mc,
			 const int heurswitchround,
			 const bool do_edge_switch_heur,
			 const int struct_switch_heur) :
  sig(sign)
{
  const int m = mc.num_edges;
  const MC_graph_edge* edges = mc.edges;
  const int num_struct_type = mc.num_structure_type;

  const double percentage[3] = {0.05, 0.01, 0.002};

  BCP_vec<int> bestsig;
  double bestcost = DBL_MAX;

  int i; 
  bool do_plain_switch = true;
  bool do_edge_switch = true;
  bool* do_structure_switch = new bool[num_struct_type];
  bool do_switch = true;

  printf("MC: Heur Sol Improvement:  ");
  for (int k = 0; k < 1; ++k) {
    compute_cost(m, edges);

    BCP_vec<double> obj;
    BCP_vec<char> type;

    obj.push_back(objective_value());
    type.push_back('o');

    do_plain_switch = true;
    do_edge_switch = true;
    CoinFillN(do_structure_switch, num_struct_type, true);
    do_switch = true;

    while (do_switch) {
      if (do_plain_switch) {
	switch_improve(mc, heurswitchround);
	if (obj.back() > objective_value()) {
	  obj.push_back(objective_value());
	  type.push_back('P');
	  // improved, got to do the others, too
	  do_edge_switch = true;
	  CoinFillN(do_structure_switch, num_struct_type, true);
	}
	do_plain_switch = false;
      }
      if (do_edge_switch) {
	if (do_edge_switch_heur) {
	  if (!mc.ising_triangles)
	    edge_switch_improve(mc, heurswitchround);
	  else 
	    ising_with_external_edge_switch_improve(mc, heurswitchround);
	  if (obj.back() > objective_value()) {
	    obj.push_back(objective_value());
	    type.push_back('E');
	    // improved, got to do the others, too
	    do_plain_switch = true;
	    CoinFillN(do_structure_switch, num_struct_type, true);
	  }
	}
	do_edge_switch = false;
      }
      for (i = 0; i < num_struct_type; ++i) {
	if (do_structure_switch[i]) {
	  if ((struct_switch_heur & (1 << i)) != 0) {
	    structure_switch_improve(mc, i, heurswitchround);
	    if (obj.back() > objective_value()) {
	      obj.push_back(objective_value());
	      type.push_back('0'+i);
	      // improved, got to do the others, too
	      do_plain_switch = true;
	      do_edge_switch = true;
	      CoinFillN(do_structure_switch, num_struct_type, true);
	    }
	  }
	}
	do_structure_switch[i] = false;
      }

      do_switch = do_plain_switch || do_edge_switch;
      for (i = 0; i < num_struct_type; ++i)
	do_switch |= do_structure_switch[i];
    }
    printf("    %.0f", obj[0]);
    const int rounds = obj.size();
    for (i = 1; i < rounds; ++i) {
      printf(" / %c %.0f", type[i], obj[i]);
    }
    printf("\n");

    if (cost < bestcost) {
      bestsig = sig;
      bestcost = cost;
    }

    for (i = sig.size() - 1; i >= 0; --i) {
      if (CoinDrand48() < percentage[k])
	sig[i] = -sig[i];
    }
  }

  sig = bestsig;
  cost = bestcost;

  delete[] do_structure_switch;
}
