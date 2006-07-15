#ifndef MCF_data_hpp
#define MCF_data_hpp

#include <iostream>
#include "BCP_buffer.hpp"

//#############################################################################

// This structure holds the input data

class MCF_data {
public:
    struct arc {
	int tail;
	int head;
	int lb;
	int ub;
	double weight;
    };
    struct commodity {
	int source;
	int sink;
	int demand;
    };
    char* problem_name;
    arc* arcs;
    commodity* commodities;
    int numarcs;
    int numnodes;
    int numcommodities;

public:
    MCF_data() :
	arcs(NULL), commodities(NULL),
	numarcs(0), numnodes(0), numcommodities(0) {}

    ~MCF_data() {
	delete[] arcs;
	delete[] commodities;
	delete[] problem_name;
    }

    int readDimacsFormat(std::istream& s, bool addDummyArcs);
    void pack(BCP_buffer& buf) const;
    void unpack(BCP_buffer& buf);
};

//#############################################################################

class MCF_branch_decision
{
public:
    int arc_index;
    int lb;
    int ub;
public: 
    MCF_branch_decision() : arc_index(-1), lb(0), ub(0) {}
    MCF_branch_decision(int i, int l, int u) : arc_index(i), lb(l), ub(u) {}
};

#endif
