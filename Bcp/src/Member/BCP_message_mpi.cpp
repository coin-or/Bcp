// BCP_message_mpi.cpp is a declaration of BCP_message_mpi
// Sonya Marcarelli & Igor Vasil'ev (vil@icc.ru)
// All Rights Reserved.

#include "BcpConfig.h"
#if defined(COIN_HAS_MPI)

#include <cstdio>
#include <cmath>

#define MPICH_SKIP_MPICXX
#include <mpi.h>

#include "BCP_error.hpp"
#include "BCP_process.hpp"
#include "BCP_buffer.hpp"
#include "BCP_vector.hpp"
#include "BCP_message_mpi.hpp"

bool BCP_mpi_environment::mpi_init_called = false;

//#############################################################################

int BCP_mpi_environment::is_mpi(int argc, char *argv[])
{
    int pid, num_proc;
    MPI_Init(&argc, &argv);
    BCP_mpi_environment::mpi_init_called = true;
    // MPI_Init may or may not have succeeded. In any case check if we can get
    // the number of procs
    if (MPI_Comm_size(MPI_COMM_WORLD, &num_proc) != MPI_SUCCESS) {
	// Now it's certain. Not an MPI environment
	return -1;
    }
    if (num_proc == 1) {
	// Might as well execute everything as a serial environment
	return -1;
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    return pid;
}

//#############################################################################

int BCP_is_mpi_id(const BCP_proc_id* pid, const char* str) {

    const BCP_mpi_id* id = dynamic_cast<const BCP_mpi_id*>(pid);

    if (id == 0){
	printf("Trying to use a non-mpi proc id in %s.\n", str);
	throw BCP_fatal_error(" ERROR in MPI -- exiting.\n");
    }
    return id->pid();
}

//#############################################################################

static int* BCP_process_array_2_int(const BCP_proc_array& parray,
				    const char* str) {
    int *ids = new int[parray.size()];
    BCP_vec<BCP_proc_id*>::const_iterator first = parray.procs().begin();
    BCP_vec<BCP_proc_id*>::const_iterator last = parray.procs().end();
    while (first != last) {
	*ids = BCP_is_mpi_id(*first, str);
	++ids;
	++first;
    }
    ids -= parray.size();
    return ids;
}

static int* BCP_process_vec_2_int(BCP_vec<BCP_proc_id*>::const_iterator beg,
				  BCP_vec<BCP_proc_id*>::const_iterator end,
				  const char* str) {
    const int new_num = end - beg;
    int *ids = new int[new_num];
    while (beg != end) {
	*ids = BCP_is_mpi_id(*beg, str);
	++ids;
	++beg;
    }
    ids -= new_num;
    return ids;
}

//#############################################################################

bool
BCP_mpi_id::is_same_process(const BCP_proc_id* other_process) const {

    return pid() == BCP_is_mpi_id(other_process, "is_same_process()");
}

//#############################################################################


BCP_mpi_environment::BCP_mpi_environment(int argc, char *argv[]) {
    /* Initialize the MPI environment. */
    seqproc = 1;
    int pid, num_proc;
    if (! mpi_init_called) {
	MPI_Init(&argc, &argv);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
}

//-----------------------------------------------------------------------------

BCP_mpi_environment::~BCP_mpi_environment() {
    MPI_Finalize();
}

//-----------------------------------------------------------------------------

void
BCP_mpi_environment::check_error(const int code, const char* str) const {

    if (code != MPI_SUCCESS){
	printf("%s returned error code %i.\n", str, code);
	throw BCP_fatal_error(" ERROR in mpi -- exiting.\n");
    }
}

//-----------------------------------------------------------------------------

BCP_proc_id*
BCP_mpi_environment::register_process(USER_initialize* user_init) {

    int pid, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    BCP_proc_id* parent = parent_process();
    delete parent;
    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
    return new BCP_mpi_id(pid);
}


BCP_proc_id*
BCP_mpi_environment::parent_process() {
    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    if (pid == 0)
	//I am the master and I no parent
	return 0;
    //The master process has got always pid=0
    pid = 0;
    return new BCP_mpi_id(pid);
}

bool
BCP_mpi_environment::alive(const BCP_proc_id* pid)
{
    //In Mpi is not possible check if a process is alive
    /* FIXME */
    return true;
}

BCP_vec<BCP_proc_id*>::iterator
BCP_mpi_environment::alive(const BCP_proc_array& parray)
{
    BCP_vec<BCP_proc_id*>::const_iterator first = parray.procs().begin();
    BCP_vec<BCP_proc_id*>::const_iterator last = parray.procs().end();
    while (first != last) {
	if (! alive(*first))
	    break;
	++first;
    }
    return const_cast<BCP_vec<BCP_proc_id*>::iterator>(first);
}

//-----------------------------------------------------------------------------

void
BCP_mpi_environment::send(const BCP_proc_id* const target,
			  const BCP_message_tag tag) {
    //create an empty buffer and send it with the tag
    int pid = BCP_is_mpi_id(target, "send()");
    void * buf;
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    check_error(MPI_Send(&buf, 0, MPI_CHAR,
			 pid, tag, MPI_COMM_WORLD),"MPI_Send");
}

void
BCP_mpi_environment::send(const BCP_proc_id* const target,
			  const BCP_message_tag tag, const BCP_buffer& buf) {
    int pid = BCP_is_mpi_id(target, "send()");
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    check_error(MPI_Send(const_cast<char*>(buf.data()), buf.size(), MPI_CHAR,
			 pid, tag, MPI_COMM_WORLD),"MPI_Send");
}

//-----------------------------------------------------------------------------

void
BCP_mpi_environment::multicast(const BCP_proc_array* const target,
			       const BCP_message_tag tag) {
    int* pids = BCP_process_array_2_int(*target, "multicast() - parray_2_int");
    void * buf;
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    for (int i=0; i<target->size(); i++) {
	check_error(MPI_Send(&buf, 0, MPI_CHAR,
			     pids[i], tag, MPI_COMM_WORLD),"MPI_Send");
    }
    delete[] pids;
}

void
BCP_mpi_environment::multicast(const BCP_proc_array* const target,
			       const BCP_message_tag tag,
			       const BCP_buffer& buf) {
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    int* pids = BCP_process_array_2_int(*target, "multicast() - parray_2_int");
    for (int i=0; i<target->size(); i++) {
	check_error(MPI_Send(const_cast<char*>(buf.data()), buf.size(), MPI_CHAR,
			     pids[i], tag, MPI_COMM_WORLD),"MPI_Send");
    }
    delete[] pids;
}

void
BCP_mpi_environment::multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
			       BCP_vec<BCP_proc_id*>::const_iterator end,
			       const BCP_message_tag tag) {
    int* pids = BCP_process_vec_2_int(beg, end, "multicast() - parray_2_int");
    void * buf;
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    for (int i=0; i<(end-beg); i++) {
	check_error(MPI_Send(&buf, 0, MPI_CHAR,
			     pids[i], tag, MPI_COMM_WORLD),"MPI_Send");
    }
    delete[] pids;
}

void
BCP_mpi_environment::multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
			       BCP_vec<BCP_proc_id*>::const_iterator end,
			       const BCP_message_tag tag,
			       const BCP_buffer& buf) {
    int* pids = BCP_process_vec_2_int(beg, end, "multicast() - parray_2_int");
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    for (int i=0; i<(end - beg); i++) {
	check_error(MPI_Send(const_cast<char*>(buf.data()),buf.size(), MPI_CHAR,
			     pids[i], tag, MPI_COMM_WORLD),"MPI_Send");
    }
    delete[] pids;
}

