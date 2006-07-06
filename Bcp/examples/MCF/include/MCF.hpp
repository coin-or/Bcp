#ifndef MCF_hpp
#define MCF_hpp

#include <iostream>
#include "BCP_buffer.hpp"
#include "OsiSolverInterface.hpp"

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
	bool generated_vars;

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
