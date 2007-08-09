#ifndef MCF3_lp_hpp
#define MCF3_lp_hpp

#include "OsiSolverInterface.hpp"
#include "BCP_buffer.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_parameters.hpp"
#include "MCF3_par.hpp"
#include "MCF3_var.hpp"
#include "MCF3_data.hpp"

class MCF3_lp : public BCP_lp_user
{
    OsiSolverInterface* cg_lp;
    BCP_parameter_set<MCF3_par> par;
    MCF3_data data;
    std::vector<MCF3_branch_decision>* branch_history;
    std::vector<MCF3_branch_decision>* new_branch;
    std::vector<int> commodities_with_candidate;
    
    // the solution to the original formulation
    std::map<int,double>* flows;

    BCP_vec<BCP_var*> gen_vars;
    bool generated_vars;

public:
    MCF3_lp() {}
    virtual ~MCF3_lp() {
	delete[] flows;
	delete[] new_branch;
	purge_ptr_vector(gen_vars);
	delete cg_lp;
    }

    virtual void unpack_module_data(BCP_buffer& buf);
    virtual void pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf) {
	MCF3_pack_var(var, buf);
    }
    virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf) {
	return MCF3_unpack_var(buf);
    }
    
    virtual void pack_user_data(const BCP_user_data* ud, BCP_buffer& buf) {
	const MCF3_user* u = dynamic_cast<const MCF3_user*>(ud);
	u->pack(buf);
    }
    virtual BCP_user_data* unpack_user_data(BCP_buffer& buf) {
	MCF3_user* u = new MCF3_user;
	u->unpack(buf);
	return u;
    }

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
				BCP_vec<BCP_lp_branching_object*>& cands);
    virtual void
    set_user_data_for_children(BCP_presolved_lp_brobj* best, 
			       const int selected);
};

#endif
