// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_os.hpp"

#ifdef NEED_TEMPLATE_CLASSES

//#############################################################################

#include "templates/BCP_vector_bool.cpp"
template class BCP_vec<bool>;

#include "templates/BCP_vector_char.cpp"
template class BCP_vec<char>;

#include "templates/BCP_vector_short.cpp"
template class BCP_vec<short>;

#include "templates/BCP_vector_int.cpp"
template class BCP_vec<int>;

#include "templates/BCP_vector_double.cpp"
template class BCP_vec<double>;

//#############################################################################

#endif
