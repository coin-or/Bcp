#ifndef MCF1_var_hpp
#define MCF1_var_hpp

#include "CoinPackedVector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"

/*---------------------------------------------------------------------------*/

// Here are two kind of variables. The first is a "real" variable, i.e., it
// describes a column of the master problem, that is a flow for a
// commodity. The second is a "fake" variable. It simply holds information on
// the branching decision so that in the subproblems we can make the
// appropriate restrictions on the flow values.

class MCF1_var : public BCP_var_algo {
public:
    int commodity;
    CoinPackedVector flow;
    double weight;

public:
    MCF1_var(int com, const CoinPackedVector& f, double w) :
	BCP_var_algo(BCP_ContinuousVar, w, 0, 1),
	commodity(com), flow(false), weight(w)
    {
	new (&flow) CoinPackedVector(f.getNumElements(),
				     f.getIndices(), f.getElements(), false);
    }
    MCF1_var(BCP_buffer& buf);
    ~MCF1_var() {}

    void pack(BCP_buffer& buf) const;
};

/*---------------------------------------------------------------------------*/

class MCF1_branching_var : public BCP_var_algo {
public:
    int commodity;
    int arc_index;
    int lb_child0;
    int ub_child0;
    int lb_child1;
    int ub_child1;

public:
    MCF1_branching_var(int comm, int ind, int lb0, int ub0, int lb1, int ub1) :
	BCP_var_algo(BCP_BinaryVar, 0, 0, 1),
	commodity(comm), arc_index(ind),
	lb_child0(lb0), ub_child0(ub0), lb_child1(lb1), ub_child1(ub1) {}
    MCF1_branching_var(BCP_buffer& buf);
    ~MCF1_branching_var() {}

    void pack(BCP_buffer& buf) const;
};

void MCF1_pack_var(const BCP_var_algo* var, BCP_buffer& buf);
BCP_var_algo* MCF1_unpack_var(BCP_buffer& buf);

#endif
