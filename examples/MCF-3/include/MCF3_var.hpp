#ifndef MCF3_var_hpp
#define MCF3_var_hpp

#include "CoinPackedVector.hpp"
#include "BCP_buffer.hpp"
#include "BCP_var.hpp"

/*---------------------------------------------------------------------------*/

// A var describing a column of the master problem, that is a flow for a
// commodity. 

// For the variables the upper bound is 1, but we set it to 2 so there
// will never be a variable out of basis at its upper bound. That would kill
// column generation. The convexity constraint will take care of keeping these
// variables under 1.

class MCF3_var : public BCP_var_algo {
public:
    int commodity;
    CoinPackedVector flow;
    double weight;

public:
    MCF3_var(int com, const CoinPackedVector& f, double w) :
	BCP_var_algo(BCP_ContinuousVar, w, 0, 2),
	commodity(com),
	flow(f.getNumElements(), f.getIndices(), f.getElements(), false),
	weight(w) {}
    MCF3_var(BCP_buffer& buf);
    ~MCF3_var() {}

    void pack(BCP_buffer& buf) const;
};

void MCF3_pack_var(const BCP_var_algo* var, BCP_buffer& buf);
BCP_var_algo* MCF3_unpack_var(BCP_buffer& buf);

#endif
