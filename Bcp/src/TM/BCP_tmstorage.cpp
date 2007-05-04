// Copyright (C) 2007, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <cerrno>

#include "BcpConfig.h"
#include "BCP_os.hpp"
#include "BCP_string.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_main_fun.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_tm_node.hpp"
#include "BCP_tmstorage.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_node_change.hpp"

using std::make_pair;

//#############################################################################

template <>
void BCP_parameter_set<BCP_ts_par>::create_keyword_list()
{
    keys.push_back(make_pair(BCP_string("BCP_NiceLevel"),
			     BCP_parameter(BCP_IntPar, 
					   NiceLevel)));
    keys.push_back(make_pair(BCP_string("BCP_LogFileName"),
			     BCP_parameter(BCP_StringPar, 
					   LogFileName)));
}

//#############################################################################

template <>
void BCP_parameter_set<BCP_ts_par>::set_default_entries()
{
    //-------------------------------------------------------------------------
    set_entry(MessagePassingIsSerial, false);
    set_entry(NiceLevel, 0);
    set_entry(LogFileName,"");
}

//#############################################################################

/** This method returns a nonnegative number that indicates how "willing is
    this process to accept more data to be stored. As a simple implementation
    it's the number of free megabytes. If it's les than 5, no new data is
    accepted. */

static inline int freeMemory()
{
    return 0;
}

//#############################################################################

BCP_process_t BCP_tmstorage_main(BCP_message_environment* msg_env,
				 USER_initialize* user_init,
				 int my_id, int parent)
{
    // If we ever get here then the environment is parallel

    BCP_ts_prob p(my_id, parent);
    p.msg_env = msg_env;

    p.par.set_entry(BCP_ts_par::MessagePassingIsSerial, false);

    // wait for the message with the parameters and unpack it
    p.msg_buf.clear();
    msg_env->receive(parent /*tree_manager*/,
		     BCP_Msg_ProcessParameters, p.msg_buf, -1);
    p.par.unpack(p.msg_buf);

    // Let us be nice
    setpriority(PRIO_PROCESS, 0, p.par.entry(BCP_ts_par::NiceLevel));

    FILE* logfile = 0;

    const BCP_string& log = p.par.entry(BCP_ts_par::LogFileName);
    if (! (p.par.entry(BCP_ts_par::LogFileName) == "")) {
	int len = log.length();
	char *logname = new char[len + 300];
	memcpy(logname, log.c_str(), len);
	memcpy(logname + len, "-ts-", 4);
	len += 4;
	gethostname(logname + len, 255);
	len = strlen(logname);
	logname[len++] = '-';
	sprintf(logname + len, "%i", static_cast<int>(GETPID));
	logfile = freopen(logname, "a", stdout);
	if (logfile == 0) {
	    fprintf(stderr, "Error while redirecting stdout: %i\n", errno);
	    abort();
	}
	setvbuf(logfile, NULL, _IOLBF, 0); // make it line buffered
	delete[] logname;
    } else {
	setvbuf(stdout, NULL, _IOLBF, 0); // make it line buffered
    }

    // now create the user universe
    p.user = user_init->ts_init(p);
    p.user->setTsProblemPointer(&p);
    p.packer = user_init->packer_init(p.user);
    p.packer->user_class = p.user;

    // wait for the core description and process it
    p.msg_buf.clear();
    p.msg_env->receive(parent /*tree_manager*/,
		       BCP_Msg_CoreDescription, p.msg_buf, -1);
    p.core->unpack(p.msg_buf);

    // wait for the user info
    p.msg_buf.clear();
    msg_env->receive(parent /*tree_manager*/,
		     BCP_Msg_InitialUserInfo, p.msg_buf, -1);
    p.user->unpack_module_data(p.msg_buf);

    // ok, we're all geared up to generate vars
    // wait for messages and process them...
    BCP_message_tag msgtag;
    BCP_process_t ptype = BCP_ProcessType_EndProcess;
    while (true) {
	p.msg_buf.clear();
	msg_env->receive(BCP_AnyProcess, BCP_Msg_AnyMessage, p.msg_buf, 15);
	msgtag = p.msg_buf.msgtag();
	if (msgtag == BCP_Msg_NoMessage) {
	    // test if the TM is still alive
	    if (! p.msg_env->alive(parent /*tree_manager*/))
		throw BCP_fatal_error("VG:   The TM has died -- VG exiting\n");
	}
	if (msgtag == BCP_Msg_ProcessType) {
	    p.msg_buf.unpack(ptype);
	    break;
	}
	p.process_message();
	if (msgtag == BCP_Msg_FinishedBCP)
	    break;
    }
    if (logfile)
	fclose(logfile);

    return ptype;
}

