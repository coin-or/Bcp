#include "BCP_solution.hpp"
#include "MCF_tm.hpp"

using std::make_pair;

//#############################################################################

void MCF_tm::initialize_core(BCP_vec<BCP_var_core*>& vars,
			     BCP_vec<BCP_cut_core*>& cuts,
			     BCP_lp_relax*& matrix)
{
    // there is nothing in the core
}

/*---------------------------------------------------------------------------*/

void MCF_tm::create_root(BCP_vec<BCP_var*>& added_vars,
			 BCP_vec<BCP_cut*>& added_cuts,
			 BCP_user_data*& user_data,
			 BCP_pricing_status& pricing_status)
{
    // create a column (a flow) for each commodity
    if (par.entry(MCF_par::AddDummySourceSinkArcs)) {
	// In this case just send all the demand through the artificial arc
	// for each commodity. That'll get the algorithm started.
	for (int i = 0; i < data.numcommodities; ++i) {
	    int ind = data.numarcs-data.numcommodities+i;
	    double val = data.commodities[i].demand;
	    double w = data.commodities[i].demand * data.arcs[ind].weight;
	    added_vars.push_back(new MCF_var(i, CoinPackedVector(1, &ind, &val,
								 false), w));
	}
    } else {
	// Generate a flow for each commodity independently
	// *** excercise ***

	// Think of the consequences of not having the artificial arcs. After
	// branching the master problem may become infeasible, subproblems can
	// be infeasible, etc...
    }
}

/*---------------------------------------------------------------------------*/

void MCF_tm::display_feasible_solution(const BCP_solution* sol)
{
    const BCP_solution_generic* gsol =
	dynamic_cast<const BCP_solution_generic*>(sol);
    const BCP_vec<BCP_var*>& vars = gsol->_vars;
    for (int i = vars.size()-1; i >= 0; --i) {
	const MCF_var* v = dynamic_cast<const MCF_var*>(vars[i]);
	printf("Flow of commodity %i:\n", v->commodity);
	const CoinPackedVector& f = v->flow;
	const int* ind = f.getIndices();
	const double* val = f.getElements();
	for (int j = 0; j < f.getNumElements(); ++j) {
	    printf("    arc %6i  ( %5i -> %5i )     flow %i\n",
		   ind[j], data.arcs[ind[j]].tail, data.arcs[ind[j]].head,
		   static_cast<int>(val[j]));
	}
    }
}

/*---------------------------------------------------------------------------*/

void MCF_tm::pack_module_data(BCP_buffer& buf, BCP_process_t ptype)
{
    switch (ptype) {
    case BCP_ProcessType_LP:
	par.pack(buf);
	data.pack(buf);
    default:
	throw BCP_fatal_error("In this example we have only an LP process!\n");
    }
}		
