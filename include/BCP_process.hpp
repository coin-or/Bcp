// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROCESS_H
#define _BCP_PROCESS_H

class BCP_buffer;
class BCP_proc_id;

class BCP_process {
public:
   virtual ~BCP_process();
   virtual BCP_buffer& get_message_buffer() = 0;
   virtual BCP_proc_id* get_parent() = 0;
   virtual void process_message() = 0;
};

#endif