//#############################################################################

BCP_ts_prob::~BCP_ts_prob()
{
    std::map<int, BCP_tm_node_data*>::iterator n;
    for (n = nodes.begin(); n != nodes.end(); ++n) {
	delete n->second;
    }
    std::map<int, BCP_cut_algo*>::iterator c;
    for (c = cuts.begin(); c != cuts.end(); ++c) {
	delete c->second;
    }
    std::map<int, BCP_var_algo*>::iterator v;
    for (v = vars.begin(); v != vars.end(); ++v) {
	delete v->second;
    }
}
    
//#############################################################################

static void process_Msg_NodeList(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME: This routine depends on writing the corresponding routine in TM
    int index;
    int num;
    buf.unpack(num);
    int i;
    for (i = 1; i <= num; ++i) {
	if ((i % 100 == 0) && freeMemory() < 2) {
	    --i;
	    break;
	}
	// OK, receive a description
	BCP_tm_node_data* data = new BCP_tm_node_data;
	buf.unpack(index);
	data->_desc = new BCP_node_change;

	data->_desc->unpack(p.packer, false, buf);
	data->_user = p.packer->unpack_user_data(buf);

	p.nodes[index] = data;
    }
    // Now 'i' contains how many node descriptions were successfully received
    buf.clear();
    buf.pack(i);
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_NodeListReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_NodeListRequest(BCP_ts_prob& p, BCP_buffer& buf)
{
    int id;
    buf.unpack(id);
    BCP_vec<int>& nodelevels = p.positions;
    buf.unpack(nodelevels);
    const int num = nodelevels.size();
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    buf.clear();
    buf.pack(id);
    buf.pack(num);
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_tm_node_data*>::iterator n = p.nodes.find(inds[i]);
	if (n == p.nodes.end()) {
	    throw BCP_fatal_error("TS: Requested node (%i) is not here\n",
				  inds[i]);
	}
	BCP_tm_node_data* data = n->second;
	buf.pack(nodelevels[i]);
	buf.pack(inds[i]);

	data->_desc->pack(p.packer, false, buf);
	p.packer->pack_user_data(data->_user, buf);
    }
    p.msg_env->send(p.get_parent(), BCP_Msg_NodeListRequestReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_NodeListDelete(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME This routine depends on writing the corresponding routine in TM
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    const int num = inds.size();
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_tm_node_data*>::iterator n = p.nodes.find(inds[i]);
	if (n == p.nodes.end()) {
	    throw BCP_fatal_error("TS: Node to be deleted (%i) is not here\n",
				  inds[i]);
	}
	delete n->second;
	p.nodes.erase(n);
    }
    buf.clear();
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_NodeListDeleteReply, buf);
}

//=============================================================================

static void process_Msg_CutList(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME This routine depends on writing the corresponding routine in TM
    int num;
    buf.unpack(num);
    int i = 0; 
    for (i = 1; i <= num; ++i) {
	if ((i % 1000 == 0) && freeMemory() < 2) {
	    --i;
	    break;
	}
	// OK, receive a description
	int index;
	buf.unpack(index);
	p.cuts[index] = p.packer->unpack_cut_algo(buf);
    }
    // Now 'i' contains how many node descriptions were successfully received
    buf.clear();
    buf.pack(i);
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_CutListReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_CutListRequest(BCP_ts_prob& p, BCP_buffer& buf)
{
    int id;
    buf.unpack(id);
    BCP_vec<int>& pos = p.positions;
    buf.unpack(pos);
    const int num = pos.size();
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    buf.clear();
    buf.pack(id);
    buf.pack(num);
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_cut_algo*>::iterator c = p.cuts.find(inds[i]);
	if (c == p.cuts.end()) {
	    throw BCP_fatal_error("TS: Requested cut (%i) is not here\n",
				  inds[i]);
	}
	buf.pack(pos[i]);
	buf.pack(inds[i]);
	p.packer->pack_cut_algo(c->second, buf);
    }
    p.msg_env->send(p.get_parent(), BCP_Msg_CutListRequestReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_CutListDelete(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME This routine depends on writing the corresponding routine in TM
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    const int num = inds.size();
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_cut_algo*>::iterator c = p.cuts.find(inds[i]);
	if (c == p.cuts.end()) {
	    throw BCP_fatal_error("TS: cut to be deleted (%i) is not here\n",
				  inds[i]);
	}
	delete c->second;
	p.cuts.erase(c);
    }
    buf.clear();
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_CutListDeleteReply, buf);
}

