// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_os.hpp"

#ifdef NEED_TEMPLATE_CLASSES

//#############################################################################

#include <utility> // for pair<>
#include "BCP_string.hpp"
#include "BCP_parameters.hpp"
#include "BCP_message.hpp"

#include "templates/BCP_vector_general.cpp"

template class BCP_vec<BCP_string>;
template class BCP_vec< std::pair<int, int> >;
template class BCP_vec< std::pair<BCP_string, BCP_parameter> >;
template class BCP_vec< std::pair<BCP_proc_id*, int> >;

#include "BCP_enum.hpp"
template class BCP_vec<BCP_obj_status>;

#include "BCP_enum_branch.hpp"
template class BCP_vec<BCP_child_action>;

#include "BCP_obj_change.hpp"
template class BCP_vec<BCP_obj_change>;

//#############################################################################

#endif
