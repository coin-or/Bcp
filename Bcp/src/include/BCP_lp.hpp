// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_H
#define _BCP_LP_H

#include <cfloat>

#include "BCP_math.hpp"
#include "BCP_enum.hpp"
#include "BCP_enum_process_t.hpp"
#include "BCP_vector.hpp"

#include "BCP_lp_param.hpp"
#include "BCP_parameters.hpp"

#include "BCP_buffer.hpp"
#include "BCP_process.hpp"

//#############################################################################
class BCP_lp_user;
class OsiSolverInterface;
class BCP_message_environment;

class BCP_lp_result;

class BCP_problem_core;
class BCP_problem_core_change;

class BCP_var;
class BCP_cut;
class BCP_var_indexed;

class BCP_col;
class BCP_row;

class BCP_solution;

class BCP_var_set_change;
class BCP_cut_set_change;

class BCP_lp_var_pool;
class BCP_lp_cut_pool;

class BCP_lp_node;
class BCP_lp_parent;

//#############################################################################

// Everything in BCP_lp_prob is public. If the user wants to shoot herself in
// the leg she can do it.

/** NO OLD DOC

*/

class BCP_lp_statistics {
public:
    /** */
    double time_feas_testing;
    /** */
    double time_cut_generation;
    /** */
    double time_var_generation;
    /** */
    double time_heuristics;
    /** */
    double time_lp_solving;
    /** */
    double time_branching;

public:
    /** The contsructor just zeros out every timing data */
    BCP_lp_statistics() : 
	time_feas_testing(0),
	time_cut_generation(0),
	time_var_generation(0),
	time_heuristics(0),
	time_lp_solving(0),
	time_branching(0)
    {}

    /**@name Packing and unpacking */
    /*@{*/
    /** */
    void pack(BCP_buffer& buf);
    /** */
    void unpack(BCP_buffer& buf);
    /*@}*/

    /** Print out the statistics */
    void display() const;

    /** Add the argument statistics to this one. This method is used when
	multiple LP processes are running and their stats need to be combined. */
    void add(const BCP_lp_statistics& stat);
};

/** NO OLD DOC

*/

class BCP_lp_prob : public BCP_process {
private:
    /**@name Disabled methods */
    /*@{*/
    /** */
    BCP_lp_prob(const BCP_lp_prob&);
    /** */
    BCP_lp_prob& operator=(const BCP_lp_prob&);
    /*@}*/

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_lp_prob(BCP_proc_id* my_id, BCP_proc_id* parent);
    /** */
    virtual ~BCP_lp_prob();
    /*@}*/

public:
    //-------------------------------------------------------------------------
    // The unpacking classes
    /**@name Data members */
    /*@{*/
    //------------------------------------------------------------------------
    // User provided members
    /**@name User provided members */
    /*@{*/
    /** */
    BCP_lp_user* user;
    /** */
    OsiSolverInterface* master_lp;
    /** */
    OsiSolverInterface* lp_solver;
    /** */
    BCP_message_environment* msg_env;
    /*@}*/

    /**@name Parameters */
    /*@{*/
    /** */
    BCP_parameter_set<BCP_lp_par> par;
    /*@}*/

    //------------------------------------------------------------------------
    // the description of the core
    /**@name Description of the core of the problem */
    /*@{*/
    /** */
    BCP_problem_core* core;
    /** */
    BCP_problem_core_change* core_as_change;
    /*@}*/
   
    //------------------------------------------------------------------------
    // the search tree node we are working on and its parent
    /**@name Current search tree node and its parent */
    /*@{*/
    /** */
    BCP_lp_node* node;
    /** */
    BCP_lp_parent* parent;
    /*@}*/
   
    //------------------------------------------------------------------------
    // Info while processing a particular node. Need to be updated when
    // starting a new node.
    /**@name Information needed for processing a node

    Need to be updated when starting a new node. */
    /*@{*/
    /** */
    BCP_lp_result* lp_result;
    /** */
    int var_bound_changes_since_logical_fixing;
    /** */
    BCP_vec<BCP_cut*> slack_pool;
    /** */
    BCP_lp_var_pool* local_var_pool;
    /** */
    BCP_lp_cut_pool* local_cut_pool;

    // The next/last index we can assign to a newly generated var/cut
    /** */
    int next_var_index;
    /** */
    int last_var_index;
    /** */
    int next_cut_index;
    /** */
    int last_cut_index;
    /*@}*/
   
    //------------------------------------------------------------------------
    // time measurements
    /**@name Time measurement */
    /*@{*/
    /** */
    BCP_lp_statistics stat;
    /*@}*/

    //------------------------------------------------------------------------
    // Internal members
    //------------------------------------------------------------------------
    /**@name Internal data members */
    /*@{*/
    /** */
    double upper_bound;
    /** */
    int phase;
    /** */
    int no_more_cuts_cnt; // a counter for how many places we got to get
    // NO_MORE_CUTS message to know for sure not to
    // expect more.
    /** */
    int no_more_vars_cnt; // similar for vars
    /*@}*/
   
    // message passing related fields
    /**@name Message passing related fields */
    /*@{*/
    /** */
    //    BCP_proc_id* tree_manager;
    /** */
    BCP_buffer  msg_buf;
    /*@}*/
    //------------------------------------------------------------------------
    // Results of BCP_lp_user::process_lp_result() are stored here
    //------------------------------------------------------------------------
    bool user_has_lp_result_processing;
    BCP_vec<BCP_cut*> new_cuts;
    BCP_vec<BCP_row*> new_rows;
    BCP_vec<BCP_var*> new_vars;
    BCP_vec<BCP_col*> new_cols;
    BCP_solution* sol;
    double new_true_lower_bound;

    //------------------------------------------------------------------------
    /*@}*/  // end of data members 
   
public:
    /**@name Methods to pack/unpack objects */
    /*@{*/
    /** */
    void pack_var(BCP_process_t target_proc, const BCP_var& var);
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
    // member functions related to accessing the parameters
    /**@name Acessing parameters */
    /*@{*/
    /** */
    inline char
    param(BCP_lp_par::chr_params key) const        { return par.entry(key); }
    /** */
    inline int
    param(BCP_lp_par::int_params key) const        { return par.entry(key); }
    /** */
    inline double
    param(BCP_lp_par::dbl_params key) const        { return par.entry(key); }
    /** */
    inline const BCP_string&
    param(BCP_lp_par::str_params key) const        { return par.entry(key); }
    /** */
    inline const BCP_vec<BCP_string>&
    param(BCP_lp_par::str_array_params key) const  { return par.entry(key); }
    /** */
    inline double granularity() const {
	return param(BCP_lp_par::Granularity);
    }
    /*@}*/

    //-------------------------------------------------------------------------
    /**@name Accessing bounds */
    /*@{*/
    /** */
    inline bool has_ub() const { return upper_bound < BCP_DBL_MAX / 10; }
    /** */
    inline double ub() const   { return upper_bound; }
    /** */
    inline bool ub(double new_ub) {
	if (new_ub < upper_bound){
	    upper_bound = new_ub;
	    return true;
	}
	return false;
    }
    /** */
    inline bool over_ub(double lb) const {
	return has_ub() && lb >= upper_bound - granularity();
    }
    /*@}*/
    /*@}*/  // end of query methods 
    //-------------------------------------------------------------------------
    virtual BCP_buffer& get_message_buffer() { return msg_buf; }
    virtual void process_message();
};

#endif
