#ifndef MCF1_lp_hpp
#define MCF1_lp_hpp

#include "OsiSolverInterface.hpp"
#include "BCP_buffer.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_parameters.hpp"
#include "MCF1_par.hpp"
#include "MCF1_var.hpp"
#include "MCF1_data.hpp"

class MCF1_lp : public BCP_lp_user
{
    OsiSolverInterface* cg_lp;
    BCP_parameter_set<MCF1_par> par;
    MCF1_data data;
    std::vector<MCF1_branch_decision>* branch_history;
    // the solution to the original formulation
    std::map<int,double>* flows;

    BCP_vec<BCP_var*> gen_vars;
    bool generated_vars;

public:
    MCF1_lp() {}
    virtual ~MCF1_lp() {
	delete[] branch_history;
	delete[] flows;
	purge_ptr_vector(gen_vars);
	delete cg_lp;
    }

    virtual void unpack_module_data(BCP_buffer& buf);

    virtual OsiSolverInterface* initialize_solver_interface();

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
    virtual double
    compute_lower_bound(const double old_lower_bound,
			const BCP_lp_result& lpres,
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
				BCP_vec<BCP_lp_branching_object*>& cands,
				bool force_branch = false);
};

#endif
