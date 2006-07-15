#include "MCF_var.hpp"

//#############################################################################

void MCF_var::pack(BCP_buffer& buf) const
{
    buf.pack(commodity);
    int numarcs = flow.getNumElements();
    buf.pack(flow.getIndices(), numarcs);
    buf.pack(flow.getElements(), numarcs);
    buf.pack(weight);
}

/*---------------------------------------------------------------------------*/

MCF_var::MCF_var(BCP_buffer& buf) :
    // we don't know the onj coeff (weight) yet, so temporarily set it to 0
    BCP_var_algo(BCP_ContinuousVar, 0, 0, 1)
{
    buf.unpack(commodity);
    int numarcs;
    int* ind;
    double* val;
    buf.unpack(ind, numarcs);
    buf.unpack(val, numarcs);
    flow.assignVector(numarcs, ind, val, false /*don't test for duplicates*/);
    buf.unpack(weight);
    set_obj(weight);
}

/*===========================================================================*/

void MCF_pack_var(const BCP_var_algo* var, BCP_buffer& buf)
{
    const MCF_var* v = dynamic_cast<const MCF_var*>(var);
    if (v) {
	v->pack(buf);
    }
}

/*---------------------------------------------------------------------------*/

BCP_var_algo* MCF_unpack_var(BCP_buffer& buf)
{
    return new MCF_var(buf);
}

