// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cmath>

#include "CoinHelperFunctions.hpp"

#include "BCP_error.hpp"
#include "BCP_USER.hpp"
#include "BCP_temporary.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_lp_user.hpp"
#include "BCP_lp.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_node.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_solution.hpp"
#include "BCP_functions.hpp"

//#############################################################################
// Informational methods for the user
double BCP_lp_user::upper_bound() const    { return p->ub(); }
int BCP_lp_user::current_phase() const     { return p->phase; }
int BCP_lp_user::current_level() const     { return p->node->level; }
int BCP_lp_user::current_index() const     { return p->node->index; }
int BCP_lp_user::current_iteration() const { return p->node->iteration_count; }

//#############################################################################
// Informational methods for the user
/* Methods to get/set BCP parameters on the fly */
char
BCP_lp_user::get_param(const BCP_lp_par::chr_params key) const
{ return p->par.entry(key); }
int
BCP_lp_user::get_param(const BCP_lp_par::int_params key) const
{ return p->par.entry(key); }
double
BCP_lp_user::get_param(const BCP_lp_par::dbl_params key) const
{ return p->par.entry(key); }
const BCP_string&
BCP_lp_user::get_param(const BCP_lp_par::str_params key) const
{ return p->par.entry(key); }

void BCP_lp_user::set_param(const BCP_lp_par::chr_params key, const bool val)
{ return p->par.set_entry(key, val); }
void BCP_lp_user::set_param(const BCP_lp_par::chr_params key, const char val)
{ return p->par.set_entry(key, val); }
void BCP_lp_user::set_param(const BCP_lp_par::int_params key, const int val)
{ return p->par.set_entry(key, val); }
void BCP_lp_user::set_param(const BCP_lp_par::dbl_params key, const double val)
{ return p->par.set_entry(key, val); }
void BCP_lp_user::set_param(const BCP_lp_par::str_params key, const char * val)
{ return p->par.set_entry(key, val); }

//#############################################################################

void
BCP_lp_user::send_feasible_solution(const BCP_solution* sol)
{
  // send the feas sol to the tree manager
  p->msg_buf.clear();
  pack_feasible_solution(p->msg_buf, sol);
  p->msg_env->send(p->tree_manager, BCP_Msg_FeasibleSolution, p->msg_buf);

  // update the UB if necessary
  const double objval = sol->objective_value();
  const bool over_ub = p->ub(objval);
  if (! p->param(BCP_lp_par::SolveLpToOptimality) && over_ub)
    p->lp_solver->setDblParam(OsiDualObjectiveLimit, objval-p->granularity());
}

//#############################################################################

// These functions are member functions of the VIRTUAL class BCP_lp_user.
// Therefore any concrete class derived from BCP_lp_user can override these
// definitions. These are here as default functions.

//#############################################################################
// a few helper functions for selecting a subset of a double vector
void
BCP_lp_user::select_nonzeros(const double * first, const double * last,
			     const double etol,
			     BCP_vec<int>& nonzeros) const {
  nonzeros.reserve(last - first);
  BCP_vec<double>::const_iterator current = first;
  for ( ; current != last; ++current)
    if (CoinAbs(*current) > etol) nonzeros.unchecked_push_back(current - first);
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::select_zeros(const double * first, const double * last,
			  const double etol,
			  BCP_vec<int>& zeros) const {
  zeros.reserve(last - first);
  BCP_vec<double>::const_iterator current = first;
  for ( ; current != last; ++current)
    if (CoinAbs(*current) <= etol) zeros.unchecked_push_back(current - first);
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::select_positives(const double * first, const double * last,
			      const double etol,
			      BCP_vec<int>& positives) const {
  positives.reserve(last - first);
  BCP_vec<double>::const_iterator current = first;
  for ( ; current != last; ++current)
    if (*current > etol) positives.unchecked_push_back(current - first);
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::select_fractions(const double * first, const double * last,
			      const double etol,
			      BCP_vec<int>& fractions) const {
  fractions.reserve(last - first);
  BCP_vec<double>::const_iterator current = first;
  for ( ; current != last; ++current)
    if (*current - floor(*current) >etol && ceil(*current) - *current >etol)
      fractions.unchecked_push_back(current - first);
}

//#############################################################################
//#############################################################################
// unpack the initial info for the appropriate process
void
BCP_lp_user::unpack_module_data(BCP_buffer & buf)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default unpack_module_data() executed.\n");
  }
}

//#############################################################################

void
BCP_lp_user::pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default BCP_lp_user::pack_warmstart() executed.\n");
  }
  BCP_pack_warmstart(ws, buf);
}

//-----------------------------------------------------------------------------

BCP_warmstart*
BCP_lp_user::unpack_warmstart(BCP_buffer& buf)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default BCP_lp_user::unpack_warmstart() executed.\n");
  }
  return BCP_unpack_warmstart(buf);
}

//#############################################################################

void
BCP_lp_user::pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_lp_user::pack_var_algo() invoked but not overridden!\n");
}

//-----------------------------------------------------------------------------
BCP_var_algo*
BCP_lp_user::unpack_var_algo(BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_lp_user::unpack_var_algo() invoked but not overridden!\n");
}
      
//-----------------------------------------------------------------------------
void
BCP_lp_user::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_lp_user::pack_cut_algo() invoked but not overridden!\n");
}

