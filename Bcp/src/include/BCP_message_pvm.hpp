// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_PVM_H
#define _BCP_MESSAGE_PVM_H

#include "BcpConfig.h"

#if defined(COIN_HAS_PVM)

#include "BCP_message.hpp"

//#############################################################################

class BCP_pvm_environment : public BCP_message_environment {
private:
    void check_error(const int code, const char* str) const;
public:
    BCP_pvm_environment() {}
    ~BCP_pvm_environment();
    
    int register_process(USER_initialize* user_init);
    int parent_process();

    bool alive(const int pid); 
    const int* alive(int num, const int* pids);

    void send(const int target, const BCP_message_tag tag);
    void send(const int target,
	      const BCP_message_tag tag, const BCP_buffer& buf);

    void multicast(int num, const int* targets,
    		   const BCP_message_tag tag);
    void multicast(int num, const int* targets,
    		   const BCP_message_tag tag, const BCP_buffer& buf);

    void receive(const int source,
    		 const BCP_message_tag tag, BCP_buffer& buf,
    		 const double timeout);
    bool probe(const int source, const BCP_message_tag tag);

    int start_process(const BCP_string& exe,
    		      const bool debug);
    int start_process(const BCP_string& exe,
    		      const BCP_string& machine,
    		      const bool debug);
    bool start_processes(const BCP_string& exe,
			 const int proc_num,
			 const bool debug,
			 int* ids);
    bool start_processes(const BCP_string& exe,
			 const int proc_num,
			 const BCP_vec<BCP_string>& machines,
			 const bool debug,
			 int* ids);

//    void stop_process(const int process);
//    void stop_processes(const BCP_proc_array* processes);
};

#endif /* COIN_HAS_PVM */

#endif
