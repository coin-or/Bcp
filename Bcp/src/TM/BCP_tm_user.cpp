// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "CoinTime.hpp"

#include "BCP_vector.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_tm.hpp"
#include "BCP_lp.hpp"
#include "BCP_solution.hpp"
#include "BCP_var.hpp"
#include "BCP_functions.hpp"

//#############################################################################
// Informational methods for the user
double BCP_tm_user::upper_bound() const { return p->ub(); }

//#############################################################################
// Informational methods for the user
/* Methods to get/set BCP parameters on the fly */
char
BCP_tm_user::get_param(const BCP_tm_par::chr_params key) const
{ return p->par.entry(key); }
int
BCP_tm_user::get_param(const BCP_tm_par::int_params key) const
{ return p->par.entry(key); }
double
BCP_tm_user::get_param(const BCP_tm_par::dbl_params key) const
{ return p->par.entry(key); }
const BCP_string&
BCP_tm_user::get_param(const BCP_tm_par::str_params key) const
{ return p->par.entry(key); }

void BCP_tm_user::set_param(const BCP_tm_par::chr_params key, const bool val)
{ p->par.set_entry(key, val); }
void BCP_tm_user::set_param(const BCP_tm_par::chr_params key, const char val)
{ p->par.set_entry(key, val); }
void BCP_tm_user::set_param(const BCP_tm_par::int_params key, const int val)
{ p->par.set_entry(key, val); }
void BCP_tm_user::set_param(const BCP_tm_par::dbl_params key, const double val)
{ p->par.set_entry(key, val); }
void BCP_tm_user::set_param(const BCP_tm_par::str_params key, const char * val)
{ p->par.set_entry(key, val); }

//#############################################################################

void
BCP_tm_user::pack_module_data(BCP_buffer& buf, BCP_process_t ptype) {}

//-----------------------------------------------------------------------------
// unpack an MIP feasible solution
BCP_solution*
BCP_tm_user::unpack_feasible_solution(BCP_buffer& buf)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default unpack_feasible_solution() executed.\n");
    }

    BCP_solution_generic* soln = new BCP_solution_generic;

    int varnum;
    buf.unpack(varnum);

    double val;
    int bcpind;
    while (--varnum >= 0) {
	buf.unpack(val);
	// these vars are stored only in the solution, so noone cares if we flip
	// negative bcpind's
	buf.unpack(bcpind);
	BCP_var* var = p->unpack_var_without_bcpind(buf);
	var->set_bcpind(bcpind < 0 ? -bcpind : bcpind);
	soln->add_entry(var, val);
    }

    return soln;
}

//-----------------------------------------------------------------------------

bool
BCP_tm_user::replace_solution(const BCP_solution* old_sol,
			      const BCP_solution* new_sol)
{
    return false;
}

//#############################################################################

void
BCP_tm_user::pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default BCP_tm_user::pack_warmstart() executed.\n");
    }
    BCP_pack_warmstart(ws, buf);
}

//-----------------------------------------------------------------------------

BCP_warmstart*
BCP_tm_user::unpack_warmstart(BCP_buffer& buf)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default BCP_tm_user::unpack_warmstart() executed.\n");
    }
    return BCP_unpack_warmstart(buf);
}

//#############################################################################

void
BCP_tm_user::pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::pack_var_algo() invoked but not overridden!\n");
}

//-----------------------------------------------------------------------------
BCP_var_algo*
BCP_tm_user::unpack_var_algo(BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::unpack_var_algo() invoked but not overridden!\n");
    return 0; // to satisfy aCC on HP-UX
}
      
//-----------------------------------------------------------------------------
void
BCP_tm_user::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::pack_cut_algo() invoked but not overridden!\n");
}

//-----------------------------------------------------------------------------
BCP_cut_algo*
BCP_tm_user::unpack_cut_algo(BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::unpack_cut_algo() invoked but not overridden!\n");
    return 0; // to satisfy aCC on HP-UX
}

//-----------------------------------------------------------------------------
void
BCP_tm_user::pack_user_data(const BCP_user_data* ud, BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::pack_user_data() invoked but not overridden!\n");
}

//-----------------------------------------------------------------------------
BCP_user_data*
BCP_tm_user::unpack_user_data(BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::unpack_user_data() invoked but not overridden!\n");
    return 0; // to satisfy aCC on HP-UX
}

//#############################################################################

/** What is the process id of the current process */
const BCP_proc_id*
BCP_tm_user::process_id() const
{
    return p->get_process_id();
}

/** Send a message to a particular process */
void
BCP_tm_user::send_message(const BCP_proc_id* const target,
			  const BCP_buffer& buf)
{
    p->msg_env->send(target, BCP_Msg_User, buf);
}    