//-----------------------------------------------------------------------------
BCP_cut_algo*
BCP_lp_user::unpack_cut_algo(BCP_buffer& buf)
{
  throw BCP_fatal_error("\
BCP_lp_user::unpack_cut_algo() invoked but not overridden!\n");
}

//#############################################################################

#if defined(COIN_USE_VOL)

#include "OsiVolSolverInterface.hpp"
OsiSolverInterface * 
BCP_lp_user::initialize_solver_interface()
{
   return new OsiVolSolverInterface();
}

#elif defined(COIN_USE_CPX)

#include "OsiCpxSolverInterface.hpp"
OsiSolverInterface * 
BCP_lp_user::initialize_solver_interface()
{
   return new OsiCpxSolverInterface();
}

#elif defined(COIN_USE_XPR)

#include "OsiXprSolverInterface.hpp"
OsiSolverInterface * 
BCP_lp_user::initialize_solver_interface()
{
   return new OsiXprSolverInterface();
}

#elif defined(COIN_USE_OSL)

#include "OsiOslSolverInterface.hpp"
OsiSolverInterface * 
BCP_lp_user::initialize_solver_interface()
{
   return new OsiOslSolverInterface();
}

#endif

//#############################################################################
void
BCP_lp_user::initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
					     const BCP_vec<BCP_cut*>& cuts,
					     const BCP_vec<BCP_obj_status>& vs,
					     const BCP_vec<BCP_obj_status>& cs,
					     BCP_vec<int>& var_changed_pos,
					     BCP_vec<double>& var_new_bd,
					     BCP_vec<int>& cut_changed_pos,
					     BCP_vec<double>& cut_new_bd)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default initialize_new_search_tree_node() executed.\n");
  }
}

//#############################################################################
// Opportunity to reset things before optimization
void
BCP_lp_user::modify_lp_parameters(OsiSolverInterface* lp,
				  bool in_strong_branching)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default prepare_for_optimization() executed.\n");
  }
}

//#############################################################################
// Generating a true lower bound
double
BCP_lp_user::compute_lower_bound(const double old_lower_bound,
				 const BCP_lp_result& lpres,
				 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts)
{
   // If columns are to be generated then we can't say anything, just return
   // the current lower bound
   if (p->node->colgen != BCP_DoNotGenerateColumns_Fathom)
      return old_lower_bound;

   // Otherwise we got the examine the termination code and the objective
   // value of the LP solution
   const int tc = lpres.termcode();
   if (tc & BCP_ProvenOptimal)
      return lpres.objval();

   // The limit (the upper bound) on the dual objective is proven to be
   // reached, but the objval might not reflect this! (the LP solver may not
   // make the last iteration that pushes objval over the limit). So we return
   // a high value ourselves.
   if (tc & BCP_DualObjLimReached)
      return p->ub() + 1e-5;

   // We can't say anything in any other case
   // (BCP_ProvenPrimalInf | BCP_ProvenDualInf | BCP_PrimalObjLimReached |
   //  BCP_TimeLimit | BCP_Abandoned), not to mention that some of these are
   //  impossible. Just return the current bound.
   return old_lower_bound;
}

//#############################################################################
// Feasibility testing
BCP_solution*
BCP_lp_user::test_feasibility(const BCP_lp_result& lpres,
			      const BCP_vec<BCP_var*>& vars,
			      const BCP_vec<BCP_cut*>& cuts)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default test_feasibility() executed.\n");
  }

  const double etol = p->param(BCP_lp_par::IntegerTolerance);
  BCP_feasibility_test test =
    static_cast<BCP_feasibility_test>(p->param(BCP_lp_par::FeasibilityTest));

  BCP_solution* sol = NULL;
  switch (test){
  case BCP_Binary_Feasible:   sol = test_binary(lpres, vars, etol); break;
  case BCP_Integral_Feasible: sol = test_integral(lpres, vars, etol); break;
  case BCP_FullTest_Feasible: sol = test_full(lpres, vars, etol); break;
  default:
    throw BCP_fatal_error("LP: unknown test_feasibility() rule.\n");
  }

  return sol;
}

//-----------------------------------------------------------------------------

