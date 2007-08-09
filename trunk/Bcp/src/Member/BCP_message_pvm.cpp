// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BcpConfig.h"
#if defined(COIN_HAS_PVM)

#include <cstdio>
#include <cmath>

#include <pvm3.h>

#include "BCP_math.hpp"
#include "BCP_error.hpp"
#include "BCP_buffer.hpp"
#include "BCP_vector.hpp"
#include "BCP_message_pvm.hpp"

//#############################################################################

BCP_pvm_environment::~BCP_pvm_environment()
{
    check_error( pvm_exit(), "~BCP_pvm_environment()");
}

//-----------------------------------------------------------------------------

void
BCP_pvm_environment::check_error(const int code, const char* str) const
{
    if (code < 0){
	printf("%s returned error code %i.\n", str, code);
	throw BCP_fatal_error(" ERROR in PVM -- exiting.\n");
    }
}

//-----------------------------------------------------------------------------

int
BCP_pvm_environment::register_process(USER_initialize* user_init)
{
    int pid = pvm_mytid();
    check_error(pid, "pvm_mytid()");
    int parent = parent_process();
    if (parent < 0)
	check_error(pvm_catchout(stdout),"register_process -- pvm_catchout\n");
    /* set stdout to be line buffered so that pvm_catchout will work faster */
    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
    return pid;
}

int
BCP_pvm_environment::parent_process()
{
    int pid = pvm_parent();
    if (pid == PvmNoParent)
	return -1;
    check_error(pid, "pvm_parent()");
    return pid;
}

bool
BCP_pvm_environment::alive(const int pid)
{
    return pvm_pstat(pid) == PvmOk;
}

BCP_vec<int>::const_iterator
BCP_pvm_environment::alive(const BCP_proc_array& parray)
{
    BCP_vec<int>::const_iterator first = parray.procs().begin();
    BCP_vec<int>::const_iterator last = parray.procs().end();
    while (first != last) {
	if (! alive(*first))
	    break;
	++first;
    }
    return BCP_vec<int>::const_iterator>(first);
}

//-----------------------------------------------------------------------------

void
BCP_pvm_environment::send(const int target,
			  const BCP_message_tag tag)
{
    // create an empty buffer and send it with the tag
    check_error( pvm_initsend(PvmDataInPlace), "send() - initsend");
    check_error( pvm_send(target, tag), "send() - send");
}

void
BCP_pvm_environment::send(const int target,
			  const BCP_message_tag tag, const BCP_buffer& buf)
{
    check_error( pvm_initsend(PvmDataInPlace), "send() - initsend");
    check_error( pvm_pkbyte(const_cast<char*>(buf.data()), buf.size(), 1),
		 "send() - pkbyte");
    check_error( pvm_send(target, tag), "send() - send");
}
   
//-----------------------------------------------------------------------------

void
BCP_pvm_environment::multicast(const BCP_proc_array* const target,
			       const BCP_message_tag tag)
{
    check_error( pvm_initsend(PvmDataInPlace), "multicast() - initsend");
    check_error( pvm_mcast(&target->procs()[0], target->size(), tag),
		 "multicast() - send");
}

void
BCP_pvm_environment::multicast(const BCP_proc_array* const target,
			       const BCP_message_tag tag,
			       const BCP_buffer& buf)
{
    check_error( pvm_initsend(PvmDataInPlace), "multicast() - initsend");
    check_error( pvm_pkbyte(const_cast<char*>(buf.data()), buf.size(), 1),
		 "multicast() - pkbyte");
    check_error( pvm_mcast(&target->procs()[0], target->size(), tag),
		 "multicast() - send");
}

void
BCP_pvm_environment::multicast(BCP_vec<int>::const_iterator beg,
			       BCP_vec<int>::const_iterator end,
			       const BCP_message_tag tag) {
    int* pids = BCP_process_vec_2_int(beg, end, "multicast() - parray_2_int");
    check_error( pvm_initsend(PvmDataInPlace), "multicast() - initsend");
    check_error( pvm_mcast(pids, end - beg, tag), "multicast() - send");
    delete[] pids;
}

