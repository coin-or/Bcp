// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_temporary.hpp"

template class BCP_temp_vec<char>;
template<> BCP_vec<BCP_vec<char>*> BCP_temp_vec<char>::_temp_vecs;

template class BCP_temp_vec<short>;
template<> BCP_vec<BCP_vec<short>*> BCP_temp_vec<short>::_temp_vecs;

template class BCP_temp_vec<int>;
template<> BCP_vec<BCP_vec<int>*> BCP_temp_vec<int>::_temp_vecs;

template class BCP_temp_vec<double>;
template<> BCP_vec<BCP_vec<double>*> BCP_temp_vec<double>::_temp_vecs;