/** Broadcast the message to all processes of the given type */
void
BCP_tm_user::broadcast_message(const BCP_process_t proc_type,
			       const BCP_buffer& buf)
{
    switch (proc_type) {
    case BCP_ProcessType_LP:
	p->msg_env->multicast(p->slaves.lp, BCP_Msg_User, buf);
	break;
    case BCP_ProcessType_CP:
	throw BCP_fatal_error("\
BCP_tm_user::broadcast_message: CP not yet implemented\n");
	break;
    case BCP_ProcessType_VP:
	throw BCP_fatal_error("\
BCP_tm_user::broadcast_message: VP not yet implemented\n");
	break;
    case BCP_ProcessType_CG:
	p->msg_env->multicast(p->slaves.cg, BCP_Msg_User, buf);
	break;
    case BCP_ProcessType_VG:
	p->msg_env->multicast(p->slaves.vg, BCP_Msg_User, buf);
	break;
    case BCP_ProcessType_Any:
	p->msg_env->multicast(p->slaves.lp, BCP_Msg_User, buf);
	p->msg_env->multicast(p->slaves.cg, BCP_Msg_User, buf);
	p->msg_env->multicast(p->slaves.vg, BCP_Msg_User, buf);
	break;
    case BCP_ProcessType_TM:
	throw BCP_fatal_error("\
BCP_tm_user::broadcast_message: broadcast to TM itself?!...\n");
    }
}

/** Process a message that has been sent by another process' user part to
    this process' user part. */
void
BCP_tm_user::process_message(BCP_buffer& buf)
{
    throw BCP_fatal_error("\
BCP_tm_user::process_message() invoked but not overridden!\n");
}

//#############################################################################
// setting the core

void
BCP_tm_user::initialize_core(BCP_vec<BCP_var_core*>& vars,
			     BCP_vec<BCP_cut_core*>& cuts,
			     BCP_lp_relax*& matrix)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default BCP_tm_user::initialize_core() executed.\n");
    }
}

//--------------------------------------------------------------------------
// create the root node
void
BCP_tm_user::create_root(BCP_vec<BCP_var*>& added_vars,
			 BCP_vec<BCP_cut*>& added_cuts,
			 BCP_user_data*& user_data,
			 BCP_pricing_status& pricing_status)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default BCP_tm_user::create_root() executed.\n");
    }
}

//--------------------------------------------------------------------------
// display a feasible solution
void
BCP_tm_user::display_feasible_solution(const BCP_solution* sol)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf("\
 TM: Default BCP_tm_user::display_feasible_solution() executed.\n");
    }

    const BCP_solution_generic* gsol =
	dynamic_cast<const BCP_solution_generic*>(sol);
    if (! gsol) {
	throw BCP_fatal_error("\
BCP_tm_user::display_feasible_solution() invoked with non-generic sol.\n");
    }

    gsol->display();
}

//-----------------------------------------------------------------------------
/** Display user information just before a new node is sent to the LP or
    diving into a node is acknowledged. */
void
BCP_tm_user::display_node_information(BCP_tree& search_tree,
				      const BCP_tm_node& node)
{
}
    
//-----------------------------------------------------------------------------
/** Display information after BCP finished processing the search tree. */
void
BCP_tm_user::display_final_information(const BCP_lp_statistics& lp_stat)
{
    if (p->param(BCP_tm_par::TmVerb_FinalStatistics)) {
	printf("TM: Running time: %.3f\n", CoinCpuTime() - p->start_time);
	printf("TM: search tree size: %i   max depth: %i\n",
	       int(p->search_tree.size()), p->search_tree.maxdepth());
	lp_stat.display();

	if (! p->feas_sol) {
	    printf("TM: No feasible solution is found\n");
	} else {
	    printf("TM: The best solution found has value %f\n",
		   p->feas_sol->objective_value());
	    if (p->param(BCP_tm_par::TmVerb_BestFeasibleSolution)) {
		p->user->display_feasible_solution(p->feas_sol);
	    }
	}
    }
}
    
//--------------------------------------------------------------------------
// Initialize new phase 
void
BCP_tm_user::init_new_phase(int phase, BCP_column_generation& colgen)
{
    if (p->param(BCP_tm_par::ReportWhenDefaultIsExecuted)) {
	printf(" TM: Default init_new_phase() executed.\n");
    }
    colgen = BCP_DoNotGenerateColumns_Fathom;
}

//--------------------------------------------------------------------------
// Compare tree nodes
bool
BCP_tm_user::compare_tree_nodes(const BCP_tm_node* node0,
				const BCP_tm_node* node1)
{
    switch (p->param(BCP_tm_par::TreeSearchStrategy)) {
    case BCP_BestFirstSearch:
	return node0->quality() < node1->quality();
    case BCP_BreadthFirstSearch:
	return node0->index() < node1->index();
    case BCP_DepthFirstSearch:
	return node0->index() > node1->index();
    }
    // fake return
    return true;
}
