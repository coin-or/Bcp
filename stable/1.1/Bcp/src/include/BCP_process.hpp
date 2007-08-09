// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROCESS_H
#define _BCP_PROCESS_H

class BCP_buffer;
class BCP_proc_id;

class BCP_process {
private:
    BCP_process(const BCP_process&);
    BCP_process& operator=(const BCP_process&);
private:
    const BCP_proc_id* my_id;
    const BCP_proc_id* parent_id;
public:
    BCP_process(const BCP_proc_id* self, const BCP_proc_id* parent) :
	my_id(self), parent_id(parent) {}
    virtual ~BCP_process();
    virtual BCP_buffer& get_message_buffer() = 0;
    virtual void process_message() = 0;
    const BCP_proc_id* get_process_id() const { return my_id; }
    const BCP_proc_id* get_parent() const { return parent_id; }
};

#endif
