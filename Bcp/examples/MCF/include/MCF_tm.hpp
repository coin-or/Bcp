#ifndef MCF_tm_hpp
#define MCF_tm_hpp

#include "BCP_buffer.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_parameters.hpp"
#include "MCF_par.hpp"
#include "MCF_var.hpp"
#include "MCF_data.hpp"

class MCF_tm : public BCP_tm_user
{
public:
    BCP_parameter_set<MCF_par> par;
    MCF_data data;

public:
    MCF_tm() {}
    virtual ~MCF_tm() {}

    virtual void pack_module_data(BCP_buffer& buf, BCP_process_t ptype);
    virtual void pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf) {
	MCF_pack_var(var, buf);
    }
    virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf) {
	return MCF_unpack_var(buf);
    }
    virtual void initialize_core(BCP_vec<BCP_var_core*>& vars,
				 BCP_vec<BCP_cut_core*>& cuts,
				 BCP_lp_relax*& matrix);
    virtual void create_root(BCP_vec<BCP_var*>& added_vars,
			     BCP_vec<BCP_cut*>& added_cuts,
			     BCP_user_data*& user_data,
			     BCP_pricing_status& pricing_status);
    virtual void display_feasible_solution(const BCP_solution* sol);
};

#endif