//-----------------------------------------------------------------------------

void
BCP_mpi_environment::receive(const BCP_proc_id* const source,
			     const BCP_message_tag tag, BCP_buffer& buf,
			     const double timeout) {
    buf.clear();
    delete buf._sender;   buf._sender = 0;
    int pid = (source == BCP_AnyProcess ?
	       MPI_ANY_SOURCE : BCP_is_mpi_id(source, "receive()"));
    int msgtag = (tag == BCP_Msg_AnyMessage ? MPI_ANY_TAG : tag);
    int flag = 0;
    MPI_Status status;
    int myid;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (timeout < 0) {
	if (source != BCP_AnyProcess) {
	    check_error(MPI_Probe(pid, msgtag, MPI_COMM_WORLD, &status),
			"MPI_Probe");
	    flag = 1;
	} else {
	    // waiting for anyone
	    check_error(MPI_Probe(pid, msgtag, MPI_COMM_WORLD, &status),
			"MPI_Probe");
	    flag = 1;
	}
    } else {
	check_error(MPI_Iprobe(pid, msgtag, MPI_COMM_WORLD, &flag, &status),
		    "MPI_Iprobe");
    }
    //There is no message
    if (!flag) {
	buf._msgtag = BCP_Msg_NoMessage;
	return;
    }

    int bytes;
    //Get the size of message
    check_error(MPI_Get_count( &status, MPI_CHAR, &bytes ),"MPI_Get_Count");
    buf.make_fit(bytes);
    buf._msgtag = static_cast<BCP_message_tag>(status.MPI_TAG);
    delete buf._sender;
    buf._sender = new BCP_mpi_id(status.MPI_SOURCE);
    buf._size = bytes;

    //Receive the content of the message
    check_error(MPI_Recv(buf._data,bytes, MPI_CHAR,
			 pid, msgtag, MPI_COMM_WORLD,&status),"MPI_Recv");
}



