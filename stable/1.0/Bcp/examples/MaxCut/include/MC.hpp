// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_H
#define _MC_H

#include "BCP_vector.hpp"

class BCP_buffer;

class MC_adjacency_entry {
public:
    int neighbor;
    int index;
    double cost;
    MC_adjacency_entry() : neighbor(-1), cost(0.0) {}
    MC_adjacency_entry(int n, double c) : neighbor(n), cost(c) {}
};

class MC_graph_node {
public:
    int degree;
    MC_adjacency_entry* adj_list;
    MC_graph_node() : degree(-1), adj_list(0) {}
    MC_graph_node(int d, MC_adjacency_entry* l) : degree(d), adj_list(l) {}
};

class MC_graph_edge {
public:
    int tail;
    int head;
    double cost;
public:
    MC_graph_edge() : tail(-1), head(-1), cost(0.0) {}
    MC_graph_edge(int t, int h, double c) : tail(t), head(h), cost(c) {}
};

class MC_switch_structure {
private:
    MC_switch_structure(const MC_switch_structure&);
    MC_switch_structure& operator=(const MC_switch_structure&);
public:
    int num_nodes;
    int num_neighbors;
    int * nodes;
    int * neighbors;
    MC_switch_structure() :
	num_nodes(0), num_neighbors(0), nodes(0), neighbors(0) {}
    ~MC_switch_structure() {
	delete[] nodes;
	// neighbors is the second part of the nodes array
    }
};

class MC_feas_sol {
public:
    double cost;
    int num_nodes;
    int* sign;
    int num_edges;
    double* value;
    MC_feas_sol(const int nnodes, int*& s,
		const int nedges, const MC_graph_edge* edges) :
	num_nodes(nnodes),
	sign(s),
	num_edges(nedges),
	value(new double[nedges])
    {
	cost = 0;
	for (int i = 0; i < nedges; ++i) {
	    value[i] = sign[edges[i].tail] == sign[edges[i].head] ? 0 : 1;
	    cost += value[i] * edges[i].cost;
	}
	s = 0;
    }
    MC_feas_sol(const double c,
		const int nnodes, int*& s,
		const int nedges, double*& v) :
	cost(c), num_nodes(nnodes), sign(s), num_edges(nedges), value(v) {
	s = 0;
	v = 0;
    }
    ~MC_feas_sol() {
	delete[] value;
	delete[] sign;
    }
};

class MC_problem {
public:
    // This data relates to the regular graph
    int num_nodes;
    int num_edges;
    // These arrays are constant arrays, better have them as real arrays
    // instead of BCP_vec's
    MC_graph_edge* edges;
    MC_graph_node* nodes;
    MC_adjacency_entry* all_adj_list;

    bool ising_problem;
    // These data members are used only if an Ising problem is solved
    double sum_edge_weight;
    double scaling_factor;

    // the squares of the grid
    int* ising_four_cycles;
    // the triangles if there is an external magnetic point
    int* ising_triangles;

    int num_structure_type;
    int * num_switch_structures;
    MC_switch_structure** switch_structures;

    // for testing we can read in a feas soln
    MC_feas_sol* feas_sol;

public:
    MC_problem() :
	num_nodes(0), num_edges(0), edges(0), nodes(0), all_adj_list(0),
	ising_problem(false), sum_edge_weight(0.0), scaling_factor(1.0),
	ising_four_cycles(0), ising_triangles(0),
	num_structure_type(0), num_switch_structures(0), switch_structures(0),
	feas_sol(0)
    {}
    ~MC_problem() {
	delete[] edges;
	delete[] nodes;
	delete[] all_adj_list;
	delete[] ising_four_cycles;
	delete[] ising_triangles;
	for (int i = 0; i < num_structure_type; ++i) {
	    delete[] switch_structures[i];
	}
	delete[] switch_structures;
	delete[] num_switch_structures;
	delete feas_sol;
    }

    void create_adj_lists();
    BCP_buffer& pack(BCP_buffer& buf);
    BCP_buffer& unpack(BCP_buffer& buf);
};

//#############################################################################

// A class to hold adjacency entries during shortest path computation

class MC_path_adj_entry {
public:
    int neighbor;
    int orig_edge;
    double cost;
    MC_path_adj_entry() : neighbor(-1), orig_edge(-1), cost(0.0) {}
};

// A class to hold nodes during shortest path computation

class MC_path_node {
public:
    bool processed;
    int degree;
    // the predecessor node in the shortest path tree
    int parent;
    // the edge (in the original edge list!) to the parent node
    int edge_to_parent;
    // distance from the root node
    double distance;
    MC_path_adj_entry* adj_list;

    MC_path_node() : processed(false), degree(-1), parent(-1),
		     edge_to_parent(-1), distance(-1.0), adj_list(0) {}
};

//#############################################################################

struct MC_path_node_ptr_compare {
    inline bool operator() (const MC_path_node* node0,
			    const MC_path_node* node1) {
	return node0->distance < node1->distance;
    }
};

#endif