void
BCP_pvm_environment::multicast(BCP_vec<int>::const_iterator beg,
			       BCP_vec<int>::const_iterator end,
			       const BCP_message_tag tag,
			       const BCP_buffer& buf) {
    int* pids = BCP_process_vec_2_int(beg, end, "multicast() - parray_2_int");
    check_error( pvm_initsend(PvmDataInPlace), "multicast() - initsend");
    check_error( pvm_pkbyte(const_cast<char*>(buf.data()), buf.size(), 1),
		 "multicast() - pkbyte");
    check_error( pvm_mcast(pids, end - beg, tag), "multicast() - send");
    delete[] pids;
}

//-----------------------------------------------------------------------------

void
BCP_pvm_environment::receive(const int const source,
			     const BCP_message_tag tag, BCP_buffer& buf,
			     const double timeout) {
    buf.clear();
    delete buf._sender;   buf._sender = 0;

    int pid = source == BCP_AnyProcess? -1 : BCP_is_pvm_id(source, "receive()");
    struct timeval tout;
    int bufid = 0;
    int msgtag = tag == BCP_Msg_AnyMessage ? -1 : tag;
    if (timeout < 0) {
	if (source != BCP_AnyProcess) {
	    // waiting for a particular process. check from time to time that
	    // it's still alive
	    tout.tv_sec = 10; tout.tv_usec = 0;
	    do {
		check_error( bufid = pvm_trecv(pid, msgtag, &tout),
			     "receive() - trecv");
		if (pvm_pstat(pid) != PvmOk)
		    throw BCP_fatal_error("receive() - source died.\n");
	    } while (! bufid);
	}else{
	    // waiting for anyone
	    check_error( bufid = pvm_recv(pid, msgtag), "receive() - recv");
	}
    } else {
	tout.tv_sec = static_cast<int>(floor(timeout));
	tout.tv_usec = static_cast<int>(floor((timeout - tout.tv_sec)*1e6));
	check_error( bufid = pvm_trecv(pid, msgtag, &tout), "receive() - trecv");
    }
    if (! bufid) {
	buf._msgtag = BCP_Msg_NoMessage;
	return;
    }

    int bytes;
    check_error( pvm_bufinfo(bufid, &bytes, &msgtag, &pid),
		 "receive() - bufinfo");
    buf.make_fit(bytes);
    buf._msgtag = static_cast<BCP_message_tag>(msgtag);
    delete buf._sender;
    buf._sender = new BCP_pvm_id(pid);
    buf._size = bytes;
    check_error( pvm_upkbyte(buf._data, bytes, 1), "receive() - upkbyte");
}

//-----------------------------------------------------------------------------

bool
BCP_pvm_environment::probe(const int const source,
			   const BCP_message_tag tag) {
    int pid = source == BCP_AnyProcess? -1 : BCP_is_pvm_id(source, "probe()");
    if (source != BCP_AnyProcess) {
	// probing for message from a particular process. check that it's still
	// alive
	if (pvm_pstat(pid) != PvmOk)
	    throw BCP_fatal_error("probe() - source died.\n");
    }
    // check if we have a matching message
    int msgtag = tag == BCP_Msg_AnyMessage ? -1 : tag;
    int probed = pvm_probe(pid, msgtag);
    if (probed < 0)
	throw BCP_fatal_error("probe() - pvm error :-(\n");
    return probed > 0;
}

//-----------------------------------------------------------------------------

int
BCP_pvm_environment::unpack_proc_id(BCP_buffer& buf) {
    int pid;
    buf.unpack(pid);
    return new BCP_pvm_id(pid);
}

void
BCP_pvm_environment::pack_proc_id(BCP_buffer& buf, const int pid) {
    buf.pack(BCP_is_pvm_id(pid, "pack_proc_id()"));
}
	    
//-----------------------------------------------------------------------------

static inline char*
BCP_get_next_word(const char*& ctmp, const char* last)
{
    for ( ; ctmp != last && !isgraph(*ctmp); ++ctmp);
    const char* word = ctmp;
    for ( ; ctmp != last && !isspace(*ctmp); ++ctmp);
    if (word == ctmp)
	return 0;
    const int len = ctmp - word;
    char* new_word = new char[len + 1];
    memcpy(new_word, word, len);
    new_word[len] = 0;
    return new_word;
}

