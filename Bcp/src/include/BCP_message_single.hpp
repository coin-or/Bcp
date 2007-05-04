// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_SINGLE_H
#define _BCP_MESSAGE_SINGLE_H

#include <map>

#include "BcpConfig.h"

#include "BCP_message.hpp"

class BCP_process;

//#############################################################################

class BCP_single_environment : public BCP_message_environment {
private:
    BCP_single_environment(const BCP_single_environment&);
    BCP_single_environment& operator=(const BCP_single_environment&);
protected:
    // The argument list of the started process
    int _argnum;
    char ** _arglist;
public:
    void set_arguments(const int argnum, const char* const * args);
protected:
    BCP_single_environment(int my_id) :
	_argnum(0), _arglist(NULL), _my_id(my_id) {}
protected:
    int _my_id;
    static std::map<int, BCP_process*> processes;
public:
    BCP_single_environment() : _argnum(0), _arglist(NULL), _my_id() {}
    virtual ~BCP_single_environment();
   
    int register_process(USER_initialize* user_init);
    int parent_process();

    bool alive(const int pid); 
    BCP_vec<int>::const_iterator alive(const BCP_proc_array& parray);

    void send(const int target, const BCP_message_tag tag);
    void send(const int target,
	      const BCP_message_tag tag, const BCP_buffer& buf);

    void multicast(const BCP_proc_array& target,
    		   const BCP_message_tag tag);
    void multicast(const BCP_proc_array& target,
    		   const BCP_message_tag tag, const BCP_buffer& buf);
    void multicast(BCP_vec<int>::const_iterator beg,
    		   BCP_vec<int>::const_iterator end,
    		   const BCP_message_tag tag);
    void multicast(BCP_vec<int>::const_iterator beg,
    		   BCP_vec<int>::const_iterator end,
    		   const BCP_message_tag tag,
    		   const BCP_buffer& buf);

    void receive(const int source,
    		 const BCP_message_tag tag, BCP_buffer& buf,
    		 const double timeout);
    bool probe(const int source, const BCP_message_tag tag);

    int start_process(const BCP_string& exe,
    		      const bool debug);
    int start_process(const BCP_string& exe,
    		      const BCP_string& machine,
    		      const bool debug);
    BCP_proc_array* start_processes(const BCP_string& exe,
    				    const int proc_num,
    				    const bool debug);
    BCP_proc_array* start_processes(const BCP_string& exe,
					    const int proc_num,
					    const BCP_vec<BCP_string>& machines,
					    const bool debug);

//    void stop_process(const int process);
//    void stop_processes(const BCP_proc_array* processes);
};

#endif
