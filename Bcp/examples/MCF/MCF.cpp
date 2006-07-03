#include <fstream>
#include "CoinHelperFunctions.hpp"
#include "OsiClpSolverInterface.hpp"
#include "BCP_solution.hpp"
#include "MCF.hpp"

using std::make_pair;

//#############################################################################

int main(int argc, char* argv[])
{
	MCF_init init;
	return bcp_main(argc, argv, &init);
}

//#############################################################################

template <>
void BCP_parameter_set<MCF_par>::create_keyword_list()
{
  // Create the list of keywords for parameter file reading
  keys.push_back(make_pair(BCP_string("MCF_AddDummySourceSinkArcs"),
                           BCP_parameter(BCP_CharPar, AddDummySourceSinkArcs)));
  keys.push_back(make_pair(BCP_string("MCF_InputFilename"),
                           BCP_parameter(BCP_StringPar, InputFilename)));
}

template <>
void BCP_parameter_set<MCF_par>::set_default_entries()
{
	set_entry(InputFilename, "mcf");
	set_entry(AddDummySourceSinkArcs, true);
}

//#############################################################################

USER_initialize * BCP_user_init()
{
  return new MCF_init;
}

/*---------------------------------------------------------------------------*/

BCP_tm_user *
MCF_init::tm_init(BCP_tm_prob& p,
                 const int argnum, const char * const * arglist)
{
  MCF_tm* tm = new MCF_tm;
  tm->par.read_from_arglist(argnum, arglist);
  std::ifstream ifs(tm->par.entry(MCF_par::InputFilename).c_str());
  tm->data.readDimacsFormat(ifs,tm->par.entry(MCF_par::AddDummySourceSinkArcs));
  return tm;
}

/*---------------------------------------------------------------------------*/

BCP_lp_user *
MCF_init::lp_init(BCP_lp_prob& p)
{
	return new MCF_lp;
}

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

//#############################################################################

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
		const std::vector<MCF_branching_var*>& bvars = branching_vars[v->commodity];
		for (j = bvars.size()-1; !violated && j >= 0; --j) {
			const MCF_branching_var* bv = bvars[j];
			const int* pos = std::lower_bound(vind, vind+vsize, bv->arc_index);
			const double f = pos == vind+vsize ? 0.0 : vval[pos-vind];
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

	/* Finally, the solver for the subproblems need to be initialized: the
	   lower/upper bounds on the vars must be set. */
	double* lb = new double[data.numarcs];
	double* ub = new double[data.numarcs];
	for (i = data.numarcs-1; i >= 0; --i) {
		lb[i] = data.arcs[i].lb;
		ub[i] = data.arcs[i].ub;
	}
	for (i = data.numcommodities-1; i >= 0; --i) {
		const std::vector<MCF_branching_var*>& bvars = branching_vars[i];
		for (j = bvars.size() - 1; j >= 0; --j) {
			const MCF_branching_var* bv = bvars[j];
			if (bv->ub() == 0.0) {
				// the upper bound of the branching var in this child is set
				// to 0, so we are in child 0
				lb[bv->arc_index] = bv->lb_child0;
				ub[bv->arc_index] = bv->ub_child0;
			} else {
				lb[bv->arc_index] = bv->lb_child1;
				ub[bv->arc_index] = bv->ub_child1;
			}
		}
	}
	osi->setColLower(lb);
	osi->setColUpper(ub);
	delete[] ub;
	delete[] lb;
}

/*---------------------------------------------------------------------------*/