BCP_solution_generic*
BCP_lp_user::test_binary(const BCP_lp_result& lpres,
			 const BCP_vec<BCP_var*>& vars,
			 const double etol) const
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default test_binary() executed.\n");
  }
  // Do anything only if the termination code is sensible
  const int tc = lpres.termcode();
  if (! (tc & BCP_ProvenOptimal))
     return 0;

  const double * x = lpres.x();
  const int varnum = vars.size();
  int i;

  for (i = 0 ; i < varnum; ++i) {
    const double xi = x[i];
    if ( (xi > etol && xi < 1 - etol) || (xi < -etol) || (xi > 1 + etol) )
      return(NULL);
  }

  // This solution does not own the pointers to the variables
  BCP_solution_generic* sol = new BCP_solution_generic(false); 
  double obj = 0;
  for (i = 0 ; i < varnum; ++i) {
    if (x[i] > 1 - etol) {
      sol->add_entry(vars[i], 1.0);
      obj += vars[i]->obj();
    }
  }
  sol->_objective = obj;
  return sol;
}
//-----------------------------------------------------------------------------
BCP_solution_generic*
BCP_lp_user::test_integral(const BCP_lp_result& lpres,
			   const BCP_vec<BCP_var*>& vars,
			   const double etol) const
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default test_integral() executed.\n");
  }
  // Do anything only if the termination code is sensible
  const int tc = lpres.termcode();
  if (! (tc & BCP_ProvenOptimal))
     return 0;

  const double * x = lpres.x();
  const int varnum = vars.size();
  int i;
  register double val;

  for (i = 0 ; i < varnum; ++i) {
    val = x[i];
    const BCP_var* vari = vars[i];
    if (val < vari->lb() - etol || val > vari->ub() + etol)
      return(NULL);
    val = val - floor(val);
    if (val > etol && val < 1 - etol)
      return(NULL);
  }
  
  // This solution does not own the pointers to the variables
  BCP_solution_generic* sol = new BCP_solution_generic(false);
  double obj = 0;
  for (i = 0 ; i < varnum; ++i) {
    val = floor(x[i] + 0.5);
    if (val < -etol || val > etol) { // could test != 0.0
      sol->add_entry(vars[i], val);
      obj += val * vars[i]->obj();
    }
  }
  sol->_objective = obj;
  return sol;
}
//-----------------------------------------------------------------------------
BCP_solution_generic*
BCP_lp_user::test_full(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var*>& vars,
		       const double etol) const
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default test_full() executed.\n");
  }
  // Do anything only if the termination code is sensible
  const int tc = lpres.termcode();
  if (! (tc & BCP_ProvenOptimal))
     return 0;

  const double * x = lpres.x();
  const int varnum = vars.size();
  const double etol1 = 1 - etol;
  int i;
  register double val;

  for (i = 0 ; i < varnum; ++i) {
    val = x[i];
    const BCP_var* vari = vars[i];
    if (val < vari->lb() - etol || val > vari->ub() + etol)
      return(NULL);
    switch (vari->var_type()){
    case BCP_BinaryVar:
      if (val > etol && val < etol1)
	return(NULL);
      break;
    case BCP_IntegerVar:
      val = val - floor(val);
      if (val > etol && val < etol1)
	return(NULL);
      break;
    case BCP_ContinuousVar:
      break;
    }
  }

  // This solution does not own the pointers to the variables
  BCP_solution_generic* sol = new BCP_solution_generic(false);
  double obj = 0;
  for (i = 0 ; i < varnum; ++i) {
    val = x[i];
    // round the integer vars
    if (vars[i]->var_type() == BCP_BinaryVar ||
	vars[i]->var_type() == BCP_IntegerVar)
      val = floor(x[i] + 0.5);
    if (val < -etol || val > etol) {
      sol->add_entry(vars[i], val);
      obj += val * vars[i]->obj();
    }
  }
  sol->_objective = obj;
  return sol;
}

//#############################################################################
// Heuristic solution generation
BCP_solution*
BCP_lp_user::generate_heuristic_solution(const BCP_lp_result& lpres,
					 const BCP_vec<BCP_var*>& vars,
					 const BCP_vec<BCP_cut*>& cuts)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default generate_heuristic_solution() executed.\n");
  }
  return NULL;
}
//#############################################################################
// Feasible solution packing
void
BCP_lp_user::pack_feasible_solution(BCP_buffer& buf, const BCP_solution* sol)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default pack_feasible_solution() executed.\n");
  }

  const BCP_solution_generic* gensol =
    dynamic_cast<const BCP_solution_generic*>(sol);
  const BCP_vec<BCP_var*> solvars = gensol->_vars;
  const BCP_vec<double> values = gensol->_values;
  const int size = solvars.size();
  buf.pack(size);
  BCP_lp_prob* p = getLpProblemPointer();
  for (int i = 0; i < size; ++i) {
    buf.pack(values[i]);
    p->pack_var(BCP_ProcessType_Any, *solvars[i]);
  }
}

//#############################################################################
// pack message for CG/CP

void
BCP_lp_user::pack_primal_solution(BCP_buffer& buf,
				  const BCP_lp_result& lpres,
				  const BCP_vec<BCP_var*>& vars,
				  const BCP_vec<BCP_cut*>& cuts)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default pack_for_cg() executed.\n");
  }

  BCP_temp_vec<int> tmp_coll;
  BCP_vec<int>& coll = tmp_coll.vec();

  const double petol = lpres.primalTolerance();
  const double * x = lpres.x();
  const int varnum = vars.size();

  switch (p->param(BCP_lp_par::InfoForCG)){
  case BCP_PrimalSolution_Nonzeros:
    select_nonzeros(x, x + varnum, petol, coll);
    buf.set_msgtag(BCP_Msg_ForCG_PrimalNonzeros);
    break;
  case BCP_PrimalSolution_Fractions:
    select_fractions(x, x+varnum, petol, coll);
    buf.set_msgtag(BCP_Msg_ForCG_PrimalFractions);
    break;
  case BCP_PrimalSolution_Full:
    coll.reserve(varnum);
    for (int i = 0; i < varnum; ++i) {
      coll.unchecked_push_back(i);
    }
    buf.set_msgtag(BCP_Msg_ForCG_PrimalFull);
    break;
  default:
    throw BCP_fatal_error("Incorrect msgtag in pack_for_cg() !\n");
  }

  const int size = coll.size();
  buf.pack(size);
  if (size > 0){
    BCP_vec<int>::const_iterator pos = coll.begin() - 1;
    const BCP_vec<int>::const_iterator last_pos = coll.end();
    while (++pos != last_pos) {
      buf.pack(x[*pos]);
      p->pack_var(BCP_ProcessType_Any, *vars[*pos]);
    }
  }
}

