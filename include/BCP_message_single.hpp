// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_SINGLE_H
#define _BCP_MESSAGE_SINGLE_H

#include "BCP_message.hpp"
#include "BCP_tm.hpp"
#include "BCP_lp.hpp"
#include "BCP_cg.hpp"
#include "BCP_vg.hpp"
//#include "BCP_cp.hpp"
//#include "BCP_vp.hpp"

//#############################################################################

class BCP_single_id : public BCP_proc_id {
private:
   int _pid;
public:
   BCP_single_id(int id = -1) : _pid(id) {}
   ~BCP_single_id() {}

   bool is_same_process(const BCP_proc_id* other_process) const;
   inline int pid() const { return _pid; }
   BCP_single_id* clone() const { return new BCP_single_id(_pid); }
};

//#############################################################################

class BCP_single_environment : public BCP_message_environment {
private:
  // The argument list of the started process
  int _argnum;
  char ** _arglist;
public:
  void set_arguments(const int argnum, const char* const * args);
private:
   BCP_single_environment(BCP_single_id& my_id) :
     _argnum(0), _arglist(NULL), _my_id(my_id) {}
private:
   BCP_single_id _my_id;
   static BCP_single_id _tm_id;
   static BCP_tm_prob* _tm_prob;
   static BCP_single_id _lp_id;
   static BCP_lp_prob* _lp_prob;
   static BCP_single_id _cg_id;
   static BCP_cg_prob* _cg_prob;
   static BCP_single_id _vg_id;
   static BCP_vg_prob* _vg_prob;
//    static BCP_single_id _cp_id;
//    static BCP_cp_prob* _cp_prob;
//    static BCP_single_id _vp_id;
//    static BCP_vp_prob* _vp_prob;
   static BCP_buffer* _buf;
public:
   BCP_single_environment() : _argnum(0), _arglist(NULL), _my_id() {}
   ~BCP_single_environment();
   
   BCP_proc_id* register_process();
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
		const int timeout);
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

BCP_single_id* BCP_is_single_id(const BCP_proc_id* pid, const char* str);

#endif
