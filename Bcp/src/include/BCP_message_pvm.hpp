// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_PVM_H
#define _BCP_MESSAGE_PVM_H

#include "BcpConfig.h"

#if defined(COIN_HAS_PVM)

#include "BCP_message.hpp"

//#############################################################################

class BCP_pvm_id : public BCP_proc_id {
private:
   int _pid;
public:
   BCP_pvm_id(int id = 0) : _pid(id) {}
   ~BCP_pvm_id() {}

   bool is_same_process(const BCP_proc_id* other_process) const;
   inline int pid() const { return _pid; }
   inline BCP_proc_id* clone() const { return new BCP_pvm_id(_pid); }
};

//#############################################################################

class BCP_pvm_environment : public BCP_message_environment {
private:
   void check_error(const int code, const char* str) const;
public:
   BCP_pvm_environment() {}
   ~BCP_pvm_environment();
   
   BCP_proc_id* register_process(USER_initialize* user_init);
   BCP_proc_id* parent_process();

   bool alive(const BCP_proc_id* pid); 
   BCP_vec<BCP_proc_id*>::iterator alive(const BCP_proc_array& parray);

   void send(const BCP_proc_id* const target, const BCP_message_tag tag);
   void send(const BCP_proc_id* const target,
	     const BCP_message_tag tag, const BCP_buffer& buf);

   void multicast(const BCP_proc_array* const target,
		  const BCP_message_tag tag);
   void multicast(const BCP_proc_array* const target,
		  const BCP_message_tag tag, const BCP_buffer& buf);
   void multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
   		  BCP_vec<BCP_proc_id*>::const_iterator end,
   		  const BCP_message_tag tag);
   void multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
		  BCP_vec<BCP_proc_id*>::const_iterator end,
		  const BCP_message_tag tag,
		  const BCP_buffer& buf);

   void receive(const BCP_proc_id* const source,
		const BCP_message_tag tag, BCP_buffer& buf,
		const double timeout);
   bool probe(const BCP_proc_id* const source,
	      const BCP_message_tag tag);

   BCP_proc_id* unpack_proc_id(BCP_buffer& buf);
   void pack_proc_id(BCP_buffer& buf, const BCP_proc_id* pid);

   BCP_proc_id* start_process(const BCP_string& exe,
			      const bool debug);
   BCP_proc_id* start_process(const BCP_string& exe,
			      const BCP_string& machine,
			      const bool debug);
   BCP_proc_array* start_processes(const BCP_string& exe,
				   const int proc_num,
				   const bool debug);
   BCP_proc_array* start_processes(const BCP_string& exe,
				   const int proc_num,
				   const BCP_vec<BCP_string>& machines,
				   const bool debug);

//    void stop_process(const BCP_proc_id* process);
//    void stop_processes(const BCP_proc_array* processes);
};

//#############################################################################

int BCP_is_pvm_id(const BCP_proc_id* pid, const char* str);

//#############################################################################

int* BCP_process_array_2_int(const BCP_proc_array* const target,
			     const char* str);

#endif /* COIN_HAS_PVM */

#endif
