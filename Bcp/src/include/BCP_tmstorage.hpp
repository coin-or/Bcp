// Copyright (C) 2007, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TMSTORAGE_H
#define _BCP_TMSTORAGE_H

#include <map>

#include "BCP_enum_process_t.hpp"
#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_parameters.hpp"
#include "BCP_process.hpp"
#include "BCP_USER.hpp"
#include "BCP_problem_core.hpp"

//#############################################################################

class BCP_var_algo;
class BCP_cut_algo;

class BCP_node_change;

//#############################################################################

class BCP_ts_prob;
class BCP_ts_user;

//#############################################################################

struct BCP_ts_par {

    enum chr_params {
	MessagePassingIsSerial,
	end_of_chr_params
    };

    enum int_params {
        /** The maximum size of the memory heap the TS can use. The amount of
	    free memory the TS indicates towards the TM uses this value. If
	    positive, it's the number of megabytes. 0 indicates the TS should
	    attempt to find this out. -1 indicates that the TS can use as much
	    memory as it wants. Default: 0. */
        MaxHeapSize,
	NiceLevel,
	end_of_int_params
    };

    enum dbl_params{
	end_of_dbl_params
    };

    enum str_params{
	LogFileName,
	end_of_str_params
    };

    enum str_array_params{
	end_of_str_array_params
    };

};
    
//#############################################################################

/** Same as BCP_tm_node_data, just there's no need to use smart pointers in
    this process */
struct BCP_ts_node_data {
    BCP_node_change* _desc;
    BCP_user_data*   _user;
};

//#############################################################################

class BCP_ts_prob : public BCP_process {
private:
    /**@name Disabled methods */
    /*@{*/
    /** The copy constructor is declared but not defined to disable it. */
    BCP_ts_prob(const BCP_ts_prob&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_ts_prob& operator=(const BCP_ts_prob&);
    /*@}*/

public:
    /** */
    BCP_ts_user* user;
    /** */
    BCP_user_pack* packer;
    /** */
    BCP_message_environment* msg_env;
    /** */
    BCP_parameter_set<BCP_ts_par> par;
    /** */
    BCP_buffer msg_buf;
    /** */
    BCP_problem_core* core;
    /** a list of indices of nodes/vars/cuts that are requested/tobe deleted */
    BCP_vec<int> indices;
    /** positions in the TM of requested nodes/vars/cuts */
    BCP_vec<int> positions;
    /** */
    std::map<int, BCP_ts_node_data*> nodes; // *FIXME*: maybe hash_map better ?
    /** */
    std::map<int, BCP_var_algo*> vars; // *FIXME*: maybe hash_map better ?
    /** */
    std::map<int, BCP_cut_algo*> cuts; // *FIXME*: maybe hash_map better ?

public:
    /** */
    BCP_ts_prob(int my_id, int parent) :
	BCP_process(my_id, parent),
	user(0),
	msg_env(0),
	core(new BCP_problem_core) {}
    /** */
    virtual ~BCP_ts_prob();

public:
    virtual BCP_buffer& get_message_buffer() { return msg_buf; }
    virtual void process_message();
};

//#############################################################################

class BCP_ts_user : public BCP_user_class {
private:
    BCP_ts_prob * p;
public:
    /**@name Methods to set and get the pointer to the BCP_ts_prob
       object. It is unlikely that the users would want to muck around with
       these (especially with the set method!) but they are here to provide
       total control.
    */
    /*@{*/
    /// Set the pointer
    void setTsProblemPointer(BCP_ts_prob * ptr) { p = ptr; }
    /// Get the pointer
    BCP_ts_prob * getTsProblemPointer() { return p; }
    /*@}*/

    /**@name Methods to get/set BCP parameters on the fly */
    /*@{*/
    inline char              get_param(const BCP_ts_par::chr_params key) const{
	return p->par.entry(key); }
    inline int               get_param(const BCP_ts_par::int_params key) const{
	return p->par.entry(key); }
    inline double            get_param(const BCP_ts_par::dbl_params key) const{
	return p->par.entry(key); }
    inline const BCP_string& get_param(const BCP_ts_par::str_params key) const{
	return p->par.entry(key); }

    inline void set_param(const BCP_ts_par::chr_params key, const bool val) {
	p->par.set_entry(key, val); }
    inline void set_param(const BCP_ts_par::chr_params key, const char val) {
	p->par.set_entry(key, val); }
    inline void set_param(const BCP_ts_par::int_params key, const int val) {
	p->par.set_entry(key, val); }
    inline void set_param(const BCP_ts_par::dbl_params key, const double val) {
	p->par.set_entry(key, val); }
    inline void set_param(const BCP_ts_par::str_params key, const char * val) {
	p->par.set_entry(key, val); }

    //=========================================================================
    /**@name Constructor, Destructor */
    /*@{*/
    BCP_ts_user() : p(0) {}
    /** Being virtual, the destructor invokes the destructor for the real type
	of the object being deleted. */
    virtual ~BCP_ts_user() {}
    /*@}*/

    //=========================================================================
    // Here are the user defined functions. For each of them a default is
    // given which can be overridden when the concrete user class is defined.
    //=========================================================================

    /** Unpack the initial information sent to the LP process by the Tree
        Manager. This information was packed by the method
        BCP_ts_user::pack_module_data() invoked with \c
        BCP_ProcessType_TS as the third (target process type) argument.
	
        Default: empty method. */
    virtual void
    unpack_module_data(BCP_buffer & buf);
};

#endif

