// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_H
#define _BCP_TM_H

#include <queue>
#include <map>

#include "BCP_math.hpp"
#include "BCP_buffer.hpp"
#include "BCP_enum.hpp"
#include "BCP_enum_process_t.hpp"
#include "BCP_tm_node.hpp"

#include "BCP_tm_param.hpp"
#include "BCP_lp_param.hpp"
#include "BCP_cg_param.hpp"
#include "BCP_vg_param.hpp"
//#include "BCP_cp_param.hpp"
//#include "BCP_vp_param.hpp"
#include "BCP_parameters.hpp"

#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_process.hpp"

//#############################################################################
class BCP_warmstart;
class BCP_solution;

class BCP_tm_user;

class BCP_var;
class BCP_cut;

class BCP_var_set_change;
class BCP_cut_set_change;

class BCP_problem_core;
class BCP_problem_core_change;

class BCP_lp_statistics;

//#############################################################################

/** NO OLD DOC */

class BCP_slave_processes {

    /**@name Disabled methods */
    /*@{*/
    /** The copy constructor is declared but not defined to disable it. */
    BCP_slave_processes(const BCP_slave_processes&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_slave_processes& operator=(const BCP_slave_processes&);
    /*@}*/

public:
    /**@name Data members */
    /*@{*/
    /** */
    BCP_proc_array* all;
    /** */
    BCP_proc_array* lp;
    /** */
    BCP_proc_array* cg;
    /** */
    BCP_proc_array* cp;
    /** */
    BCP_proc_array* vg;
    /** */
    BCP_proc_array* vp;
    /*@}*/

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_slave_processes() : all(0), lp(0), cg(0), cp(0), vg(0), vp(0) {}
    /** */
    ~BCP_slave_processes() {
	delete all;   all = 0;
	delete lp;   lp = 0; 
	delete cp;   cp = 0; 
	delete vp;   vp = 0; 
	delete cg;   cg = 0; 
	delete vg;   vg = 0; 
    }
    /*@}*/
};


/** NO OLD DOC */

struct BCP_slave_params {
    /** */
    BCP_parameter_set<BCP_lp_par> lp;
    //   BCP_parameter_set<BCP_cp_par> cp;
    //   BCP_parameter_set<BCP_vp_par> vp;
    /** */
    BCP_parameter_set<BCP_cg_par> cg;
    /** */
    BCP_parameter_set<BCP_vg_par> vg;
};

//-----------------------------------------------------------------------------

/** NO OLD DOC */

struct BCP_tm_flags {
    /** Set to true if the result of root pricing is already unpacked.
	Important in a single process environment, so we don't unpack things
	twice. */
    bool root_pricing_unpacked; // set if the result of root pricing is already
    // unpacked. important in a single process
    // environment, so we don't unpack things twice
};

//-----------------------------------------------------------------------------

/** NO OLD DOC */

class BCP_tm_prob : public BCP_process {
private:
    /**@name Disabled methods */
    /*@{*/
    /** The copy constructor is declared but not defined to disable it. */
    BCP_tm_prob(const BCP_tm_prob&);
    /** The assignment operator is declared but not defined to disable it. */
    BCP_tm_prob& operator=(const BCP_tm_prob&);
    /*@}*/

    //-------------------------------------------------------------------------
public: // Data members
    /**@name User provided members */
    /*@{*/
    /** */
    BCP_tm_user* user;
    /** */
    BCP_message_environment* msg_env;
    /*@}*/

    /**@name Statistics in the other processes */
    BCP_lp_statistics* lp_stat;

    /** */
    BCP_solution* feas_sol;

    /**@name Parameters */
    /*@{*/
    /** */
    BCP_parameter_set<BCP_tm_par> par;
    /** */
    BCP_slave_params slave_pars;
    /*@}*/

    /**@name Flags */
    /*@{*/
    // flags to signal various things
    /** */
    BCP_tm_flags flags;
    /*@}*/

    /**@name Message passing related fields */
    /*@{*/
    /** */
    BCP_buffer msg_buf;
    /** */
    BCP_slave_processes slaves;
    /*@}*/

