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
    // Go through all the variables, select the MCF_branching_var's and save
    // the upper/lower bounds to be applied in the subproblems
    int i, j;
    for (i = data.numcommodities-1; i >= 0; --i) {
	branching_vars[i].clear();
	arcs_affected[i].clear();
    }
    for (i = vars.size()-1; i >= 0; --i) {
	MCF_branching_var* v = dynamic_cast<MCF_branching_var*>(vars[i]);
	if (v) {
	    branching_vars[v->commodity].push_back(v);
	    arcs_affected[v->commodity].push_back(v->arc_index);
	}
    }

    /* The branching decisions may set bounds that are violated by some
       of the variables (more precisely by the flow they represent). Find them
       and set their upper bounds to 0, since it's highly unlikely that we'd
       be able to find a fractional \lambda vector such that:
       1) such a flow has nonzero coefficient;
       2) the convex combination of all flows with nonzero \lambda is integer.
    */
    for (i = vars.size()-1; i >= 0; --i) {
	MCF_var* v = dynamic_cast<MCF_var*>(vars[i]);
	if (!v) continue;
	const int vsize = v->flow.getNumElements();
	const int* vind = v->flow.getIndices();
	const double* vval = v->flow.getElements();

	bool violated = false;
	const std::vector<MCF_branching_var*>& bvars =
	    branching_vars[v->commodity];
	for (j = bvars.size()-1; !violated && j >= 0; --j) {
	    const MCF_branching_var* bv = bvars[j];
	    double f = 0.0;
	    for (int k = 0; k < vsize; ++k) {
		if (vind[k] == bv->arc_index) {
		    f = vval[k];
		    break;
		}
	    }
	    if (bv->ub() == 0.0) {
		// the upper bound of the branching var in this child is set
		// to 0, so we are in child 0
		violated = (f < bv->lb_child0 || f > bv->ub_child0);
	    } else {
		violated = (f < bv->lb_child1 || f > bv->ub_child1);
	    }
	}
	if (violated) {
	    var_changed_pos.push_back(i);
	    var_new_bd.push_back(0.0); // the new lower bound on the var
	    var_new_bd.push_back(0.0); // the new upper bound on the var
	}
    }

    // clear out our local pool
    purge_ptr_vector(gen_vars);
}

/*---------------------------------------------------------------------------*/

BCP_solution*
MCF_lp::test_feasibility(const BCP_lp_result& lpres,
			 const BCP_vec<BCP_var*>& vars,
			 const BCP_vec<BCP_cut*>& cuts)
{
    getLpProblemPointer()->lp_solver->writeMps("currlp", "mps");
    printf("Current LP written in file currlp.mps\n");
    getLpProblemPointer()->lp_solver->writeLp("currlp", "lp");
    printf("Current LP written in file currlp.lp\n");

    // Feasibility testing: we need to test whether the convex combination of
    // the current columns (according to \lambda, the current primal solution)
    // is integer feasible for the original problem. The only thing that can
    // be violated is integrality.
  
    int i, j;
    for (i = data.numcommodities-1; i >= 0; --i) {
	flows[i].clear();
    }

    const double* x = lpres.x();
    for (i = vars.size()-1; i >= 0; --i) {
	MCF_var* v = dynamic_cast<MCF_var*>(vars[i]);
	if (!v) continue;
	std::map<int,double>& f = flows[v->commodity];
	const int vsize = v->flow.getNumElements();
	const int* vind = v->flow.getIndices();
	const double* vval = v->flow.getElements();
	for (j = 0; j < vsize; ++j) {
	    f[vind[j]] += vval[j]*x[i];
	}
    }
    for (i = data.numcommodities-1; i >= 0; --i) {
	std::map<int,double>& f = flows[i];
	for (std::map<int,double>::iterator fi=f.begin(); fi != f.end(); ++fi) {
	    const double frac = fabs(fi->second - floor(fi->second) - 0.5);
	    if (frac < 0.5-1e-6)
		return NULL;
	}
    }

    // Found an integer solution
    BCP_solution_generic* gsol = new BCP_solution_generic;
    for (i = data.numcommodities-1; i >= 0; --i) {
	std::map<int,double>& f = flows[i];
	CoinPackedVector flow(false);
	double weight = 0;
	for (std::map<int,double>::iterator fi=f.begin();
	     fi != f.end();
	     ++fi) {
	    const double val = floor(fi->second + 0.5);
	    if (val > 0) {
		flow.insert(fi->first, val);
		weight += val * data.arcs[fi->first].weight;
	    }
	}
	gsol->add_entry(new MCF_var(i, flow, weight), 1.0);
    }
    return gsol;
}

/*---------------------------------------------------------------------------*/

