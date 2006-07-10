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
    // Apply the bounds on the cuts to the vars of the subproblem, i.e., don't
    // generate flows that violate the global constraints
    const int numarcs = data.numarcs;
    double* ub = new double[numarcs];
    for (int i = numarcs - 1; i >= 0; --i) {
	ub[i] = cuts[i]->ub();
    }
    cg_lp->setColUpper(ub);

    // Excercise (logical fixing): (implemented here)
    // Set the bound on those vars to 0 which violate the global flow
    // constraints
    for (int i = vars.size()-1; i >= 0; --i) {
	MCF_var* v = dynamic_cast<MCF_var*>(vars[i]);
	const int vsize = v->flow.getNumElements();
	const int* vind = v->flow.getIndices();
	const double* vval = v->flow.getElements();

	bool violated = false;
	for (int j = 0; j < vsize && !violated; ++j) {
	    violated = (vval[j] > ub[vind[j]]);
	}
	if (violated) {
	    var_changed_pos.push_back(i);
	    var_new_bd.push_back(0.0); // the new lower bound on the var
	    var_new_bd.push_back(0.0); // the new upper bound on the var
	}
    }

    delete[] ub;

    generated_vars = 0;
}

/*---------------------------------------------------------------------------*/

BCP_solution* MCF_lp::test_feasibility(const BCP_lp_result& lp_result,
				       const BCP_vec<BCP_var*>& vars,
				       const BCP_vec<BCP_cut*>& cuts)
{
     getLpProblemPointer()->lp_solver->writeLp("currlp", "lp");
     printf("Current LP written in file currlp.lp\n");

     return(0);
}
/*---------------------------------------------------------------------------*/

void MCF_lp::process_lp_result(const BCP_lp_result& lpres,
			       const BCP_vec<BCP_var*>& vars,
			       const BCP_vec<BCP_cut*>& cuts,
			       const double old_lower_bound,
			       double& true_lower_bound,
			       BCP_solution*& sol,
			       BCP_vec<BCP_cut*>& new_cuts,
			       BCP_vec<BCP_row*>& new_rows,
			       BCP_vec<BCP_var*>& new_vars,
			       BCP_vec<BCP_col*>& new_cols)
{
    // Here we need to test feasibility, compute a new true lower bound and
    // generate variables

    // testing feasibility: we need to test whether the left-hand-side of each
    // row is integer or not.
    const double* lhs = lpres.lhs();
    const int numarcs = data.numarcs;
    int i, j;
    for (i = 0; i < numarcs; ++i) {
	const double frac = fabs(lhs[i] - floor(lhs[i]) - 0.5);
	if (frac < 0.5-1e-6)
	    break;
    }

    if (i == numarcs) {
	// Solution is feasible, for each commodity combine the correspondng
	// flows into one
	const double* x = lpres.x();
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
	BCP_solution_generic* gsol = new BCP_solution_generic;
	for (i = data.numcommodities-1; i >= 0; --i) {
	    std::map<int,double>& f = flows[i];
	    CoinPackedVector flow(false);
	    double weight = 0;
	    for (std::map<int,double>::iterator fi = f.begin();
		 fi != f.end(); ++fi) {
		flow.insert(fi->first, fi->second);
		weight += fi->second * data.arcs[fi->first].weight;
	    }
	    gsol->add_entry(new MCF_var(i, flow, weight), 1.0);
	}
	for (i = data.numcommodities-1; i >= 0; --i) {
	    flows[i].clear();
	}
	sol = gsol;
    }

    // Now generate variables:  for each commodity generate a flow.
    double* cost = new double[numarcs];
    const double* pi = lpres.pi();
    const double* nu = pi + numarcs;
    for (i = numarcs-1; i >= 0; --i) {
	cost[i] = data.arcs[i].weight - pi[i];
    }
    cg_lp->setObjective(cost);

    // This will hold generated variables
    int* ind = new int[numarcs+1];
    double* val = new double[numarcs+1];
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
	    for (int j = 0; j < numarcs; ++j) {
		const double xval = floor(x[j] + 0.5);
		if (xval != 0.0) {
		    ind[cnt] = j;
		    val[cnt] = xval;
		    ++cnt;
		    obj += data.arcs[j].weight * xval;
		}
	    }
	    new_vars.push_back(new MCF_var(i, CoinPackedVector(cnt, ind, val,
							       false), obj));
	    // The corresponding column
	    ind[cnt] = numarcs + i;
	    val[cnt] = 1.0;
	    new_cols.push_back(new BCP_col());
	    new_cols.back()->copy(cnt+1, ind, val, obj, 0.0, 1.0);
	}
	cg_lp->setRowBounds(comm.source, 0, 0);
	cg_lp->setRowBounds(comm.sink, 0, 0);
    }
    delete[] val;
    delete[] ind;
    delete[] cost;

    generated_vars = new_vars.size() > 0;

    // Excercise: do the same in a random order and apply dual discounting
    // Not yet implemented.

    // Now get a true lower bound
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
	    // the flow (plus the entry in the appropriate convexity
	    // constraint)
	    BCP_col* col = new BCP_col(v->flow, v->weight, 0.0, 1.0);
	    col->insert(data.numarcs + v->commodity, 1.0);
	    cols.push_back(col);
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

    const double* lhs = lpres.lhs();
    
    int i;
    
    int most_frac_ind = -1;
    double most_frac_val = 0.5-1e-6;

    for (i = 0; i < data.numarcs; ++i) {
	const double frac = fabs(lhs[i] - floor(lhs[i]) - 0.5);
	if (frac < most_frac_val) {
	    most_frac_ind = i;
	    most_frac_val = frac;
	}
    }

    if(most_frac_ind == -1) {
      printf("### ERROR: MCF_lp::select_braching_candidates(): most_frac_ind = -1\n");
      exit(1);
    }

    // This vector contains which vars have their ounds forcibly changed.
    BCP_vec<int> fcp;
    // This vector contains the new lb/ub pairs for all children.
    BCP_vec<double> fcb;

    fcp.push_back(most_frac_ind);
    fcb.push_back(cuts[most_frac_ind]->lb());
    fcb.push_back(floor(lhs[most_frac_ind]));
    fcb.push_back(ceil(lhs[most_frac_ind]));
    fcb.push_back(cuts[most_frac_ind]->ub());

    cands.push_back(new BCP_lp_branching_object(2, // num of children
						NULL, // no new vars
						NULL, // no new cuts
						NULL, &fcp, NULL, &fcb,
						NULL, NULL, NULL, NULL));
    return BCP_DoBranch;

    // Excercise: select several candidates and run strong branching
    // Not yet implemented
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
