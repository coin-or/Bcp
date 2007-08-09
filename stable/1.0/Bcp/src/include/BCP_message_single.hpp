// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_SINGLE_H
#define _BCP_MESSAGE_SINGLE_H

#include "BCP_process.hpp"
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

inline bool
operator<(const BCP_single_id& id0, const BCP_single_id& id1)
{
   return id0.pid() < id1.pid();
}

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
   BCP_single_environment(BCP_single_id& my_id) :
     _argnum(0), _arglist(NULL), _my_id(my_id) {}
protected:
   BCP_single_id _my_id;
   static std::map<BCP_single_id, BCP_process*> processes;
public:
   BCP_single_environment() : _argnum(0), _arglist(NULL), _my_id() {}
   virtual ~BCP_single_environment();
   
   virtual BCP_proc_id* register_process(USER_initialize* user_init);
   virtual BCP_proc_id* parent_process();

   virtual bool alive(const BCP_proc_id* pid); 
   virtual BCP_vec<BCP_proc_id*>::iterator alive(const BCP_proc_array& parray);

   virtual void send(const BCP_proc_id* const target,
		     const BCP_message_tag tag);
   virtual void send(const BCP_proc_id* const target,
		     const BCP_message_tag tag, const BCP_buffer& buf);

   virtual void multicast(const BCP_proc_array* const target,
			  const BCP_message_tag tag);
   virtual void multicast(const BCP_proc_array* const target,
			  const BCP_message_tag tag, const BCP_buffer& buf);
   virtual void multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
			  BCP_vec<BCP_proc_id*>::const_iterator end,
			  const BCP_message_tag tag);
   virtual void multicast(BCP_vec<BCP_proc_id*>::const_iterator beg,
			  BCP_vec<BCP_proc_id*>::const_iterator end,
			  const BCP_message_tag tag,
			  const BCP_buffer& buf);

   virtual void receive(const BCP_proc_id* const source,
			const BCP_message_tag tag, BCP_buffer& buf,
			const double timeout);
   virtual bool probe(const BCP_proc_id* const source,
		      const BCP_message_tag tag);

   virtual BCP_proc_id* unpack_proc_id(BCP_buffer& buf);
   virtual void pack_proc_id(BCP_buffer& buf, const BCP_proc_id* pid);

   virtual BCP_proc_id* start_process(const BCP_string& exe,
				      const bool debug);
   virtual BCP_proc_id* start_process(const BCP_string& exe,
				      const BCP_string& machine,
				      const bool debug);
   virtual BCP_proc_array* start_processes(const BCP_string& exe,
					   const int proc_num,
					   const bool debug);
   virtual BCP_proc_array* start_processes(const BCP_string& exe,
					   const int proc_num,
					   const BCP_vec<BCP_string>& machines,
					   const bool debug);

//    void stop_process(const BCP_proc_id* process);
//    void stop_processes(const BCP_proc_array* processes);
};

//#############################################################################

BCP_single_id* BCP_is_single_id(const BCP_proc_id* pid, const char* str);

#endif
