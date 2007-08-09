// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_USER.hpp"

BCP_node_change::BCP_node_change() : 
   core_change(), var_change(), cut_change(), indexed_pricing(),
   warmstart(0) {}

BCP_node_change::~BCP_node_change() {
   delete warmstart;
}
