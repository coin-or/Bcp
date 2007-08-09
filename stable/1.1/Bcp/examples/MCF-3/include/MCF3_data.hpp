#ifndef MCF3_data_hpp
#define MCF3_data_hpp

#include <iostream>
#include "BCP_buffer.hpp"
#include "BCP_USER.hpp"

//#############################################################################

// This structure holds the input data

class MCF3_data {
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
    MCF3_data() :
	arcs(NULL), commodities(NULL),
	numarcs(0), numnodes(0), numcommodities(0) {}

    ~MCF3_data() {
	delete[] arcs;
	delete[] commodities;
	delete[] problem_name;
    }

    int readDimacsFormat(std::istream& s, bool addDummyArcs);
    void pack(BCP_buffer& buf) const;
    void unpack(BCP_buffer& buf);
};

//#############################################################################

class MCF3_branch_decision
{
public:
    int arc_index;
    int lb;
    int ub;
public: 
    MCF3_branch_decision() : arc_index(-1), lb(0), ub(0) {}
    MCF3_branch_decision(int i, int l, int u) : arc_index(i), lb(l), ub(u) {}
    void pack(BCP_buffer& buf) const;
    void unpack(BCP_buffer& buf);
};

//#############################################################################

class MCF3_user : public BCP_user_data {
private:
    MCF3_user& operator=(const MCF3_user& rhs);

public:
    int numCommodities;
    std::vector<MCF3_branch_decision>* branch_history;

public:
    MCF3_user(int numComm=-1) : numCommodities(numComm), branch_history(NULL) {
	if (numComm > 0) {
	    branch_history = new std::vector<MCF3_branch_decision>[numComm];
	}
    }
    MCF3_user(const MCF3_user& rhs) : numCommodities(0), branch_history(NULL) {
	numCommodities = rhs.numCommodities;
	branch_history = new std::vector<MCF3_branch_decision>[numCommodities];
	for (int i = 0; i < numCommodities; ++i) {
	    branch_history[i] = rhs.branch_history[i];
	}
    }
    ~MCF3_user() {
	delete[] branch_history;
    }
    void pack(BCP_buffer& buf) const;
    void unpack(BCP_buffer& buf);
};

#endif
