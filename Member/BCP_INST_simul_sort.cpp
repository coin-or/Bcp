// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_os.hpp"

#ifdef NEED_TEMPLATE_CLASSES
#include "templates/BCP_vector_general.cpp"
template class BCP_vec< std::pair<double, int> >;
template class BCP_vec< std::pair<int, double> >;
#endif

#ifdef NEED_TEMPLATE_FUNCTIONS
#include "templates/BCP_simul_sort.cpp"
template void
BCP_simul_sort(double *, double *, int *);
template void
BCP_simul_sort(int *, int *, double *);
template void
BCP_simul_sort(int *, int *, int *);
#endif
