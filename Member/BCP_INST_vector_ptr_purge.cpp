// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_os.hpp"

#ifdef NEED_TEMPLATE_FUNCTIONS

//#############################################################################

#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_lp_branch.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_lp_pool.hpp"
#include "BCP_tm_node.hpp"
#include "BCP_vector.hpp"

template void keep_ptr_vector_by_index(BCP_vec<BCP_var *> &,
				       BCP_vec<int>::const_iterator,
				       BCP_vec<int>::const_iterator);
template void keep_ptr_vector_by_index(BCP_vec<BCP_col *> &,
				       BCP_vec<int>::const_iterator,
				       BCP_vec<int>::const_iterator);

template void purge_ptr_vector_by_index(BCP_vec<BCP_var *> &,
				       BCP_vec<int>::const_iterator,
				       BCP_vec<int>::const_iterator);
template void purge_ptr_vector_by_index(BCP_vec<BCP_cut *> &,
				       BCP_vec<int>::const_iterator,
				       BCP_vec<int>::const_iterator);

template void purge_ptr_vector(BCP_vec<BCP_var *> &);
template void purge_ptr_vector(BCP_vec<BCP_var_core *> &);
template void purge_ptr_vector(BCP_vec<BCP_var_indexed *> &);

template void purge_ptr_vector(BCP_vec<BCP_cut *> &);
template void purge_ptr_vector(BCP_vec<BCP_cut_core *> &);

template void purge_ptr_vector(BCP_vec<BCP_var *> &,
			       BCP_vec<BCP_var *>::iterator,
			       BCP_vec<BCP_var *>::iterator);
template void purge_ptr_vector(BCP_vec<BCP_cut *> &,
			       BCP_vec<BCP_cut *>::iterator,
			       BCP_vec<BCP_cut *>::iterator);

template void purge_ptr_vector(BCP_vec<BCP_lp_branching_object *> &,
			       BCP_vec<BCP_lp_branching_object *>::iterator,
			       BCP_vec<BCP_lp_branching_object *>::iterator);

template void purge_ptr_vector(BCP_vec<BCP_lp_result *> &);

template void purge_ptr_vector(BCP_vec<BCP_lp_waiting_row *> &);
template void purge_ptr_vector(BCP_vec<BCP_lp_waiting_row *> &,
			       BCP_vec<BCP_lp_waiting_row *>::iterator,
			       BCP_vec<BCP_lp_waiting_row *>::iterator);

template void purge_ptr_vector(BCP_vec<BCP_lp_waiting_col *> &);
template void purge_ptr_vector(BCP_vec<BCP_lp_waiting_col *> &,
			       BCP_vec<BCP_lp_waiting_col *>::iterator,
			       BCP_vec<BCP_lp_waiting_col *>::iterator);

template void purge_ptr_vector(BCP_vec<BCP_col *> &);
template void purge_ptr_vector(BCP_vec<BCP_row *> &);

//#############################################################################

#endif
