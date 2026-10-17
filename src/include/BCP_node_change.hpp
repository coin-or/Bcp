// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_NODE_H
#define _BCP_NODE_H

#include "CoinSmartPtr.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_obj_change.hpp"
#include "BCP_indexed_pricing.hpp"

//#############################################################################

class BCP_warmstart;
class BCP_buffer;
class BCP_user_pack;

//#############################################################################

class BCP_node_change : public Coin::ReferencedObject {
private:
    BCP_node_change(const BCP_node_change&);
    BCP_node_change& operator=(const BCP_node_change&);
public:
    BCP_problem_core_change core_change;
    BCP_obj_set_change var_change;
    BCP_obj_set_change cut_change;
    BCP_warmstart* warmstart;
public:
    BCP_node_change();
    BCP_node_change(BCP_user_pack* packer, const bool def, BCP_buffer& buf);
    ~BCP_node_change();

    void pack(BCP_user_pack* packer, const bool def, BCP_buffer& buf) const;
    void unpack(BCP_user_pack* packer, const bool def, BCP_buffer& buf);
};

#endif