//=============================================================================
// pack message for VG/VP

void
BCP_lp_user::pack_dual_solution(BCP_buffer& buf,
				const BCP_lp_result& lpres,
				const BCP_vec<BCP_var*>& vars,
				const BCP_vec<BCP_cut*>& cuts)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default pack_for_vg() executed.\n");
  }

  BCP_temp_vec<int> tmp_coll;
  BCP_vec<int>& coll = tmp_coll.vec();

  const double detol = lpres.dualTolerance();
  const double * pi = lpres.pi();
  const int cutnum = cuts.size();

  switch (p->param(BCP_lp_par::InfoForVG)){
  case BCP_DualSolution_Nonzeros:
    select_nonzeros(pi, pi+cutnum, detol, coll);
    buf.set_msgtag(BCP_Msg_ForVG_DualNonzeros);
    break;
  case BCP_DualSolution_Full:
    coll.reserve(cutnum);
    for (int i = 0; i < cutnum; ++i) {
      coll.unchecked_push_back(i);
    }
    buf.set_msgtag(BCP_Msg_ForVG_DualFull);
    break;
  default:
    throw BCP_fatal_error("Incorrect msgtag in pack_lp_solution() !\n");
  }

  const int size = coll.size();
  buf.pack(size);
  if (size > 0){
    BCP_vec<int>::const_iterator pos = coll.begin() - 1;
    const BCP_vec<int>::const_iterator last_pos = coll.end();
    while (++pos != last_pos) {
      buf.pack(pi[*pos]);
      p->pack_cut(BCP_ProcessType_Any, *cuts[*pos]);
    }
  }
}

//#############################################################################
// lp solution displaying
void
BCP_lp_user::display_lp_solution(const BCP_lp_result& lpres,
				 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts,
				 const bool final_lp_solution)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default display_lp_solution() executed.\n");
  }

  if (final_lp_solution) {
    if (! p->param(BCP_lp_par::LpVerb_FinalRelaxedSolution))
      return;
    printf("  LP : Displaying LP solution (FinalRelaxedSolution) :\n");
  } else {
    if (! p->param(BCP_lp_par::LpVerb_RelaxedSolution))
      return;
    printf("  LP : Displaying LP solution (RelaxedSolution) :\n");
  }

  const double ietol = p->param(BCP_lp_par::IntegerTolerance);

  printf("  LP : Displaying solution :\n");
  BCP_temp_vec<int> tmp_coll;
  BCP_vec<int>& coll = tmp_coll.vec();
  const double * x = lpres.x();
  select_nonzeros(x, x+vars.size(), ietol, coll);
  const int size = coll.size();
  for (int i = 0; i < size; ++i) {
    const int ind = coll[i];
    vars[ind]->display(x[ind]);
  }
}

//#############################################################################

// Functions related to indexed vars. 
int
BCP_lp_user::next_indexed_var(int prev_index)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default next_indexed_var() executed.\n");
  }
   return -1;
}
//-----------------------------------------------------------------------------
BCP_var_indexed*
BCP_lp_user::create_indexed_var(int index,
				const BCP_vec<BCP_cut*>& cuts,
				BCP_col& col)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default create_indexed_var() executed.\n");
  }
   throw BCP_fatal_error("create_indexed_var() missing.\n");
}

//#############################################################################
// restoring feasibility
void
BCP_lp_user::restore_feasibility(const BCP_lp_result& lpres,
				 const std::vector<double*> dual_rays,
				 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts,
				 BCP_vec<BCP_var*>& vars_to_add,
				 BCP_vec<BCP_col*>& cols_to_add)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default restore_feasibility() executed.\n");
  }
}

//#############################################################################
//#############################################################################

void
BCP_lp_user::cuts_to_rows(const BCP_vec<BCP_var*>& vars, // on what to expand
			  BCP_vec<BCP_cut*>& cuts,       // what to expand
			  BCP_vec<BCP_row*>& rows,       // the expanded rows
			  // things that the user can use for lifting cuts
			  // if allowed
			  const BCP_lp_result& lpres,
			  BCP_object_origin origin, bool allow_multiple)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default cuts_to_rows() executed.\n");
  }
  throw BCP_fatal_error("cuts_to_rows() missing.\n");
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // on what to expand
			  BCP_vec<BCP_var*>& vars,       // what to expand
			  BCP_vec<BCP_col*>& cols,       // the expanded cols
			  // things that the user can use for lifting vars
			  // if allowed
			  const BCP_lp_result& lpres,
			  BCP_object_origin origin, bool allow_multiple)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default vars_to_cols() executed.\n");
  }
  throw BCP_fatal_error("vars_to_cols() missing.\n");
}

//#############################################################################