static void
BCP_pvm_split_exe(const BCP_string& exe, char*& exe_name, char**& exe_args)
{
    const char* ctmp = exe.c_str();
    const char* last = ctmp + exe.length();
    std::vector<char*> arglist;
    exe_name = BCP_get_next_word(ctmp, last);
    while (ctmp != last) {
	char* word = BCP_get_next_word(ctmp, last);
	if (word)
	    arglist.push_back(word);
    }
    if (arglist.size() == 0) {
	exe_args = 0;
    } else {
	exe_args = new char*[arglist.size() + 1];
	std::copy(arglist.begin(), arglist.end(), exe_args);
	exe_args[arglist.size()] = 0;
    }
}

int
BCP_pvm_environment::start_process(const BCP_string& exe, const bool debug) {
    int flag = debug ? PvmTaskDebug : 0;
    int pid;
    char* exe_name;
    char** exe_args;
    BCP_pvm_split_exe(exe, exe_name, exe_args);
    pvm_spawn(exe_name, exe_args, flag, 0, 1, &pid);
    delete[] exe_name;
    if (exe_args != 0) {
	while (*exe_args != 0) {
	    delete[] *exe_args;
	    ++exe_args;
	}
	delete[] exe_args;
    }
    check_error(pid, "start_process() - spawn");
    return new BCP_pvm_id(pid);
}

int
BCP_pvm_environment::start_process(const BCP_string& exe,
				   const BCP_string& machine,
				   const bool debug) {
    int flag = PvmTaskHost | (debug ? PvmTaskDebug : 0);
    int pid;
    char* exe_name;
    char** exe_args;
    BCP_pvm_split_exe(exe, exe_name, exe_args);
    pvm_spawn(exe_name, exe_args, flag,
	      const_cast<char*>(machine.c_str()), 1, &pid);
    delete[] exe_name;
    if (exe_args != 0) {
	while (*exe_args != 0) {
	    delete[] *exe_args;
	    ++exe_args;
	}
	delete[] exe_args;
    }
    check_error(pid, "start_process() - spawn");
    return new BCP_pvm_id(pid);
}

BCP_proc_array*
BCP_pvm_environment::start_processes(const BCP_string& exe,
				     const int proc_num,
				     const bool debug) {
    BCP_vec<int> procs;
    procs.reserve(proc_num);

    int flag = debug ? PvmTaskDebug : 0;
    int* pids = new int[proc_num];
    char* exe_name;
    char** exe_args;
    BCP_pvm_split_exe(exe, exe_name, exe_args);
    pvm_spawn(exe_name, exe_args, flag, 0, proc_num, pids);
    delete[] exe_name;
    if (exe_args != 0) {
	while (*exe_args != 0) {
	    delete[] *exe_args;
	    ++exe_args;
	}
	delete[] exe_args;
    }
    for (int i = 0; i != proc_num; ++i)
	check_error(pids[i], "start_processes() - spawn");
    for (int i = 0; i != proc_num; ++i)
	procs.push_back(new BCP_pvm_id(pids[i]));
    delete[] pids;
    BCP_proc_array* pa = new BCP_proc_array;
    pa->add_procs(procs.begin(), procs.end());
    return pa;
}

BCP_proc_array*
BCP_pvm_environment::start_processes(const BCP_string& exe,
				     const int proc_num,
				     const BCP_vec<BCP_string>& machines,
				     const bool debug){
    BCP_vec<int> procs;
    procs.reserve(proc_num);
    // spawn the jobs one-by-one on the specified machines
    for (int i = 0; i != proc_num; ++i)
	procs.push_back(start_process(exe, machines[i%machines.size()], debug));
    BCP_proc_array* pa = new BCP_proc_array;
    pa->add_procs(procs.begin(), procs.end());
    return pa;
}

//-----------------------------------------------------------------------------

// void BCP_pvm_environment::stop_process(const int process) {
//    check_error( pvm_kill(process, "stop_process()"), "stop_process()");
// }

// void BCP_pvm_environment::stop_processes(const BCP_proc_array* processes){
//    BCP_vec<int>::const_iterator first = processes.procs.begin();
//    BCP_vec<int>::const_iterator last = pprocesses.procs.end();
//    while (first != last) {
//       check_error( pvm_kill(*first, "stop_processes()"),"stop_processes()");
//       ++first;
//    }
// }

#endif /* COIN_HAS_PVM */
