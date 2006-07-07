#include "CoinHelperFunctions.hpp"
#include "OsiClpSolverInterface.hpp"
#include "MCF_lp.hpp"

//#############################################################################

OsiSolverInterface* MCF_lp::initialize_solver_interface()
{
    return new OsiClpSolverInterface;
}

/*---------------------------------------------------------------------------*/

void MCF_lp::initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
					     const BCP_vec<BCP_cut*>& cuts,
					     const BCP_vec<BCP_obj_status>& var_status,
					     const BCP_vec<BCP_obj_status>& cut_status,
					     BCP_vec<int>& var_changed_pos,
					     BCP_vec<double>& var_new_bd,
					     BCP_vec<int>& cut_changed_pos,
					     BCP_vec<double>& cut_new_bd)
{
}

/*---------------------------------------------------------------------------*/

BCP_solution* MCF_lp::test_feasibility(const BCP_lp_result& lp_result,
				       const BCP_vec<BCP_var*>& vars,
				       const BCP_vec<BCP_cut*>& cuts)
{
    // Here we need to test whether the left-hand-side of each row is integer
    // or not.
    const double* lhs = lpres.lhs();
    const int numrows = cuts.size();
    int i;
    for (i = 0; i < numrows; ++i) {
	const double frac = fabs(lhs[i] - floor(lhs[i]) - 0.5);
	if (frac < 0.5-1e-6)
	    return NULL;
    }

    const double* x = lp_result.x();
    for (i = vars.size()-1; i >= 0; --i) {
	MCF_var* v = dynamic_cast<MCF_var*>(vars[i]);
	std::map<int,double>& f = flows[v->commodity];
	const int vsize = v->flow.getNumElements();
	const int* vind = v->flow.getIndices();
	const double* vval = v->flow.getElements();
	for (j = 0; j < vsize; ++j) {
	    f[vind[j]] += vval[j]*x[i];
	}
    }

    BCP_solution_generic* sol = new BCP_solution_generic;
    for (i = data.numcommodities-1; i >= 0; --i) {
	std::map<int,double>& f = flows[i];
	CoinPackedVector flow(false);
	double weight = 0;
	for (std::map<int,double>::iterator fi=f.begin(); fi!=f.end(); ++fi) {
	    flow.insert(fi->first, fi->second);
	    weight += fi->second * data.arcs[fi->first].weight;
	}
	sol->add_entry(new MCF_var(i, flow, weight), 1.0);
    }

    for (i = data.numcommodities-1; i >= 0; --i) {
	flows[i].clear();
    }

    return sol;
}

/*---------------------------------------------------------------------------*/

void
MCF_lp::generate_vars_in_lp(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    const bool before_fathom,
			    BCP_vec<BCP_var*>& new_vars,
			    BCP_vec<BCP_col*>& new_cols)
{
    // for each commodity generate a flow. do them in a random order and aply
    // dual discounting
    double* cost = new double[data.numarcs];
    const double* pi = lpres.pi();
    const double* nu = pi + data.numarcs;
    int i, j;
    for (i = data.numarcs-1; i >= 0; --i) {
	cost[i] = data.arcs[i].weight - pi[i];
    }
    cg_lp->setObjective(cost);

    // This will hold generated variables
    int* ind = new int[data.numarcs];
    double* val = new double[data.numarcs];
    int cnt = 0;

    for (i = data.numcommodities-1; i >= 0; --i) {
	const MCF_data::commodity& comm = data.commodities[i];
	cg_lp->setRowBounds(comm.source, -comm.demand, -comm.demand);
	cg_lp->setRowBounds(comm.sink, comm.demand, comm.demand);
	cg_lp->initialSolve();
	if (cg_lp->getObjValue() < nu[i] - 1e-8) {
	    // we have generated a column Create a var out of it. Round the
	    // double values while we are here, after all, they should be
	    // integer. there can only be some tiny roundoff error by the LP
	    // solver
	    const double* x = cg_lp->getColSolution();
	    cnt = 0;
	    double obj = 0.0;
	    for (int j = 0; j < data.numarcs; ++j) {
		const double xval = floor(x[j] + 0.5);
		if (xval != 0.0) {
		    ind[cnt] = j;
		    val[cnt] = xval;
		    ++cnt;
		    obj += data.arcs[j].weight;
		}
	    }
	    new_vars.push_back(new MCF_var(i, CoinPackedVector(cnt, ind, val,
							       false), obj));
	}
	cg_lp->setRowBounds(comm.source, 0, 0);
	cg_lp->setRowBounds(comm.sink, 0, 0);
    }
    delete[] val;
    delete[] ind;
    delete[] cost;

    generated_vars = new_vars.size() > 0;
}

/*---------------------------------------------------------------------------*/

