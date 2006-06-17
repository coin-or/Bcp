// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_NODE_H
#define _BCP_NODE_H

#include "BCP_problem_core.hpp"
#include "BCP_obj_change.hpp"
#include "BCP_indexed_pricing.hpp"

//#############################################################################

class BCP_warmstart;

//#############################################################################

class BCP_node_change{
private:
   BCP_node_change(const BCP_node_change&);
   BCP_node_change& operator=(const BCP_node_change&);
public:
   BCP_problem_core_change core_change;
   BCP_var_set_change var_change;
   BCP_cut_set_change cut_change;
   BCP_indexed_pricing_list indexed_pricing;
   BCP_warmstart* warmstart;
public:
   BCP_node_change();
   ~BCP_node_change();
};

#endif
