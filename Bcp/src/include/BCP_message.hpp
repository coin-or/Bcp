// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_H
#define _BCP_MESSAGE_H

// This file is fully docified.

#include <utility>

#include "BCP_string.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_vector.hpp"

//#############################################################################

class BCP_buffer;
class USER_initialize;

//#############################################################################

const int BCP_AnyProcess = -1;

/** This is an abstract base class that describes the message passing
    environment. The implementation of the message passing protocol
    must implement this class. 

    All methods are pure virtual, enforcing the correct overriding of
    the methods. */

class BCP_message_environment{
public:
    /**@name Destructor */
    /*@{*/
    /** Being virtual, the destructor invokes the destructor for the real 
	type of the object being deleted. */
    virtual ~BCP_message_environment() {}
    /*@}*/
    
    /**@name Registering a process */
    /*@{*/
    /** A process can register (receive its process id) with the message
	passing environment. */
    virtual int register_process(USER_initialize* user_init) = 0;
    /*@}*/
    /**@name Identify parent process */
    /*@{*/
    /** Return the process id of the parent process (the process that
	spawned the currnet process. Returns null if the process does
	not have a parent.  */
    virtual int parent_process() = 0;
    /*@}*/

    /**@name Testing processes */
    /*@{*/
    /** Test if the process given by the argument is alive or
	not. Return true if alive, false otherwise. */
    virtual bool alive(const int pid) = 0;
    /** Test if the processes given by the process array in the argument
	are alive or not. Return a pointer to the first dead process,
	or to the end of the array if there are no dead processes. */
    virtual const int* alive(int num, const int* pids) = 0;
    /*@}*/

    /**@name Send to one process */
    /*@{*/
    /** Send an empty message (message tag only) to the process given
	by the frist argument. */
    virtual void send(const int target, const BCP_message_tag tag) = 0;
    /** Send the message in the buffer with the given message tag to the
	process given by the first argument. */
    virtual void send(const int target, const BCP_message_tag tag,
		      const BCP_buffer& buf) = 0;
    /*@}*/

    /**@name Broadcasting */
    /*@{*/
    /** Send an empty message (message tag only) to all the processes in
	the process array. */
    virtual void multicast(int num, const int* targets,
			   const BCP_message_tag tag) = 0;
    /** Send the message in the buffer with the given message tag to all
	processes in the process array. */
    virtual void multicast(int num, const int* targets,
			   const BCP_message_tag tag,
			   const BCP_buffer& buf) = 0;
    /*@}*/

    // blocking receive w/ timeout (ms??) from given source given msgtag (can
    // give wildcard anymsg, anyproc)

    /**@name Receiving */
    /*@{*/
    /** Blocking receive with timeout. Wait until a message is received from
	the given process with the given message tag within the given
	timelimit. If <code>timeout</code> is positive then the receive routine
	times out after this many seconds. If it is negative then the
	method blocks until a message is received. The received message is
	saved into the buffer. With a 0 pointer as the first argument (or the
	predefined <code>BCP_AnyProcess</code> process id can be used) a
	message from any process will be accepted. Messages with any message
	tag are accepted if the second argument is
	<code>BCP_Msg_AnyMessage</code>. */
    virtual void receive(const int source,
			 const BCP_message_tag tag, BCP_buffer& buf,
			 const double timeout) = 0;
    /** Probe if there are any messages from the given process with the
	given message tag. Return true if such a message is found, false
	otherwise. Note that the message is not "read", only its
	existence is checked. Similarly as above, the wild cards
	<code>BCP_AnyProcess</code> and <code>BCP_Msg_AnyMessage</code>
	can be used. */
    virtual bool probe(const int source, const BCP_message_tag tag) = 0; 
    /*@}*/

    /**@name Starting a process */
    /*@{*/
    /** Spawn a new process. The first argument contains the path to the
	executable to be spawned. If the second argument is set to true, then
	the executable is spawned under a debugger. */
    virtual int start_process(const BCP_string& exe,
			      const bool debug) = 0;
    /** Spawn a new process on the machine specified by the second argument. */
    virtual int start_process(const BCP_string& exe,
			      const BCP_string& machine,
			      const bool debug) = 0;
    /** Spawn <code>proc_num</code> processes, all with the same executable.
	NOTE: ids must be allocated already and have enough space for \c
	proc_num entries.
	\return true/false depending on success.
    */
    virtual bool start_processes(const BCP_string& exe,
				 const int proc_num,
				 const bool debug,
				 int* ids) = 0;
    /** Spawn <code>proc_num</code> processes on the machines given by
	the third argument, all with the same executable.
	NOTE: ids must be allocated already and have enough space for \c
	proc_num entries.
	\return true/false depending on success.
    */
    virtual bool start_processes(const BCP_string& exe,
				 const int proc_num,
				 const BCP_vec<BCP_string>& machines,
				 const bool debug,
				 int* ids) = 0;
    /*@}*/
  
    /**@name Additional function for MPI interface */
    /*@{*/
    /**Return the number of processes. For non-MPI this just returns 0.*/
    virtual int num_procs() { return 0; }
    /*@}*/

    //    virtual void stop_process(int process) = 0;
    //    virtual void stop_processes(BCP_proc_array& processes) = 0;
};

#endif
