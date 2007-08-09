// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MAIN_FUN_H
#define _BCP_MAIN_FUN_H

#include "BCP_enum_process_t.hpp"

class BCP_message_environment;
class USER_initialize;

void BCP_tm_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 const int argnum, const char* const * arglist);

BCP_process_t BCP_tmstorage_main(BCP_message_environment* msg_env,
				 USER_initialize* user_init,
				 int my_id, int parent, double ub);

BCP_process_t BCP_lp_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub);

#if 0
BCP_process_t BCP_cp_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub);

BCP_process_t BCP_vp_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub);
#endif

BCP_process_t BCP_cg_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub);

BCP_process_t BCP_vg_main(BCP_message_environment* msg_env,
			  USER_initialize* user_init,
			  int my_id, int parent, double ub);

#endif
