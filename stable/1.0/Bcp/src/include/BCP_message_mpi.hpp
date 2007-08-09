// BCP_message_mpi.hpp is adeclaration of BCP_message_mpi
// Sonya Marcarelli & Igor Vasil'ev (vil@icc.ru)
// All Rights Reserved.

#ifndef _BCP_MESSAGE_MPI_H
#define _BCP_MESSAGE_MPI_H

#include "BcpConfig.h"

#if defined(COIN_HAS_MPI)

#include "BCP_message.hpp"

//#############################################################################

class BCP_mpi_id : public BCP_proc_id {
public:
   int _pid;
public:
   BCP_mpi_id(int id = 0) : _pid(id) {}

   ~BCP_mpi_id() {}

   bool is_same_process(const BCP_proc_id* other_process) const;
   inline int pid() const { return _pid; }
   inline BCP_proc_id* clone() const { return new BCP_mpi_id(_pid); }
};

//#############################################################################

class BCP_mpi_environment : public BCP_message_environment {
private:
    int seqproc;
    static bool mpi_init_called;
   
private:
   void check_error(const int code, const char* str) const;

public:
    /** Function that determines whether we are running in an mpi environment.
	Returns the mpi id of the process if we are *and* there are more than 1
	processes. Otherwise returns -1 */
    static int is_mpi(int argc, char *argv[]);

    /** Constructor will initialize the MPI environment */
   BCP_mpi_environment(int argc,char *argv[]);
   ~BCP_mpi_environment();

   int num_procs();

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
   //int pid(const BCP_proc_id* pid);
//    void stop_process(const BCP_proc_id* process);
//    void stop_processes(const BCP_proc_array* processes);
};

//#############################################################################

int BCP_is_mpi_id(const BCP_proc_id* pid, const char* str);

//#############################################################################

int* BCP_process_array_2_int(const BCP_proc_array* const target,
			     const char* str);

#endif /* COIN_HAS_MPI */

#endif