    //-------------------------------------------------------------------------
    /** */
    double upper_bound;
    /** */
    double start_time;
    //-------------------------------------------------------------------------
    /**@name The description of the core of the problem */
    /*@{*/
    /** */
    BCP_problem_core* core;
    /** */
    BCP_problem_core_change* core_as_change;
    /*@}*/

    /** */
    int phase;
    /** */
    BCP_column_generation current_phase_colgen;

    /** */
    std::map<int, BCP_var*> vars; // *FIXME*: maybe hash_map better ?
    /** */
    std::map<int, BCP_cut*> cuts; // *FIXME*: maybe hash_map better ?
    /** */
    int next_cut_index_set_start;
    /** */
    int next_var_index_set_start;

    //-------------------------------------------------------------------------
    /** */
    BCP_tree search_tree;
    /** */
    BCP_vec<BCP_tm_node*> active_nodes;
    /** */
    BCP_node_queue candidates;
    /** a vector of nodes to be processed in the next phase */
    BCP_vec<BCP_tm_node*> next_phase_nodes;
    /** */
    BCP_vec<BCP_tm_node*> nodes_to_free;

    //-------------------------------------------------------------------------
    /**@name Vectors indicating the number of leaf nodes assigned to each CP/VP
     */ 
    /*@{*/
    /** */
    BCP_vec< std::pair<BCP_proc_id*, int> > leaves_per_cp;
    /** */
    BCP_vec< std::pair<BCP_proc_id*, int> > leaves_per_vp;
    /*@}*/

    //-------------------------------------------------------------------------

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_tm_prob(BCP_proc_id* my_id, BCP_proc_id* parent);
    /** */
    virtual ~BCP_tm_prob();
    /*@}*/

public:
    /**@name Methods to pack/unpack objects */
    /*@{*/
    /** */
    void pack_var(BCP_process_t target_proc, const BCP_var& var);
    /** */
    BCP_var* unpack_var_without_bcpind(BCP_buffer& buf);
    /** */
    BCP_var* unpack_var();
    /** */
    void pack_cut(BCP_process_t target_proc, const BCP_cut& cut);
    /** */
    BCP_cut* unpack_cut();
    /** */
    void pack_var_set_change(const BCP_var_set_change& ch);
    /** */
    void unpack_var_set_change(BCP_var_set_change& ch);
    /** */
    void pack_cut_set_change(const BCP_cut_set_change& ch);
    /** */
    void unpack_cut_set_change(BCP_cut_set_change& ch);
    /*@}*/
    //-------------------------------------------------------------------------

    /**@name Query methods */
    /*@{*/
    /** */
    inline char
    param(BCP_tm_par::chr_params key) const        { return par.entry(key); }
    /** */
    inline int
    param(BCP_tm_par::int_params key) const        { return par.entry(key); }
    /** */
    inline double
    param(BCP_tm_par::dbl_params key) const        { return par.entry(key); }
    /** */
    inline const BCP_string&
    param(BCP_tm_par::str_params key) const        { return par.entry(key); }
    /** */
    inline const BCP_vec<BCP_string>&
    param(BCP_tm_par::str_array_params key) const  { return par.entry(key); }

    /** */
    inline double granularity() const {
	return param(BCP_tm_par::Granularity);
    }

    //-------------------------------------------------------------------------
    /** */
    inline bool has_ub() const { return upper_bound < BCP_DBL_MAX / 10; }
    /** */
    inline double ub() const { return upper_bound; }
    /** */
    inline bool ub(double new_ub) {
	if (new_ub < upper_bound){
	    upper_bound = new_ub;
	    return true;
	}
	return false;
    }
    /** */
    inline bool over_ub(const double lb) const {
	return lb > upper_bound - param(BCP_tm_par::Granularity);
    }
    /*@}*/
    //-------------------------------------------------------------------------
    virtual BCP_buffer& get_message_buffer() { return msg_buf; }
    virtual void process_message();
};

//#############################################################################

#endif

