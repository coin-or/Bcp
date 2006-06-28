#ifndef MCF_hpp
#define MCF_hpp

#include <iostream>
#include "BCP_buffer.hpp"
#include "OsiSolverInterface.hpp"

//#############################################################################

// These are the parameters for the MCF code
    
class MCF_par {
public:
    enum chr_params {
		AddDummySourceSinkArcs,
        //
        end_of_chr_params
    };
    enum int_params {
        //
        end_of_int_params
    };
    enum dbl_params {
        //
        end_of_dbl_params
    };
    enum str_params {
        InputFilename,
        //
        end_of_str_params
    };
    enum str_array_params {
        //
        end_of_str_array_params
    };
};

//#############################################################################

// This structure holds the input data

class MCF_data {
public:
	struct arc {
		int tail;
		int head;
		int lb;
		int ub;
		double weight;
	};
	struct commodity {
		int source;
		int sink;
		int demand;
	};
	char* problem_name;
	arc* arcs;
	commodity* commodities;
	int numarcs;
	int numnodes;
	int numcommodities;

public:
	MCF_data() :
		arcs(NULL), commodities(NULL),
		numarcs(0), numnodes(0), numcommodities(0) {}

	~MCF_data() {
		delete[] arcs;
		delete[] commodities;
		delete[] problem_name;
	}

	int readDimacsFormat(std::istream& s, bool addDummyArcs);
	void pack(BCP_buffer& buf) const;
	void unpack(BCP_buffer& buf);
};

//#############################################################################

#include "CoinPackedVector.hpp"
#include "BCP_var.hpp"

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
		commodity(com), weight(w)
	{
		new (&flow) CoinPackedVector(f.getNumElements(),
									 f.getIndices(), f.getElements(), false);
	}
	MCF_var(BCP_buffer& buf);
	~MCF_var() {}

	void pack(BCP_buffer& buf) const;
};

/*----------------------------------------------------------------------------*/

class MCF_branching_var : public BCP_var_algo {
public:
	int commodity;
	int arc_index;
	int lb_child0;
	int ub_child0;
	int lb_child1;
	int ub_child1;

public:
	MCF_branching_var(int comm, int ind, int lb0, int ub0, int lb1, int ub1) :
		BCP_var_algo(BCP_BinaryVar, 0, 0, 1),
		commodity(comm), arc_index(ind),
		lb_child0(lb0), ub_child0(ub0), lb_child1(lb1), ub_child1(ub1) {}
	MCF_branching_var(BCP_buffer& buf);
	~MCF_branching_var() {}

	void pack(BCP_buffer& buf) const;
};

BCP_var_algo* MCF_unpack_var(BCP_buffer& buf);

//#############################################################################

#include "BCP_parameters.hpp"
#include "BCP_tm_user.hpp"

class MCF_tm : public BCP_tm_user
{
public:
	BCP_parameter_set<MCF_par> par;
	MCF_data data;

public:
	MCF_tm() {}
	virtual ~MCF_tm() {}

	virtual void pack_module_data(BCP_buffer& buf, BCP_process_t ptype);
	virtual void pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);
	virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf);
	virtual void initialize_core(BCP_vec<BCP_var_core*>& vars,
								 BCP_vec<BCP_cut_core*>& cuts,
								 BCP_lp_relax*& matrix);
	virtual void create_root(BCP_vec<BCP_var*>& added_vars,
							 BCP_vec<BCP_cut*>& added_cuts,
							 BCP_user_data*& user_data,
							 BCP_pricing_status& pricing_status);
	virtual void display_feasible_solution(const BCP_solution* sol);
};

//#############################################################################

#include "BCP_lp_user.hpp"

class MCF_lp : public BCP_lp_user
{
	OsiSolverInterface* osi;
	BCP_parameter_set<MCF_par> par;
	MCF_data data;
	std::vector<MCF_branching_var*>* branching_vars;
	std::vector<int>* arcs_affected;
	// the solution to the original formulation
	std::map<int,double>* flows;

public:
	MCF_lp() {}
	virtual ~MCF_lp() {
		delete[] branching_vars;
		delete[] arcs_affected;
		delete[] flows;
	}

	virtual void unpack_module_data(BCP_buffer& buf);
	virtual void pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf);
	virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf);

	virtual void
	initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
									const BCP_vec<BCP_cut*>& cuts,
									const BCP_vec<BCP_obj_status>& var_status,
									const BCP_vec<BCP_obj_status>& cut_status,
									BCP_vec<int>& var_changed_pos,
									BCP_vec<double>& var_new_bd,
									BCP_vec<int>& cut_changed_pos,
									BCP_vec<double>& cut_new_bd);
	virtual BCP_solution*
    test_feasibility(const BCP_lp_result& lp_result,
					 const BCP_vec<BCP_var*>& vars,
					 const BCP_vec<BCP_cut*>& cuts);
	virtual void
    generate_vars_in_lp(const BCP_lp_result& lpres,
						const BCP_vec<BCP_var*>& vars,
						const BCP_vec<BCP_cut*>& cuts,
						const bool before_fathom,
						BCP_vec<BCP_var*>& new_vars,
						BCP_vec<BCP_col*>& new_cols);
	virtual void
	vars_to_cols(const BCP_vec<BCP_cut*>& cuts,
				 BCP_vec<BCP_var*>& vars,
				 BCP_vec<BCP_col*>& cols,
				 const BCP_lp_result& lpres,
				 BCP_object_origin origin, bool allow_multiple);
    virtual BCP_branching_decision
    select_branching_candidates(const BCP_lp_result& lpres,
								const BCP_vec<BCP_var*>& vars,
								const BCP_vec<BCP_cut*>& cuts,
								const BCP_lp_var_pool& local_var_pool,
								const BCP_lp_cut_pool& local_cut_pool,
								BCP_vec<BCP_lp_branching_object*>& cands);
};

//#############################################################################

#include "BCP_USER.hpp"

class MCF_init : public USER_initialize {
public:
  virtual BCP_tm_user * tm_init(BCP_tm_prob& p,
                                const int argnum,
                                const char * const * arglist);
  virtual BCP_lp_user * lp_init(BCP_lp_prob& p);
};

#endif
