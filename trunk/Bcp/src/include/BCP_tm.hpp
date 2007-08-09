// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_H
#define _BCP_TM_H

#include <queue>
#include <map>

#include "CoinSearchTree.hpp"
#include "CoinSmartPtr.hpp"

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
#include "BCP_tmstorage.hpp"

#include "BCP_buffer.hpp"
#include "BCP_message.hpp"
#include "BCP_process.hpp"

#include "BCP_var.hpp"
#include "BCP_cut.hpp"

//#############################################################################
class BCP_warmstart;
class BCP_solution;

class BCP_tm_user;
class BCP_user_pack;

//class BCP_var;
//class BCP_cut;

class BCP_obj_set_change;

class BCP_problem_core;
class BCP_problem_core_change;

class BCP_lp_statistics;

//#############################################################################

#define BCP_CG_VG_PROCESS_HANDLING_BROKEN

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
#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
    /** */
    BCP_proc_array* cg;
#endif
    /** */
    BCP_proc_array* cp;
#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
    /** */
    BCP_proc_array* vg;
#endif
    /** */
    BCP_proc_array* vp;
    /** */
    BCP_proc_array* ts;
    /*@}*/

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_slave_processes() :
      all(0),
      lp(0),
#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
      cg(0),
#endif
      cp(0),
#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
      vg(0),
#endif
      vp(0),
      ts(0) {}
    /** */
    ~BCP_slave_processes() {
	delete all;   all = 0;
	delete lp;   lp = 0; 
	delete cp;   cp = 0; 
	delete vp;   vp = 0; 
#if ! defined(BCP_CG_VG_PROCESS_HANDLING_BROKEN)
	delete cg;   cg = 0; 
	delete vg;   vg = 0; 
#endif
	delete ts;   ts = 0; 
    }
    /*@}*/
};


/** NO OLD DOC */

struct BCP_slave_params {
    /** */
    BCP_parameter_set<BCP_ts_par> ts;
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
    /** A class that holds the methods about how to pack things. */
    BCP_user_pack* packer;

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

    // *FIXME*: maybe hash_map better for the next four?
    /** */
    std::map<int, Coin::SmartPtr<BCP_var> > vars_local; 
    /** */
    std::map<int, int>      vars_remote;
    /** */
    std::map<int, Coin::SmartPtr<BCP_cut> > cuts_local;
    /** */
    std::map<int, int>      cuts_remote;

    /** */
    int next_cut_index_set_start;
    /** */
    int next_var_index_set_start;

    //-------------------------------------------------------------------------
    bool need_a_TS;
    std::map<int, int> ts_space;
  
    //-------------------------------------------------------------------------
    /** */
    BCP_tree search_tree;
    /** A map from the process ids to the nodes (what they work on) */
    std::map<int, BCP_tm_node*> active_nodes;
    /** */
    CoinSearchTreeManager candidate_list;

    /** */
    std::map<int, BCP_tm_node_to_send*> nodes_to_send;
    
    // BCP_node_queue candidates;
    /** a vector of nodes to be processed in the next phase */
    BCP_vec<BCP_tm_node*> next_phase_nodes;
    /** */
    BCP_vec<BCP_tm_node*> nodes_to_free;

    //-------------------------------------------------------------------------
    /**@name Vectors indicating the number of leaf nodes assigned to each CP/VP
     */ 
    /*@{*/
    /** */
    BCP_vec< std::pair<int, int> > leaves_per_cp;
    /** */
    BCP_vec< std::pair<int, int> > leaves_per_vp;
    /*@}*/

    //-------------------------------------------------------------------------

public:
    /**@name Constructor and destructor */
    /*@{*/
    /** */
    BCP_tm_prob();
    /** */
    virtual ~BCP_tm_prob();
    /*@}*/

public:
    /**@name Methods to pack/unpack objects */
    /*@{*/
    /** */
    void pack_var(const BCP_var& var);
    /** */
    BCP_var* unpack_var_without_bcpind(BCP_buffer& buf);
    /** */
    int unpack_var();
    /** */
    void pack_cut(const BCP_cut& cut);
    /** */
    BCP_cut* unpack_cut_without_bcpind(BCP_buffer& buf);
    /** */
    int unpack_cut();
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

