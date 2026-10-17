#ifndef MCF2_tm_hpp
#define MCF2_tm_hpp

#include "BCP_buffer.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_parameters.hpp"
#include "MCF2_par.hpp"
#include "MCF2_var.hpp"
#include "MCF2_data.hpp"

class MCF2_packer : public BCP_user_pack
{
  /** Pack an algorithmic variable */
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf) {
    MCF2_pack_var(var, buf);
  }

  /** Unpack an algorithmic variable */
  virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf) {
    return MCF2_unpack_var(buf);
  }
};

//##############################################################################

class MCF2_tm : public BCP_tm_user
{
public:
  BCP_parameter_set<MCF2_par> par;
  MCF2_data data;

public:
  MCF2_tm() {}
  virtual ~MCF2_tm() {}

  virtual void pack_module_data(BCP_buffer& buf, BCP_process_t ptype);
  virtual void init_new_phase(int phase,
			      BCP_column_generation& colgen,
			      CoinSearchTreeBase*& candidates);
  virtual void initialize_core(BCP_vec<BCP_var_core*>& vars,
			       BCP_vec<BCP_cut_core*>& cuts,
			       BCP_lp_relax*& matrix);
  virtual void create_root(BCP_vec<BCP_var*>& added_vars,
			   BCP_vec<BCP_cut*>& added_cuts,
			   BCP_user_data*& user_data);
  virtual void display_feasible_solution(const BCP_solution* sol);
};

#endif
