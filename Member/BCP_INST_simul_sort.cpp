// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "templates/BCP_vector_general.cpp"

using std::pair;

template class BCP_vec< pair<double, int> >;
template class BCP_vec< pair<int, double> >;

#include "templates/BCP_simul_sort.cpp"
template void
BCP_simul_sort(double *, double *, int *);
template void
BCP_simul_sort(int *, int *, double *);
template void
BCP_simul_sort(int *, int *, int *);
