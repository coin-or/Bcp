#ifndef MCF3_tm_hpp
#define MCF3_tm_hpp

#include "BCP_buffer.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_parameters.hpp"
#include "MCF3_par.hpp"
#include "MCF3_var.hpp"
#include "MCF3_data.hpp"

class MCF3_packer : public BCP_user_pack
{
  /** Pack an algorithmic variable */
  virtual void
  pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf) {
    MCF3_pack_var(var, buf);
  }

  /** Unpack an algorithmic variable */
  virtual BCP_var_algo* unpack_var_algo(BCP_buffer& buf) {
    return MCF3_unpack_var(buf);
  }

  /** Pack an user data */
  virtual void
  pack_user_data(const BCP_user_data* ud, BCP_buffer& buf) {
    const MCF3_user* u = dynamic_cast<const MCF3_user*>(ud);
    u->pack(buf);
  }

  /** Unpack an user data */
  virtual BCP_user_data* unpack_user_data(BCP_buffer& buf) {
    return new MCF3_user(buf);
  }
};

//##############################################################################

class MCF3_tm : public BCP_tm_user
{
public:
  BCP_parameter_set<MCF3_par> par;
  MCF3_data data;

public:
  MCF3_tm() {}
  virtual ~MCF3_tm() {}

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