//=============================================================================

static void process_Msg_VarList(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME This routine depends on writing the corresponding routine in TM
    int num;
    buf.unpack(num);
    int i = 0; 
    for (i = 1; i <= num; ++i) {
	if ((i % 1000 == 0) && freeMemory() < 2) {
	    --i;
	    break;
	}
	// OK, receive a description
	int index;
	buf.unpack(index);
	p.vars[index] = p.packer->unpack_var_algo(buf);
    }
    // Now 'i' contains how many node descriptions were successfully received
    buf.clear();
    buf.pack(i);
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_VarListReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_VarListRequest(BCP_ts_prob& p, BCP_buffer& buf)
{
    int id;
    buf.unpack(id);
    BCP_vec<int>& pos = p.positions;
    buf.unpack(pos);
    const int num = pos.size();
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    buf.clear();
    buf.pack(id);
    buf.pack(num);
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_var_algo*>::iterator c = p.vars.find(inds[i]);
	if (c == p.vars.end()) {
	    throw BCP_fatal_error("TS: Requested var (%i) is not here\n",
				  inds[i]);
	}
	buf.pack(pos[i]);
	buf.pack(inds[i]);
	p.packer->pack_var_algo(c->second, buf);
    }
    p.msg_env->send(p.get_parent(), BCP_Msg_VarListRequestReply, buf);
}

//-----------------------------------------------------------------------------

static void process_Msg_VarListDelete(BCP_ts_prob& p, BCP_buffer& buf)
{
    // FIXME This routine depends on writing the corresponding routine in TM
    BCP_vec<int>& inds = p.indices;
    buf.unpack(inds);
    const int num = inds.size();
    for (int i = 0; i < num; ++i) {
	std::map<int, BCP_var_algo*>::iterator v = p.vars.find(inds[i]);
	if (v == p.vars.end()) {
	    throw BCP_fatal_error("TS: var to be deleted (%i) is not here\n",
				  inds[i]);
	}
	delete v->second;
	p.vars.erase(v);
    }
    buf.clear();
    buf.pack(freeMemory());
    p.msg_env->send(p.get_parent(), BCP_Msg_VarListDeleteReply, buf);
}

//#############################################################################

void
BCP_ts_prob::process_message()
{
    switch (msg_buf.msgtag()){
    case BCP_Msg_InitialUserInfo:
	throw BCP_fatal_error("\
TS: BCP_ts_prob::process_message(): BCP_Msg_InitialUserInfo arrived\n");

    case BCP_Msg_NodeList:
	process_Msg_NodeList(*this, msg_buf);
	break;

    case BCP_Msg_NodeListRequest:
	process_Msg_NodeListRequest(*this, msg_buf);
	break;

    case BCP_Msg_NodeListDelete:
	process_Msg_NodeListDelete(*this, msg_buf);
	break;

    case BCP_Msg_CutList:
	process_Msg_CutList(*this, msg_buf);
	break;

    case BCP_Msg_CutListRequest:
	process_Msg_CutListRequest(*this, msg_buf);
	break;

    case BCP_Msg_CutListDelete:
	process_Msg_CutListDelete(*this, msg_buf);
	break;

    case BCP_Msg_VarList:
	process_Msg_VarList(*this, msg_buf);
	break;

    case BCP_Msg_VarListRequest:
	process_Msg_VarListRequest(*this, msg_buf);
	break;
	
    case BCP_Msg_VarListDelete:
	process_Msg_VarListDelete(*this, msg_buf);
	break;

    case BCP_Msg_FinishedBCP:
	break;

    default:
	throw BCP_fatal_error("\
TS: BCP_ts_prob::process_message(): Unexpected message tag (%i) arrived\n",
			      msg_buf.msgtag());
    }
    msg_buf.clear();

}

//=============================================================================

