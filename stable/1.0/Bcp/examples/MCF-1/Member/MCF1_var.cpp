#include "MCF1_var.hpp"

//#############################################################################

void MCF1_var::pack(BCP_buffer& buf) const
{
    buf.pack(commodity);
    int numarcs = flow.getNumElements();
    buf.pack(flow.getIndices(), numarcs);
    buf.pack(flow.getElements(), numarcs);
    buf.pack(weight);
}

/*---------------------------------------------------------------------------*/

MCF1_var::MCF1_var(BCP_buffer& buf) :
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

void MCF1_branching_var::pack(BCP_buffer& buf) const
{
    buf.pack(commodity);
    buf.pack(arc_index);
    buf.pack(lb_child0);
    buf.pack(ub_child0);
    buf.pack(lb_child1);
    buf.pack(ub_child1);
}

/*---------------------------------------------------------------------------*/

MCF1_branching_var::MCF1_branching_var(BCP_buffer& buf) :
    BCP_var_algo(BCP_BinaryVar, 0, 0, 1)
{
    buf.unpack(commodity);
    buf.unpack(arc_index);
    buf.unpack(lb_child0);
    buf.unpack(ub_child0);
    buf.unpack(lb_child1);
    buf.unpack(ub_child1);
}

/*===========================================================================*/

void MCF1_pack_var(const BCP_var_algo* var, BCP_buffer& buf)
{
    const MCF1_var* v = dynamic_cast<const MCF1_var*>(var);
    if (v) {
	int type = 0;
	buf.pack(type);
	v->pack(buf);
	return;
    }
    const MCF1_branching_var* bv = dynamic_cast<const MCF1_branching_var*>(var);
    if (bv) {
	int type = 1;
	buf.pack(type);
	bv->pack(buf);
	return;
    }
}

/*---------------------------------------------------------------------------*/

BCP_var_algo* MCF1_unpack_var(BCP_buffer& buf)
{
    int type;
    buf.unpack(type);
    switch (type) {
    case 0: return new MCF1_var(buf);
    case 1: return new MCF1_branching_var(buf);
    default: throw BCP_fatal_error("MCF1_unpack_var: bad var type");
    }
    return NULL; // fake return
}