//-----------------------------------------------------------------------------

bool
BCP_mpi_environment::probe(const BCP_proc_id* const source,
			   const BCP_message_tag tag) {
    MPI_Status  status;
    // MPI_Request request;
    int  flag=0;
    int pid = (source == BCP_AnyProcess ?
	       MPI_ANY_SOURCE : BCP_is_mpi_id(source, "probe()"));

    // check if we have a matching message
    int msgtag = tag == BCP_Msg_AnyMessage ? -1 : tag;
    check_error(MPI_Iprobe(pid, msgtag, MPI_COMM_WORLD, &flag, &status),
		"MPI_Iprobe");

    return flag > 0;
}

//-----------------------------------------------------------------------------

BCP_proc_id*
BCP_mpi_environment::unpack_proc_id(BCP_buffer& buf) {
    int pid;
    buf.unpack(pid);
    return new BCP_mpi_id(pid);
}

void
BCP_mpi_environment::pack_proc_id(BCP_buffer& buf, const BCP_proc_id* pid) {
    buf.pack(BCP_is_mpi_id(pid, "pack_proc_id()"));
}

//-----------------------------------------------------------------------------

BCP_proc_id*
BCP_mpi_environment::start_process(const BCP_string& exe, const bool debug) {
    //int pid = seqproc;
    seqproc++;
    return new BCP_mpi_id(seqproc);
}

BCP_proc_id*
BCP_mpi_environment::start_process(const BCP_string& exe,
				   const BCP_string& machine,
				   const bool debug) {
    return new BCP_mpi_id(seqproc);
}

BCP_proc_array*
BCP_mpi_environment::start_processes(const BCP_string& exe,
				     const int proc_num,
				     const bool debug) {
    BCP_vec<BCP_proc_id*> procs;
    procs.reserve(proc_num);
    int* pids = new int[proc_num];

    /*There isn't the spawn function in MPI.
      We assign to pid consecutive number*/
    for (int i = 0; i != proc_num; ++i) {
	pids[i] = seqproc;
	seqproc++;
    }

    for (int i = 0; i != proc_num; ++i)
	procs.push_back(new BCP_mpi_id(pids[i]));
    delete[] pids;
    BCP_proc_array* pa = new BCP_proc_array;
    pa->add_procs(procs.begin(), procs.end());
    return pa;
}

BCP_proc_array*
BCP_mpi_environment::start_processes(const BCP_string& exe,
				     const int proc_num,
				     const BCP_vec<BCP_string>& machines,
				     const bool debug) {
    //It's not possible start the process on a specified machine
    return start_processes(exe,proc_num,debug);
}

int BCP_mpi_environment::num_procs() {
    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    return num_procs;
}

#endif /* COIN_HAS_MPI */
