#ifndef MCF3_var_hpp
#define MCF3_var_hpp

#include "CoinPackedVector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"

/*---------------------------------------------------------------------------*/

// A var describing a column of the master problem, that is a flow for a
// commodity. The second is a "fake" variable. It simply holds information on
// the branching decision so that in the subproblems we can make the
// appropriate restrictions on the flow values.

class MCF3_var : public BCP_var_algo {
public:
    int commodity;
    CoinPackedVector flow;
    double weight;

public:
    MCF3_var(int com, const CoinPackedVector& f, double w) :
	BCP_var_algo(BCP_ContinuousVar, w, 0, 1),
	commodity(com), flow(false), weight(w)
    {
	new (&flow) CoinPackedVector(f.getNumElements(),
				     f.getIndices(), f.getElements(), false);
    }
    MCF3_var(BCP_buffer& buf);
    ~MCF3_var() {}

    void pack(BCP_buffer& buf) const;
};

void MCF3_pack_var(const BCP_var_algo* var, BCP_buffer& buf);
BCP_var_algo* MCF3_unpack_var(BCP_buffer& buf);

#endif