void
MCF_lp::vars_to_cols(const BCP_vec<BCP_cut*>& cuts,
		     BCP_vec<BCP_var*>& vars,
		     BCP_vec<BCP_col*>& cols,
		     const BCP_lp_result& lpres,
		     BCP_object_origin origin, bool allow_multiple)
{
    static const CoinPackedVector emptyVector(false);
    const int numvars = vars.size();
    for (int i = 0; i < numvars; ++i) {
	const MCF_var* v = dynamic_cast<const MCF_var*>(vars[i]);
	if (v) {
	    // Since we do not generate cuts, we can just disregard the "cuts"
	    // argument, since the column corresponding to the var is exactly
	    // the flow
	    cols.push_back(new BCP_col(v->flow, v->weight, 0.0, 1.0));
	    // Excercise: if we had generated cuts, then the coefficients for
	    // those rows would be appended to the end of each column
	    continue;
	}
    }
}

/*---------------------------------------------------------------------------*/

BCP_branching_decision
MCF_lp::select_branching_candidates(const BCP_lp_result& lpres,
				    const BCP_vec<BCP_var*>& vars,
				    const BCP_vec<BCP_cut*>& cuts,
				    const BCP_lp_var_pool& local_var_pool,
				    const BCP_lp_cut_pool& local_cut_pool,
				    BCP_vec<BCP_lp_branching_object*>& cands)
{
    if (generated_vars) {
	return BCP_DoNotBranch;
    }

    if (lpres.objval() > upper_bound() - 1e-6) {
	return BCP_DoNotBranch_Fathomed;
    }
		
    // This vector will contain one entry only, but we must pass in a vector
    BCP_vec<BCP_var*> new_vars;
    // This vector contains which vars have their ounds forcibly changed. It's
    // going to be the newly added branching var, hence it's position is -1
    BCP_vec<int> fvp(1, -1);
    // This vector contains the new lb/ub pairs for all children. So it's
    // going to be {0.0, 0.0, 1.0, 1.0}
    BCP_vec<double> fvb(4, 0.0);
    fvb[2] = fvb[3] = 1.0;

    // find a few fractional original variables and do strong branching on them
    for (int i = data.numcommodities-1; i >= 0; --i) {
	std::map<int,double>& f = flows[i];
	int most_frac_ind = -1;
	double most_frac_val = 0.5-1e-6;
	double frac_val = 0.0;
	for (std::map<int,double>::iterator fi=f.begin(); fi != f.end(); ++fi) {
	    const double frac = fabs(fi->second - floor(fi->second) - 0.5);
	    if (frac < most_frac_val) {
		most_frac_ind = fi->first;
		most_frac_val = frac;
		frac_val = fi->second;
	    }
	}
	if (most_frac_ind >= 0) {
	    int lb = data.arcs[most_frac_ind].lb;
	    int ub = data.arcs[most_frac_ind].ub;
	    for (int j = branching_vars[i].size() - 1; j >= 0; --j) {
		const MCF_branching_var* bv = branching_vars[i][j];
		if (bv->arc_index == most_frac_ind) {
		    if (bv->ub() == 0.0) {
			lb = CoinMax(lb, bv->lb_child0);
			ub = CoinMin(ub, bv->ub_child0);
		    } else {
			lb = CoinMax(lb, bv->lb_child1);
			ub = CoinMin(ub, bv->ub_child1);
		    }
		}
	    }
	    const int mid = static_cast<int>(floor(frac_val));
	    new_vars.push_back(new MCF_branching_var(i, most_frac_ind,
						     lb, mid, mid+1, ub));
	    cands.push_back(new BCP_lp_branching_object(2, // num of children
							&new_vars,
							NULL, // no new cuts
							&fvp,NULL,&fvb,NULL,
							NULL,NULL,NULL,NULL));
	    new_vars.clear();
	}
    }
    return BCP_DoBranch;
}

/*===========================================================================*/

void MCF_lp::unpack_module_data(BCP_buffer& buf)
{
    par.unpack(buf);
    data.unpack(buf);

    // This is the place where we can preallocate some data structures
    flows = new std::map<int,double>[data.numcommodities];

    // Create the LP that will be used to generate columns
    cg_lp = new OsiClpSolverInterface();

    const int numCols = data.numarcs;
    const int numRows = data.numnodes;
    const int numNz = 2*numCols;

    double *clb = new double[numCols];
    double *cub = new double[numCols];
    double *obj = new double[numCols];
    double *rlb = new double[numRows];
    double *rub = new double[numRows];
    CoinBigIndex *start = new int[numCols+1];
    int *index = new int[numNz];
    double *value = new double[numNz];

    // all these will be properly set for the search tree node in the
    // initialize_new_search_tree_node method
    CoinZeroN(obj, numCols);
    CoinZeroN(clb, numCols);
    CoinZeroFillN(cub, numCols, 1.0);
    // and these will be properly set for the subproblem in the
    // generate_vars_in_lp method
    CoinZeroN(rlb, numRows);
    CoinZeroN(rub, numRows);

    for (int i = 0; i < data.numarcs; ++i) {
	start[i] = 2*i;
	index[2*i] = data.arcs[i].tail;
	index[2*i+1] = data.arcs[i].head;
	value[2*i] = -1;
	value[2*i+1] = 1;
    }
    start[numCols] = 2*numCols;

    cg_lp->loadProblem(numCols, numRows, start, index, value,
		       clb, cub, obj, rlb, rub);

    delete[] value;
    delete[] index;
    delete[] start;
    delete[] rub;
    delete[] rlb;
    delete[] obj;
    delete[] cub;
    delete[] clb;
}			