void
BCP_lp_user::generate_cuts_in_lp(const BCP_lp_result& lpres,
				 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts,
				 BCP_vec<BCP_cut*>& new_cuts,
				 BCP_vec<BCP_row*>& new_rows)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default generate_cuts_in_lp() executed.\n");
  }
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::generate_vars_in_lp(const BCP_lp_result& lpres,
				 const BCP_vec<BCP_var*>& vars,
				 const BCP_vec<BCP_cut*>& cuts,
				 const bool before_fathom,
				 BCP_vec<BCP_var*>& new_vars,
				 BCP_vec<BCP_col*>& new_cols)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default generate_vars_in_lp() executed.\n");
  }
}
//-----------------------------------------------------------------------------
BCP_object_compare_result
BCP_lp_user::compare_cuts(const BCP_cut* c0, const BCP_cut* c1)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default compare_cuts() executed.\n");
  }
   return BCP_DifferentObjs;
}
//-----------------------------------------------------------------------------
BCP_object_compare_result
BCP_lp_user::compare_vars(const BCP_var* v0, const BCP_var* v1)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default compare_vars() executed.\n");
  }
   return BCP_DifferentObjs;
}

//#############################################################################

void
BCP_lp_user::logical_fixing(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    const BCP_vec<BCP_obj_status>& var_status,
			    const BCP_vec<BCP_obj_status>& cut_status,
			    const int var_bound_changes_since_logical_fixing,
			    BCP_vec<int>& changed_pos,
			    BCP_vec<double>& new_bd)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default logical_fixing() executed.\n");
  }
}

//#############################################################################
void
BCP_lp_user::reduced_cost_fixing(const double* dj, const double* x,
				 const double gap,
				 BCP_vec<BCP_var*>& vars, int& newly_changed)
{
  newly_changed = 0;
  const bool atZero = get_param(BCP_lp_par::DoReducedCostFixingAtZero);
  const bool atAny = get_param(BCP_lp_par::DoReducedCostFixingAtAnything);

  if (! atZero && ! atAny)
     return;

  double petol = 0.0;
  p->lp_solver->getDblParam(OsiPrimalTolerance, petol);

  // If the gap is negative that means that we are above the limit, so
  // don't do anything.
  if (gap < 0)
    return;

  const int varnum = vars.size();
  BCP_temp_vec<int> tmp_changed_indices(varnum);
  BCP_vec<int>& changed_indices = tmp_changed_indices.vec();
  BCP_temp_vec<double> tmp_changed_bounds(2 * varnum);
  BCP_vec<double>& changed_bounds = tmp_changed_bounds.vec();

  // Note that when this function is called, we must have a dual
  // feasible dual solution. Therefore we can use the lagrangean
  // relaxation to fix variables.

  // *FIXME* : If we knew that there are integral vars only, then
  // we could leave out the test for BCP_ContinuousVar...
  for (int i = 0; i < varnum; ++i) {
    BCP_var* var = vars[i];
    if (! var->is_fixed() && var->var_type() != BCP_ContinuousVar){
      if (dj[i] > 0) {
	const double lb = var->lb();
	const double new_ub = lb + floor(gap / dj[i]);
	if (new_ub < var->ub() && (atAny || CoinAbs(x[i])<petol) ) {
	  vars[i]->set_ub(new_ub);
	  changed_indices.unchecked_push_back(i);
	  changed_bounds.unchecked_push_back(lb);
	  changed_bounds.unchecked_push_back(new_ub);
	}
      } else if (dj[i] < 0) {
	const double ub = var->ub();
	const double new_lb = ub - floor(gap / (-dj[i]));
	if (new_lb > var->lb() && (atAny || CoinAbs(x[i])<petol) ) {
	  vars[i]->set_lb(new_lb);
	  changed_indices.unchecked_push_back(i);
	  changed_bounds.unchecked_push_back(new_lb);
	  changed_bounds.unchecked_push_back(ub);
	}
      }
    }
  }
  newly_changed = changed_indices.size();
  if (newly_changed > 0) {
    p->lp_solver->setColSetBounds(changed_indices.begin(),
				  changed_indices.end(),
				  changed_bounds.begin());
  }
}

//#############################################################################
//#############################################################################

BCP_branching_decision BCP_lp_user::
select_branching_candidates(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    const BCP_lp_var_pool& local_var_pool,
			    const BCP_lp_cut_pool& local_cut_pool,
			    BCP_vec<BCP_lp_branching_object*>& cans)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default select_branching_candidates() executed.\n");
  }

  // *THINK* : this branching object selection is very primitive
  // *THINK* : should check for tail-off, could check for branching cuts, etc.
  if (local_var_pool.size() > 0 || local_cut_pool.size() > 0)
    return BCP_DoNotBranch;

  if (p->param(BCP_lp_par::StrongBranch_CloseToHalfNum) > 0)
    branch_close_to_half(lpres, vars,
			 p->param(BCP_lp_par::StrongBranch_CloseToHalfNum),
			 p->param(BCP_lp_par::IntegerTolerance),
			 cans);
  if (p->param(BCP_lp_par::StrongBranch_CloseToOneNum) > 0)
    branch_close_to_one(lpres, vars,
			p->param(BCP_lp_par::StrongBranch_CloseToOneNum),
			p->param(BCP_lp_par::IntegerTolerance),
			cans);
  // Get rid of duplicates in cans
  BCP_vec<int> brvars;
  brvars.reserve(cans.size());
  for (size_t i = 0; i < cans.size(); ) {
     const int brvar = (*cans[i]->forced_var_pos)[0];
     if (std::find(brvars.begin(), brvars.end(), brvar) == brvars.end()) {
	brvars.unchecked_push_back(brvar);
	++i;
     } else {
	std::swap(cans[i], cans.back());
	cans.pop_back();
     }
  }
  
  if (cans.size() == 0) {
    throw BCP_fatal_error("\
 LP : No var/cut in pool but couldn't select branching object.\n");
  }
  return BCP_DoBranch;
}

