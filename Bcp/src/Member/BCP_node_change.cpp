// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_node_change.hpp"
#include "BCP_warmstart.hpp"
#include "BCP_buffer.hpp"
#include "BCP_USER.hpp"

//#############################################################################

BCP_node_change::BCP_node_change() : 
    core_change(), var_change(), cut_change(), warmstart(0) {}

BCP_node_change::BCP_node_change(BCP_user_pack* packer,
				 const bool def, BCP_buffer& buf) :
    core_change(), var_change(), cut_change(), warmstart(0)
{
    unpack(packer, def, buf);
}

BCP_node_change::~BCP_node_change() {
    delete warmstart;
}

//#############################################################################

void
BCP_node_change::pack(BCP_user_pack* packer, const bool report_default,
		      BCP_buffer& buf) const
{
    core_change.pack(buf);
    var_change.pack(buf);
    cut_change.pack(buf);
    const bool has_ws = warmstart != NULL;
    buf.pack(has_ws);
    if (has_ws) {
	packer->pack_warmstart(warmstart, buf, report_default);
    }
}

//=============================================================================

void
BCP_node_change::unpack(BCP_user_pack* packer, const bool report_default,
			BCP_buffer& buf)
{
    core_change.unpack(buf);
    var_change.unpack(buf);
    cut_change.unpack(buf);
    bool has_ws;
    buf.unpack(has_ws);
    if (has_ws) {
	warmstart = packer->unpack_warmstart(buf, report_default);
    }
}
