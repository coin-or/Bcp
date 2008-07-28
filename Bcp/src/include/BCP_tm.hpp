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

#define BCP_ONLY_LP_PROCESS_HANDLING_WORKS

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

class BCP_tm_stat {
private:
  int num_lp;
  // An array whose i-th entry indicates what was the totel wait time when
  // exactly i LP processes were working
  double* wait_time;
  // An array whose i-th entry indicates what was the totel queue length when
  // exactly i LP processes were working
  double* sumQueueLength;
  // An array whose i-th entry indicates how many times we have sampled the
  // queue length when exactly i LP processes were working
  int* numQueueLength;
  int cnt; // how many times we have printed stats
public:
  BCP_tm_stat() :
      num_lp(0),
      wait_time(NULL),
      sumQueueLength(NULL),
      numQueueLength(NULL),
      cnt(0) {}
  ~BCP_tm_stat() {
    delete[] wait_time;
    delete[] sumQueueLength;
    delete[] numQueueLength;
  }
  void set_num_lp(int num) {
    delete[] wait_time;
    delete[] sumQueueLength;
    delete[] numQueueLength;
    num_lp = num;
    wait_time = new double[num+1];
    sumQueueLength = new double[num+1];
    numQueueLength = new int[num+1];
    for (int i = 0; i <= num_lp; ++i) {
      wait_time[i] = 0;
      sumQueueLength[i] = 0;
      numQueueLength[i] = 0;
    }
  }
  void update_wait_time(int i, double t) { wait_time[i] += t; }
  void update_queue_length(int i, int len) {
    sumQueueLength[i] += len;
    ++numQueueLength[i];
  }
  void print(bool final, double t);
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
  std::vector<int> ts_procs;
  std::vector<int> lp_procs;
  /** */
  BCP_scheduler lp_scheduler;
  /** members to measure how long it took to process the root node. Needed for
      the scheduler (both are in wallclock) */
  double root_node_sent_;
  double root_node_received_;
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
  BCP_tm_stat stat;

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

