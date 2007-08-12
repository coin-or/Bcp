// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROCESS_H
#define _BCP_PROCESS_H

class BCP_buffer;
#include "BCP_vector.hpp"

class BCP_process {
private:
    const int me;
    const int parent;
public:
    BCP_process(int self, int my_parent) : me(self), parent(my_parent) {}
    // default copy constructor & assignment operator are OK.
    virtual ~BCP_process() {}
    int get_process_id() const { return me; }
    int get_parent() const { return parent; }

    virtual BCP_buffer& get_message_buffer() = 0;
    virtual void process_message() = 0;
};

//#############################################################################

/** This class holds an array of processes.  */

class BCP_proc_array {
private:
    /**@name Disabled methods */
    /*@{*/
    /** The default constructor is declared but not defined to disable it. */
    BCP_proc_array(const BCP_proc_array&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_proc_array& operator=(const BCP_proc_array&);
    /*@}*/
private:
    /**@name Data members */
    /*@{*/
    /** Vector of all processes. */
    BCP_vec<int> _procs;
    /** Vector of free processes (subset of all processes). */
    BCP_vec<int> _free_procs;
    /*@}*/
public:
    /**@name Constructor and destructor */
    /*@{*/
    /** The default constructor creates an empty process array */
    BCP_proc_array() : _procs(), _free_procs() {}
    /** The destructor needs to do nothing */
    ~BCP_proc_array() {}
    /*@}*/

    /**@name Query methods */
    /*@{*/
    /** Return the vector of all processes (const version). */
    inline const BCP_vec<int>& procs() const { return _procs; }
    /** Return the number of free processes. */
    inline int free_num() const { return _free_procs.size(); }
    /** Return the number of busy (not free) processes. */
    inline int busy_num() const { return _procs.size() - _free_procs.size(); }
    /** Get the process id of a free process. Return 0 if there are none. */
    int get_free_proc() {
       if (_free_procs.size() > 0) {
           int proc = _free_procs.back();
           _free_procs.pop_back();
           return proc;
       }
       return -1;
    }
    /*@}*/

    /**@name Modifying methods */
    /*@{*/
    /** Purge all process ids from the process array. */
    inline void clear() {
       _procs.clear();
       _free_procs.clear();
    }
    /** Append a process id to the end of the vector of all processes and mark
       it as free. */
   inline void add_proc(int proc_id) {
       _procs.push_back(proc_id);
       // the new proc is free to begin with
       _free_procs.push_back(proc_id);
   }
    /** Append the processes in <code>[first,last)</code> to the end of
       the vector of all processes and mark them as free. */
   inline void add_procs(BCP_vec<int>::const_iterator first,
                        BCP_vec<int>::const_iterator last) {
       _procs.insert(_procs.end(), first, last);
       // the new procs are free to begin with
       _free_procs.insert(_free_procs.end(), first, last);
   }
    /** Delete the process (whose id is the argument) from the vector of
       all processes (and also from the vector of free processes if
       applicable) */
    inline void delete_proc(const int proc) { 
       int i; 
       for (i = _free_procs.size() - 1; i >= 0; --i) {
	 if (_free_procs[i] == proc) {
	   _free_procs.erase(_free_procs.begin() + i);
	   break;
	 }
       }
       for (i = _procs.size() - 1; i >= 0; --i) {
	 if (_procs[i] == proc) {
	   _procs.erase(_procs.begin() + i);
	   break;
	 }
       }
    }

    // *FIXME* check if the process is in _procs! 
    /** Append the process to the end of the vector of free processes.
       The process to be set free must already be in the vector of all
       processes. */
    void set_proc_free(int proc) {
       _free_procs.push_back(proc);
    }
    /*@}*/
};

#endif
