// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MAIN_FUN_H
#define _BCP_MAIN_FUN_H

class BCP_message_environment;
class USER_initialize;
class BCP_proc_id;

void
BCP_tm_main(BCP_message_environment* msg_env,
	    USER_initialize* user_init,
	    BCP_proc_id* my_id,
	    const int argnum, const char* const * arglist);

void BCP_lp_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 BCP_proc_id* my_id,
		 BCP_proc_id* parent);

#if 0
void BCP_cp_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 BCP_proc_id* my_id,
		 BCP_proc_id* parent);

void BCP_vp_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 BCP_proc_id* my_id,
		 BCP_proc_id* parent);
#endif

void BCP_cg_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 BCP_proc_id* my_id,
		 BCP_proc_id* parent);

void BCP_vg_main(BCP_message_environment* msg_env,
		 USER_initialize* user_init,
		 BCP_proc_id* my_id,
		 BCP_proc_id* parent);

#endif