//-----------------------------------------------------------------------------
// A helper function for select_branching_candidates()
void
BCP_lp_user::branch_close_to_half(const BCP_lp_result& lpres,
				  const BCP_vec<BCP_var*>& vars,
				  const int to_be_selected,
				  const double etol,
				  BCP_vec<BCP_lp_branching_object*>& cans)
{
  // order the variables based on their distance from a MIP feasible value
  const double etol5 = .5 - etol;
  BCP_vec<BCP_var*>::const_iterator vi = vars.begin();
  const double * x = lpres.x();
  const double * xi = x;
  const double * lastxi = x + vars.size();

  // choose 5 times the required ones (ordered primarily by closeness to half
  // and secondarily by cost)
  int to_select = 5*to_be_selected;

  BCP_temp_vec<int> tmp_select_pos(to_select + 1, -1);
  BCP_vec<int>& select_pos = tmp_select_pos.vec();
  BCP_temp_vec<double> tmp_select_val(to_select + 1, 1.0);
  BCP_vec<double>& select_val = tmp_select_val.vec();
  BCP_temp_vec<double> tmp_select_cost(to_select + 1, -1e20);
  BCP_vec<double>& select_cost = tmp_select_cost.vec();

  const double* objcoeff = p->lp_solver->getObjCoefficients();

  double val;
  int pos, i, k;

  for (pos = 0, k = 0; k < to_select && xi != lastxi; ++pos, ++xi, ++vi){
    // check if the next var violates MIP feasibility
    if ((*vi)->var_type() == BCP_ContinuousVar)
      continue;
    val = CoinAbs(*xi - .5 - floor(*xi));
    if (val > etol5)
      continue;
    // if not, insert it into the list
    const double cost = CoinAbs(objcoeff[pos]);
    for (i = k - 1; i >= 0; --i) {
      if (select_val[i] < val ||
	  (select_val[i] == val && select_cost[i] >= cost))
	break;
      select_pos[i+1] = select_pos[i];
      select_val[i+1] = select_val[i];
      select_cost[i+1] = select_cost[i];
    }
    select_pos[i+1] = pos;
    select_val[i+1] = val;
    select_cost[i+1] = cost;
    ++k;
  }

  for ( ; xi != lastxi; ++pos, ++xi, ++vi) {
    // check if the next var violates MIP feasibility
    if ((*vi)->var_type() == BCP_ContinuousVar)
      continue;
    val = CoinAbs(*xi - .5 - floor(*xi));
    if (val > etol5)
      continue;
    // if not, insert it into the list
    const double cost = objcoeff[pos];
    for (i = to_select - 1; i >= 0; --i) {
      if (select_val[i] < val ||
	  (select_val[i] == val && select_cost[i] >= cost))
	break;
      select_pos[i+1] = select_pos[i];
      select_val[i+1] = select_val[i];
      select_cost[i+1] = select_cost[i];
    }
    if (i != to_select - 1) {
      select_pos[i+1] = pos;
      select_val[i+1] = val;
      select_cost[i+1] = cost;
    }
  }

  k = select_val.index(std::upper_bound(select_val.begin(),
					select_val.entry(k),
					2 * select_val[to_be_selected - 1]));

  select_pos.erase(select_pos.entry(k), select_pos.end());
  select_cost.erase(select_cost.entry(k), select_cost.end());

  // now reorder these and keep only as many as required. this time order by
  // cost alone.
  if (k > to_be_selected) {
    BCP_vec<double>::iterator di;
    BCP_temp_vec<int> tmp_new_pos(to_be_selected);
    BCP_vec<int>& new_pos = tmp_new_pos.vec();
    BCP_vec<double>& new_cost = select_val;
    new_cost.clear();
    for (pos = 0; pos < to_be_selected; ++pos) {
      const double val = select_cost[pos];
      // insert the next into the list
      di = std::upper_bound(new_cost.begin(), new_cost.end(), val);
      new_pos.insert(new_pos.entry(new_cost.index(di)), select_pos[pos]);
      new_cost.insert(di, val);
    }
    for ( ; pos < k; ++pos) {
      const double val = select_cost[pos];
      // insert the next into the list
      di = std::upper_bound(new_cost.begin(), new_cost.end(), val);
      if (di != new_cost.end()) {
	new_cost.pop_back();
	new_pos.pop_back();
	new_pos.insert(new_pos.entry(new_cost.index(di)), select_pos[pos]);
	new_cost.insert(di, val);
      }
    }
    select_pos.swap(new_pos);
  }

  append_branching_vars(lpres.x(), vars, select_pos, cans);
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::branch_close_to_one(const BCP_lp_result& lpres,
				 const BCP_vec<BCP_var*>& vars,
				 const int to_be_selected,
				 const double etol,
				 BCP_vec<BCP_lp_branching_object*>& cans)
{
  // order the variables based on their distance from a MIP feasible value
  BCP_vec<BCP_var*>::const_iterator vi = vars.begin();
  const double * x = lpres.x();
  const double * xi = x;
  const double * lastxi = x + vars.size();

  BCP_temp_vec<int> tmp_select_pos(to_be_selected);
  BCP_vec<int>& select_pos = tmp_select_pos.vec();
  BCP_temp_vec<double> tmp_select_val(to_be_selected);
  BCP_vec<double>& select_val = tmp_select_val.vec();
  BCP_vec<double>::iterator spv;

  double val;
  int pos;

  for (pos = 0;
       select_pos.size() < static_cast<const size_t>(to_be_selected) &&
	 xi != lastxi;
       ++pos, ++xi, ++vi) {
    // check if the next var violates MIP feasibility
    if ((*vi)->var_type() == BCP_ContinuousVar)
      continue;
    if (*xi < etol)
       continue;
    val = 1 - *xi;
    if (val < etol)
      continue;
    // if not, insert it into the list
    spv = std::upper_bound(select_val.begin(), select_val.end(), val);
    select_pos.insert(select_pos.entry(select_val.index(spv)), pos);
    select_val.insert(spv, val);
  }

  for ( ; xi != lastxi; ++pos, ++xi, ++vi) {
    // check if the next var violates MIP feasibility
    if ((*vi)->var_type() == BCP_ContinuousVar)
      continue;
    if (*xi < etol)
       continue;
    val = 1 - *xi;
    if (val < etol)
      continue;
    // if not, insert it into the list
    spv = std::upper_bound(select_val.begin(), select_val.end(), val);
    if (spv != select_val.end()) {
      select_pos.pop_back();
      select_pos.insert(select_pos.entry(select_val.index(spv)), pos);
      select_val.pop_back();
      select_val.insert(spv, val);
    }
  }

  append_branching_vars(lpres.x(), vars, select_pos, cans);
}
//-----------------------------------------------------------------------------
void
BCP_lp_user::append_branching_vars(const double* x,
				   const BCP_vec<BCP_var*>& vars,
				   const BCP_vec<int>& select_pos,
				   BCP_vec<BCP_lp_branching_object*>& cans)
{
  BCP_vec<int>::const_iterator spi;
  BCP_vec<double> vbd(4, 0.0);
  BCP_vec<int> vpos(1, 0);

  for (spi = select_pos.begin(); spi != select_pos.end(); ++spi) {
    const int pos = *spi;
    vpos[0] = pos;
    vbd[0] = vars[pos]->lb();
    vbd[1] = floor(x[pos]);
    vbd[2] = ceil(x[pos]);
    vbd[3] = vars[pos]->ub();
    cans.push_back
      (new  BCP_lp_branching_object(2,
				    0, 0, /* vars/cuts_to_add */
				    &vpos, 0, &vbd, 0, /* forced parts */
				    0, 0, 0, 0 /* implied parts */));
  }
}

//-----------------------------------------------------------------------------
BCP_branching_object_relation BCP_lp_user::
compare_branching_candidates(BCP_presolved_lp_brobj* new_presolved,
			     BCP_presolved_lp_brobj* old_presolved)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default compare_presolved_branching_objects() executed.\n");
  }

  // change the objvals according to the termcodes (in order to be able to
  // make decisions based on objval, no matter what the termcode is).
  new_presolved->fake_objective_values(p->lp_result->objval());

   // If using this branching object we can fathom all children, then choose
   // this object
  if (new_presolved->fathomable(p->ub() - p->granularity()))
    return(BCP_NewPresolvedIsBetter_BranchOnIt);

   // If this is the first, keep it
  if (! old_presolved)
    return(BCP_NewPresolvedIsBetter);

   // If something had gone wrong with at least one descendant in the new then
   // we prefer to keep the old.
  if (new_presolved->had_numerical_problems())
    return(BCP_OldPresolvedIsBetter);

   // OK, so all descendants finished just fine. Do whatever the parameter says

  if (p->param(BCP_lp_par::BranchingObjectComparison) & BCP_Comparison_Objval){
    const double objetol = 1e-7;
      
    BCP_temp_vec<double> tmp_new_obj;
    BCP_vec<double>& new_obj = tmp_new_obj.vec();
    new_presolved->get_lower_bounds(new_obj);
    std::sort(new_obj.begin(), new_obj.end());
    const int new_not_fathomed =
      new_obj.index(std::lower_bound(new_obj.begin(), new_obj.end(),
				     DBL_MAX / 4));

    BCP_temp_vec<double> tmp_old_obj;
    BCP_vec<double>& old_obj = tmp_old_obj.vec();
    old_presolved->get_lower_bounds(old_obj);
    std::sort(old_obj.begin(), old_obj.end());
    const int old_not_fathomed =
      old_obj.index(std::lower_bound(old_obj.begin(), old_obj.end(),
				     DBL_MAX / 4));

    if (new_not_fathomed < old_not_fathomed)
      return BCP_NewPresolvedIsBetter;

    if (new_not_fathomed > old_not_fathomed)
      return BCP_OldPresolvedIsBetter;

    BCP_vec<double>::const_iterator new_first = new_obj.begin();
    BCP_vec<double>::const_iterator new_last = new_obj.end();
    BCP_vec<double>::const_iterator old_first = old_obj.begin();
    BCP_vec<double>::const_iterator old_last = old_obj.end();
   
    switch(p->param(BCP_lp_par::BranchingObjectComparison)){
    case BCP_LowestAverageObjval:
    case BCP_HighestAverageObjval:
      {
	double newavg = 0;
	for ( ; new_first != new_last; ++new_first) {
	  if (*new_first < DBL_MAX / 4)
	    newavg += *new_first;
	}
	newavg /= new_not_fathomed;
	double oldavg = 0;
	for ( ; old_first != old_last; ++old_first) {
	  if (*old_first < DBL_MAX / 4)
	    oldavg += *old_first;
	}
	oldavg /= old_not_fathomed;
	const bool high = (p->param(BCP_lp_par::BranchingObjectComparison)
			   == BCP_HighestAverageObjval);
	return (high && newavg > oldavg) || (!high && newavg < oldavg) ?
	  BCP_NewPresolvedIsBetter : BCP_OldPresolvedIsBetter;
      }

    case BCP_LowestLowObjval:
      for ( ; new_first != new_last && old_first != old_last;
	    ++new_first, ++old_first) {
	if (*new_first < *old_first - objetol)
	  return BCP_NewPresolvedIsBetter;
	if (*new_first > *old_first + objetol)
	  return BCP_OldPresolvedIsBetter;
      }
      return old_first == old_last ?
	BCP_OldPresolvedIsBetter : BCP_NewPresolvedIsBetter;

    case BCP_HighestLowObjval:
      for ( ; new_first != new_last && old_first != old_last;
	    ++new_first, ++old_first) {
	if (*new_first > *old_first + objetol)
	  return BCP_NewPresolvedIsBetter;
	if (*new_first < *old_first - objetol)
	  return BCP_OldPresolvedIsBetter;
      }
      return old_first == old_last ?
	BCP_OldPresolvedIsBetter : BCP_NewPresolvedIsBetter;

    case BCP_LowestHighObjval:
      while (new_first != new_last && old_first != old_last) {
	--new_last;
	--old_last;
	if (*new_last < *old_last - objetol)
	  return BCP_NewPresolvedIsBetter;
	if (*new_last > *old_last + objetol)
	  return BCP_OldPresolvedIsBetter;
      }
      return old_first == old_last ?
	BCP_OldPresolvedIsBetter : BCP_NewPresolvedIsBetter;

    case BCP_HighestHighObjval:
      while (new_first != new_last && old_first != old_last) {
	--new_last;
	--old_last;
	if (*new_last > *old_last + objetol)
	  return BCP_NewPresolvedIsBetter;
	if (*new_last < *old_last - objetol)
	  return BCP_OldPresolvedIsBetter;
      }
      return old_first == old_last ?
	BCP_OldPresolvedIsBetter : BCP_NewPresolvedIsBetter;
    default:
      throw BCP_fatal_error("Unknown branching object comparison rule.\n");
    }
  }

  // shouldn't ever get here
  throw BCP_fatal_error("No branching object comparison rule is chosen.\n");

  // fake return
  return BCP_NewPresolvedIsBetter;
}

