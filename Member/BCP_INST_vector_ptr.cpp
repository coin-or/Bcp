// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_vector.hpp"
#include "templates/BCP_vector_general.cpp"

//#############################################################################

template class BCP_vec<BCP_vec<char>*>;
template class BCP_vec<BCP_vec<short>*>;
template class BCP_vec<BCP_vec<int>*>;
template class BCP_vec<BCP_vec<double>*>;

//#############################################################################

class BCP_var;
class BCP_var_core;
class BCP_var_indexed;

class BCP_cut;
class BCP_cut_core;

class BCP_col;
class BCP_row;

class BCP_lp_waiting_col;
class BCP_lp_waiting_row;

class BCP_lp_result;
class BCP_lp_branching_object;

class BCP_proc_id;

class BCP_tm_node;

//#############################################################################

template class BCP_vec<BCP_var*>;
template class BCP_vec<BCP_var_core*>;
template class BCP_vec<BCP_var_indexed*>;
template class BCP_vec<const BCP_var_indexed*>;

template class BCP_vec<BCP_cut*>;
template class BCP_vec<BCP_cut_core*>;

template class BCP_vec<BCP_lp_result*>;

template class BCP_vec<BCP_col*>;
template class BCP_vec<BCP_row*>;

template class BCP_vec<BCP_lp_waiting_col*>;
template class BCP_vec<BCP_lp_waiting_row*>;

template class BCP_vec<BCP_lp_branching_object*>;

template class BCP_vec<BCP_proc_id*>;

template class BCP_vec<BCP_tm_node*>;
