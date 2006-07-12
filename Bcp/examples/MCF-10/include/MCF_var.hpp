#ifndef MCF_var_hpp
#define MCF_var_hpp

#include "CoinPackedVector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"

/*---------------------------------------------------------------------------*/

// Here are two kind of variables. The first is a "real" variable, i.e., it
// describes a column of the master problem, that is a flow for a
// commodity. The second is a "fake" variable. It simply holds information on
// the branching decision so that in the subproblems we can make the
// appropriate restrictions on the flow values.

class MCF_var : public BCP_var_algo {
public:
    int commodity;
    CoinPackedVector flow;
    double weight;

public:
    MCF_var(int com, const CoinPackedVector& f, double w) :
	BCP_var_algo(BCP_ContinuousVar, w, 0, 1),
	commodity(com), flow(false), weight(w)
    {
	new (&flow) CoinPackedVector(f.getNumElements(),
				     f.getIndices(), f.getElements(), false);
    }
    MCF_var(BCP_buffer& buf);
    ~MCF_var() {}

    void pack(BCP_buffer& buf) const;
};

void MCF_pack_var(const BCP_var_algo* var, BCP_buffer& buf);
BCP_var_algo* MCF_unpack_var(BCP_buffer& buf);

#endif
