// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_os.hpp"

//#############################################################################
#ifdef NEED_EXPLICIT_INSTANTIATION

#include <algorithm>
#include <numeric>

#ifdef NEED_IMPLICIT_TEMPLATE_FUNCTIONS
template const int *
std::find(int const *, int const *, int const &, random_access_iterator_tag);

template double
std::inner_product(double const *, double const *, double const *, double);
#endif

//#############################################################################

#include "MC_cut.hpp"

//#############################################################################

#include "MC.hpp"
#include "BCP_vector.hpp"
#include "templates/BCP_vector_general.cpp"

template class BCP_vec<MC_graph_edge>;
template class BCP_vec<MC_graph_node>;

//#############################################################################

template double 
inner_product(double *, double *, double *, double);

template double *
rotate(double *, double *, double *);
template double *
__rotate(double *, double *, double *,
         BCP_PtrDiff *, bidirectional_iterator_tag);

//#############################################################################

// beats me why is this needed...

template void
__reverse(double *, double *, bidirectional_iterator_tag);
template void
__reverse(double *, double *, random_access_iterator_tag);

//#############################################################################

class BCP_cut;
template void std::sort(BCP_cut **, BCP_cut **,
			bool(*)(BCP_cut const *, BCP_cut const *));

#ifdef NEED_IMPLICIT_TEMPLATE_FUNCTIONS
template void
__final_insertion_sort(BCP_cut **, BCP_cut **,
		       bool(*)(BCP_cut const *, BCP_cut const *));
template void
__introsort_loop(BCP_cut **, BCP_cut **, BCP_cut **,
		 BCP_PtrDiff,
		 bool(*)(BCP_cut const *, BCP_cut const *));
#endif

//#############################################################################

class BCP_cut;
class BCP_row;
typedef std::pair<BCP_cut*, BCP_row*> cut_row_pair;

template void
std::sort(cut_row_pair *, cut_row_pair *,
	  bool (*)(cut_row_pair const &, cut_row_pair const &));

#ifdef NEED_IMPLICIT_TEMPLATE_FUNCTIONS
template void
__final_insertion_sort(cut_row_pair *, cut_row_pair *,
		       bool(*)(cut_row_pair const &, cut_row_pair const &));
template void
__introsort_loop(cut_row_pair *, cut_row_pair *, cut_row_pair *,
		 BCP_PtrDiff,
		 bool(*)(cut_row_pair const &, cut_row_pair const &));
#endif

//#############################################################################

#include <vector>
#include <queue>
#include "MC.hpp"
template class
std::priority_queue<MC_path_node*,
                    std::vector<MC_path_node*>,
                    MC_path_node_ptr_compare>;

#ifdef NEED_IMPLICIT_TEMPLATE_CLASSES
template class 
std::vector<MC_path_node*>;
#endif

#ifdef NEED_IMPLICIT_TEMPLATE_FUNCTIONS
template void
__push_heap(MC_path_node **, BCP_PtrDiff, BCP_PtrDiff,
	    MC_path_node *, MC_path_node_ptr_compare);
template void
__adjust_heap(MC_path_node **, BCP_PtrDiff, BCP_PtrDiff,
	      MC_path_node *, MC_path_node_ptr_compare);
#endif

//#############################################################################

#include <map>
typedef std::pair<int,int> intpair;
template class
std::map<intpair, int>;

#ifdef NEED_IMPLICIT_TEMPLATE_CLASSES
template class
std::_Rb_tree<intpair, 
              std::pair<const intpair, int>, 
              std::_Select1st<std::pair<const intpair, int> >, 
              std::less<intpair>, 
              std::allocator<int> >;
#endif


//#############################################################################

#include <set>

template class std::set<int>;

#ifdef NEED_IMPLICIT_TEMPLATE_CLASSES
template class
std::_Rb_tree<int, int,
              std::_Identity<int>, std::less<int>, std::allocator<int> >;
#endif

#endif