double
MCF_lp::compute_lower_bound(const double old_lower_bound,
			    const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts)
{
    // To compute a true lower bound we need to generate variables first (if
    // we can). These are saved so that we can return them in
    // generate_vars_in_lp.

    // generate variables:  for each commodity generate a flow.

    const int numarcs = data.numarcs;
    double* cost = new double[numarcs];
    const double* pi = lpres.pi();
    const double* nu = pi + numarcs;

    int i, j;

    for (i = numarcs-1; i >= 0; --i) {
	cost[i] = data.arcs[i].weight - pi[i];
    }
    cg_lp->setObjective(cost);

    // This will hold generated variables
    int* ind = new int[numarcs];
    double* val = new double[numarcs];
    int cnt = 0;

    for (i = data.numcommodities-1; i >= 0; --i) {
	const MCF_data::commodity& comm = data.commodities[i];
	cg_lp->setRowBounds(comm.source, -comm.demand, -comm.demand);
	cg_lp->setRowBounds(comm.sink, comm.demand, comm.demand);
	const std::vector<MCF_branching_var*>& bvars = branching_vars[i];
	for (j = bvars.size() - 1; j >= 0; --j) {
	    const MCF_branching_var* bv = bvars[j];
	    const int ind = bv->arc_index;
	    if (bv->ub() == 0.0) {
		// the upper bound of the branching var in this child is set
		// to 0, so we are in child 0
		cg_lp->setColBounds(ind, bv->lb_child0, bv->ub_child0);
	    } else {
		cg_lp->setColBounds(ind, bv->lb_child1, bv->ub_child1);
	    }
	}
	cg_lp->initialSolve();
	if (cg_lp->isProvenOptimal() && cg_lp->getObjValue() < nu[i] - 1e-8) {
	    // we have generated a column. Create a var out of it. Round the
	    // double values while we are here, after all, they should be
	    // integer. there can only be some tiny roundoff error by the LP
	    // solver
	    const double* x = cg_lp->getColSolution();
	    cnt = 0;
	    double obj = 0.0;
	    for (int j = 0; j < numarcs; ++j) {
		const double xval = floor(x[j] + 0.5);
		if (xval != 0.0) {
		    ind[cnt] = j;
		    val[cnt] = xval;
		    ++cnt;
		    obj += data.arcs[j].weight * xval;
		}
	    }
	    gen_vars.push_back(new MCF_var(i, CoinPackedVector(cnt, ind, val,
							       false), obj));
	}
	for (j = bvars.size() - 1; j >= 0; --j) {
	    const MCF_branching_var* bv = bvars[j];
	    const int ind = bv->arc_index;
	    cg_lp->setColBounds(ind, data.arcs[ind].lb, data.arcs[ind].ub);
	}
	cg_lp->setRowBounds(comm.source, 0, 0);
	cg_lp->setRowBounds(comm.sink, 0, 0);
    }
    delete[] val;
    delete[] ind;
    delete[] cost;

    // Excercise: do the same in a random order and apply dual discounting
    // Not yet implemented.

    // Now get a true lower bound
    double true_lower_bound = 0.0;
    generated_vars = (gen_vars.size() > 0);

    if (generated_vars) {
	true_lower_bound = old_lower_bound;
    } else {
	true_lower_bound = lpres.objval();
	// Excercise: Get a better true lower bound

	// Hint: lpres.objval() + The sum of the reduced costs of the
	// variables with the most negative reduced cost in each subproblem
	// yield a true lower bound

	// Not yet implemented.
    }

    return true_lower_bound;
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
    new_vars.append(gen_vars);
    gen_vars.clear();
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
	const MCF_var* v =
	    dynamic_cast<const MCF_var*>(vars[i]);
	if (v) {
	    // Since we do not generate cuts, we can just disregard the "cuts"
	    // argument, since the column corresponding to the var is exactly
	    // the flow (plus the entry in the appropriate convexity
	    // constraint)
	    BCP_col* col = new BCP_col(v->flow, v->weight, 0.0, 1.0);
	    col->insert(data.numarcs + v->commodity, 1.0);
	    cols.push_back(col);
	    // Excercise: if we had generated cuts, then the coefficients for
	    // those rows would be appended to the end of each column
	    continue;
	}
	const MCF_branching_var* bv =
	    dynamic_cast<const MCF_branching_var*>(vars[i]);
	if (bv) {
	    cols.push_back(new BCP_col(emptyVector, 0.0, bv->lb(), bv->ub()));
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
    if (generated_vars > 0) {
	return BCP_DoNotBranch;
    }

    if (lpres.objval() > upper_bound() - 1e-6) {
	return BCP_DoNotBranch_Fathomed;
    }

    const int dummyStart = par.entry(MCF_par::AddDummySourceSinkArcs) ?
	data.numarcs - data.numcommodities : data.numarcs;
		
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
	for (std::map<int,double>::iterator fi=f.begin(); fi != f.end(); ++fi){
	    if (fi->first >= dummyStart)
		continue;
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
    branching_vars = new std::vector<MCF_branching_var*>[data.numcommodities];
    arcs_affected = new std::vector<int>[data.numcommodities];
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
    for (int i = numCols - 1; i >= 0; --i) {
	cub[i] = data.arcs[i].ub;
    }
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