BCP_solution* MCF_lp::test_feasibility(const BCP_lp_result& lp_result,
									   const BCP_vec<BCP_var*>& vars,
									   const BCP_vec<BCP_cut*>& cuts)
{
	// Here we need to test whether the convex combination of the current
	// columns (according to \lambda, the current primal solution) is integer
	// feasible for the original problem. The only thing that can be violated
	// is integrality
	int i, j;
	for (i = data.numcommodities-1; i >= 0; --i) {
		flows[i].clear();
	}

	const double* x = lp_result.x();
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
	BCP_solution_generic* sol = new BCP_solution_generic;
	for (i = data.numcommodities-1; i >= 0; --i) {
		std::map<int,double>& f = flows[i];
		CoinPackedVector flow(false);
		double weight = 0;
		for (std::map<int,double>::iterator fi=f.begin(); fi != f.end(); ++fi) {
			const double val = floor(fi->second + 0.5);
			flow.insert(fi->first, val);
			weight += val * data.arcs[fi->first].weight;
		}
		sol->add_entry(new MCF_var(i, flow, weight), 1.0);
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
	int i;
	for (i = data.numarcs-1; i >= 0; --i) {
		cost[i] = data.arcs[i].weight - pi[i];
	}
	osi->setObjective(cost);

	// This will hold generated variables
	int* ind = new int[data.numarcs];
	double* val = new double[data.numarcs];
	int cnt = 0;

	for (i = data.numcommodities-1; i >= 0; --i) {
		const MCF_data::commodity& comm = data.commodities[i];
		osi->setRowBounds(comm.source, -comm.demand, -comm.demand);
		osi->setRowBounds(comm.sink, comm.demand, comm.demand);
		osi->initialSolve();
		if (osi->getObjValue() < nu[i] - 1e-8) {
			// we have generated a column Create a var out of it. Round the
			// double values while we are here, after all, they should be
			// integer. there can only be some tiny roundoff error by the LP
			// solver
			const double* x = osi->getColSolution();
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
		osi->setRowBounds(comm.source, 0, 0);
		osi->setRowBounds(comm.sink, 0, 0);
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
		const MCF_var* v =
			dynamic_cast<const MCF_var*>(vars[i]);
		if (v) {
			// Since we do not generate cuts, we can just disregard the "cuts"
			// argument, since the column corresponding to the var is exactly
			// the flow
			cols.push_back(new BCP_col(v->flow, v->weight, 0.0, 1.0));
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

/*
  #############################################################################
  #   Here are the various methods that pack/unpack stuff from a BCP_buffer   #
  #############################################################################
  # They are presented in pairs so it's easy to catch if there is a mismatch  #
  #############################################################################
*/

void MCF_tm::pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
{
	const MCF_var* v = dynamic_cast<const MCF_var*>(var);
	if (v) {
		v->pack(buf);
		return;
	}
	const MCF_branching_var* bv = dynamic_cast<const MCF_branching_var*>(var);
	if (bv) {
		bv->pack(buf);
		return;
	}
}

/*---------------------------------------------------------------------------*/

BCP_var_algo* MCF_tm::unpack_var_algo(BCP_buffer& buf)
{
	return MCF_unpack_var(buf);
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

/*===========================================================================*/

void MCF_lp::unpack_module_data(BCP_buffer& buf)
{
	par.unpack(buf);
	data.unpack(buf);

	// This is the place where we can preallocate some data structures
	branching_vars = new std::vector<MCF_branching_var*>[data.numcommodities];
	arcs_affected = new std::vector<int>[data.numcommodities];

	// Create the LP that will be used to generate columns
	osi = new OsiClpSolverInterface();

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
	CoinZeroN(cub, numCols);
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

	osi->loadProblem(numCols, numRows, start, index, value,
					 clb, cub, obj, rlb, rub);
}			

/*---------------------------------------------------------------------------*/

void MCF_lp::pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
{
	const MCF_var* v = dynamic_cast<const MCF_var*>(var);
	if (v) {
		v->pack(buf);
		return;
	}
	const MCF_branching_var* bv = dynamic_cast<const MCF_branching_var*>(var);
	if (bv) {
		bv->pack(buf);
		return;
	}
}

/*---------------------------------------------------------------------------*/

BCP_var_algo* MCF_lp::unpack_var_algo(BCP_buffer& buf)
{
	return MCF_unpack_var(buf);
}

//#############################################################################

void MCF_data::pack(BCP_buffer& buf) const
{
	int namelen = strlen(problem_name);
	buf.pack(problem_name, namelen);
	buf.pack(numnodes);
	buf.pack(numarcs);
	buf.pack(numcommodities);
	int i;
	for (i = 0; i < numarcs; ++i) {
		buf.pack(arcs[i].tail);
		buf.pack(arcs[i].head);
		buf.pack(arcs[i].lb);
		buf.pack(arcs[i].ub);
		buf.pack(arcs[i].weight);
	}
	for (i = 0; i < numcommodities; ++i) {
		buf.pack(commodities[i].source);
		buf.pack(commodities[i].sink);
		buf.pack(commodities[i].demand);
	}
}

/*---------------------------------------------------------------------------*/

void MCF_data::unpack(BCP_buffer& buf)
{
	int namelen;
	buf.unpack(problem_name, namelen);
	buf.unpack(numnodes);
	buf.unpack(numarcs);
	buf.unpack(numcommodities);
	
	int i;
	for (i = 0; i < numarcs; ++i) {
		buf.unpack(arcs[i].tail);
		buf.unpack(arcs[i].head);
		buf.unpack(arcs[i].lb);
		buf.unpack(arcs[i].ub);
		buf.unpack(arcs[i].weight);
	}
	for (i = 0; i < numcommodities; ++i) {
		buf.unpack(commodities[i].source);
		buf.unpack(commodities[i].sink);
		buf.unpack(commodities[i].demand);
	}
}

//#############################################################################

void MCF_var::pack(BCP_buffer& buf) const
{
	int type = 0;
	buf.pack(type);

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
	buf.unpack(numarcs);
	buf.unpack(ind, numarcs);
	buf.unpack(val, numarcs);
	flow.assignVector(numarcs, ind, val, false /*don't test for duplicates*/);
	buf.unpack(weight);
	set_obj(weight);
}

/*===========================================================================*/

void MCF_branching_var::pack(BCP_buffer& buf) const
{
	int type = 1;
	buf.pack(type);
	buf.pack(commodity);
	buf.pack(arc_index);
	buf.pack(lb_child0);
	buf.pack(ub_child0);
	buf.pack(lb_child1);
	buf.pack(ub_child1);
}

/*---------------------------------------------------------------------------*/

MCF_branching_var::MCF_branching_var(BCP_buffer& buf) :
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

BCP_var_algo* MCF_unpack_var(BCP_buffer& buf)
{
	int t;
	buf.unpack(t);
	switch (t) {
	case 0: return new MCF_var(buf);
	case 1: return new MCF_branching_var(buf);
	default: throw BCP_fatal_error("MCF_unpack_var: bad var type");
	}
	return NULL; // fake return
}

/*
  #############################################################################
  #    Here is the method that reads in the input file
  #############################################################################
*/

int MCF_data::readDimacsFormat(std::istream& s, bool addDummyArcs)
{
	double maxweight = 0;

	bool size_read = false;
	int arcs_read = 0;
	int commodities_read = 0;;

	char line[1000];
	char name[1000];

	while (s.good()) {
		s.getline(line, 1000);
		if (s.gcount() >= 999) {
			printf("Input file is incorrect. A line more than 1000 characters is found.\n");
			return 1;
		}
		switch (line[0]) {
		case 'p':
			if (sscanf(line, "p%s%i%i%i",
					   name, &numnodes, &numarcs, &numcommodities) != 4) {
				printf("Input file is incorrect. (p line)\n");
				return 1;
			}
			problem_name = new char[strlen(name)+1];
			memcpy(problem_name, name, strlen(name)+1);
			arcs = new arc[numarcs + (addDummyArcs ? numcommodities : 0)];
			commodities = new commodity[numcommodities];
			break;
		case 'c':
			break;
		case 'd':
			if (sscanf(line, "d%i%i%i",
					   &commodities[commodities_read].source,
					   &commodities[commodities_read].sink,
					   &commodities[commodities_read].demand) != 3) {
				printf("Input file is incorrect. (d line)\n");
				return 1;
			}
			++commodities_read;
			break;
		case 'a':
			if (sscanf(line, "a%i%i%i%i%lf",
					   &arcs[arcs_read].tail,
					   &arcs[arcs_read].head,
					   &arcs[arcs_read].lb,
					   &arcs[arcs_read].ub,
					   &arcs[arcs_read].weight) != 5) {
				printf("Input file is incorrect. (a line)\n");
				return 1;
			}
			if (fabs(arcs[arcs_read].weight) > maxweight) {
				maxweight = fabs(arcs[arcs_read].weight);
			}
			++arcs_read;
			break;
		default:
			if (sscanf(line+1, "%s", name) <= 0) {
				printf("Input file is incorrect. (non-recognizable line)\n");
				return 1;
			}
			break;
		}
	}

	if (!size_read || arcs_read!=numarcs || commodities_read!=numcommodities) {
		printf("Input file is incorrect. size_read=%i arcs_read=%i commodities_read=%i\n",
			   size_read, arcs_read, commodities_read);
		return 1;
	}

	if (addDummyArcs) {
		for (int i = 0; i < numcommodities; ++i) {
			arcs[numarcs].tail   = commodities[i].source;
			arcs[numarcs].head   = commodities[i].sink;
			arcs[numarcs].lb     = 0;
			arcs[numarcs].ub     = commodities[i].demand;
			arcs[numarcs].weight = maxweight+1;
			++numarcs;
		}
	}
	return 0;
}

//#############################################################################