//-----------------------------------------------------------------------------
void
BCP_lp_user::set_actions_for_children(BCP_presolved_lp_brobj* best)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default set_actions_for_children() executed.\n");
  }

  // by default every action is set to BCP_ReturnChild
  if (p->node->dive == BCP_DoNotDive)
    return;
   
  BCP_vec<BCP_child_action>& action = best->action();
  int i, ind;

  switch (p->param(BCP_lp_par::ChildPreference)){
  case BCP_PreferChild_LowBound:
    // NOTE: if the lowest objval child is fathomed then everything is
    for (ind = 0, i = best->candidate()->child_num - 1; i > 0; --i){
      if (best->lpres(i).objval() < best->lpres(ind).objval())
	ind = i;
    }
    break;

  case BCP_PreferChild_HighBound:
    // NOTE: this selects the highest objval child NOT FATHOMED, thus if
    // the highest objval child is fathomed then everything is
    for (ind = 0, i = best->candidate()->child_num - 1; i > 0; --i) {
      if (! p->over_ub(best->lpres(i).objval()) &&
	  best->lpres(i).objval() < best->lpres(ind).objval())
	ind = i;
    }
    break;
  default:
    // *THINK* : fractional branching
    throw BCP_fatal_error("LP : Unrecognized child preference.\n");
  }
  // mark the ind-th to be kept (if not over ub)
  if (! p->over_ub(best->lpres(ind).objval()))
    action[ind] = BCP_KeepChild;
}

//#############################################################################
// purging the slack cut pool (candidates for branching on cut)
void
BCP_lp_user::purge_slack_pool(const BCP_vec<BCP_cut*>& slack_pool,
			      BCP_vec<int>& to_be_purged)
{
  if (p->param(BCP_lp_par::ReportWhenDefaultIsExecuted)) {
    printf(" LP: Default purge_slack_pool() executed.\n");
  }

  switch (p->param(BCP_lp_par::SlackCutDiscardingStrategy)) {
  case BCP_DiscardSlackCutsAtNewNode:
    if (current_iteration() != 1)
      break;
  case BCP_DiscardSlackCutsAtNewIteration:
    {
      const int size = slack_pool.size();
      if (size > 0) {
	to_be_purged.reserve(size);
	for (int i = 0; i < size; ++i)
	  to_be_purged.unchecked_push_back(i);
      }
    }
    break;
  }
}

//#############################################################################
