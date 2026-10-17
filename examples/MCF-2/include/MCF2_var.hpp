#ifndef MCF2_var_hpp
#define MCF2_var_hpp

#include "CoinPackedVector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"

/*---------------------------------------------------------------------------*/

// Here are two kind of variables. The first is a "real" variable, i.e., it
// describes a column of the master problem, that is a flow for a
// commodity. The second is a "fake" variable. It simply holds information on
// the branching decision so that in the subproblems we can make the
// appropriate restrictions on the flow values.

// For the real variables the upper bound is 1, but we set it to 2 so there
// will never be a variable out of basis at its upper bound. That would kill
// column generation. The convexity constraint will take care of keeping these
// variables under 1.

class MCF2_var : public BCP_var_algo {
public:
    int commodity;
    CoinPackedVector flow;
    double weight;

public:
    MCF2_var(int com, const CoinPackedVector& f, double w) :
	BCP_var_algo(BCP_ContinuousVar, w, 0, 2),
	commodity(com),
	flow(f.getNumElements(), f.getIndices(), f.getElements(), false),
	weight(w) {}
    MCF2_var(BCP_buffer& buf);
    ~MCF2_var() {}

    void pack(BCP_buffer& buf) const;
};

/*---------------------------------------------------------------------------*/

class MCF2_branching_var : public BCP_var_algo {
public:
    int commodity;
    int arc_index;
    int lb_child0;
    int ub_child0;
    int lb_child1;
    int ub_child1;

public:
    MCF2_branching_var(int comm, int ind, int lb0, int ub0, int lb1, int ub1) :
	BCP_var_algo(BCP_BinaryVar, 0, 0, 1),
	commodity(comm), arc_index(ind),
	lb_child0(lb0), ub_child0(ub0), lb_child1(lb1), ub_child1(ub1) {}
    MCF2_branching_var(BCP_buffer& buf);
    ~MCF2_branching_var() {}

    void pack(BCP_buffer& buf) const;
};

void MCF2_pack_var(const BCP_var_algo* var, BCP_buffer& buf);
BCP_var_algo* MCF2_unpack_var(BCP_buffer& buf);

#endif
