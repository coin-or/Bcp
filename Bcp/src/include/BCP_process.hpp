// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROCESS_H
#define _BCP_PROCESS_H

class BCP_buffer;

class BCP_process {
private:
    const int me;
    const int parent;
public:
    BCP_process(int self, int my_parent) : me(self), parent(my_parent) {}
    // default copy constructor & assignment operator are OK.
    virtual ~BCP_process() {}
    const int get_process_id() const { return me; }
    const int get_parent() const { return parent; }

    virtual BCP_buffer& get_message_buffer() = 0;
    virtual void process_message() = 0;
};

#endif
