// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PROCESS_H
#define _BCP_PROCESS_H

#if 1
   #define TMDBG  
   #define LPDBG 
#else
   #define TMDBG   printf("TMDBG: %s:%i\n", __FILE__, __LINE__)
   #define LPDBG   printf("LPDBG: %s:%i\n", __FILE__, __LINE__)
#endif

class BCP_buffer;

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

// (C) 2007 Copyright International Business Machines Corporation
// All Rights Reserved.
// This code is published under the Common Public License.
//
// Authors :
// Pierre Bonami, International Business Machines Corporation
// Andreas Waechter, International Business Machines Corporation
// Laszlo Ladanyi, International Business Machines Corporation
//
// Date : 10/03/2007

#include <map>
#include <vector>
#include <cmath>
#include "CoinHelperFunctions.hpp"

class BCP_scheduler {
public:
  /** Default constructor.*/
  BCP_scheduler();

  /** Method for setting scheduler parameters.
   * \param OverEstimationStatic: Factor for providing more IDs in static strategy.
   * \param SwitchToRateThreshold: When more than SwitchToRateThreshold times the number of strong-branching CPUs are busy, which to rate-based strategy.
   * \param TimeRootNodeSolve: Time to solve root node NLP (in secs)
   * \param FactorTimeHorizon: This number times TimeRootNodeSolve is used to compute the rates
   * \param OverEstimationRate: Factor for providing more IDs in rate-based strategy.
   * \param MaxNodeIdRatio: At most this fraction of the total number of ids can be used as a nodeid.
   * \param MaxNodeIdNum: At most this many ids can be used as a nodeid.
   */
  void setParams(double OverEstimationStatic,
		 double SwitchToRateThreshold,
		 double TimeRootNodeSolve,
		 double FactorTimeHorizon,
		 double OverEstimationRate,
		 double MaxNodeIdRatio,
		 int    MaxNodeIdNum,
		 int    MaxSbIdNum,
		 int    MinSbIdNum);

  /** Pass in a list of freeIds_ to add.*/
  void add_free_ids(int numIds, const int* ids);
  /** Request for a number of id's to do some strong branching.
      NOTE: ids must be already allocated to be at least \c numIds size.
    * \param numIds : number of ids requested
    * \param ids : filled vector with the number of ids served.
    * \return number of ids served.
  */
  int request_sb_ids(int numIds, int* ids);
  /** Gives back to scheduler an id used for strong branching.*/
  void release_sb_id(int id);

  /** Request an id for processing nodes.
      \return id number or -1 if none is available. */
  int request_node_id();
  /** Give back an id to scheduler used for processing a node */
  void release_node_id(int id);
  /** Decide if there is an id that can be returned for processin a node */
  inline bool has_free_node_id() const {
    return (!freeIds_.empty() && maxNodeIds_ > numNodeIds_);
  }
  /** Return the number of busy LP processes */
  inline int numNodeIds() const {
    return numNodeIds_;
  }
  /** Return the maximum possible number of busy LP processes */
  inline int maxNodeIds() const {
    return maxNodeIds_;
  }
  /** Return how much time did process p spent idling as a node process */
  inline double node_idle(int p) {
    return node_idle_time_[p];
  }
  /** Return how much time did process p spent idling as a SB process */
  inline double sb_idle(int p) {
    return sb_idle_time_[p];
  }
  /** Update idle times with the last idle time */
  void update_idle_times();

private:
  /** Compute max allowed allocation of CPUs.*/
  int max_id_allocation(int numIds);
  /** Update the counts and the static_ flag */
  void update_rates(int add_req, int add_rel);

private:
  /** the SB idle time for each process */
  std::map<int, double> sb_idle_time_;
  /** the node idle time for each process */
  std::map<int, double> node_idle_time_;
  /** the type of the last release (0: algo start, 1: from SB, 2: from node */
  std::map<int, double> last_release_type_;
  /** when was the process release last time */
  std::map<int, double> last_release_time_;
  /** Store the total number of CPUs.*/
  int totalNumberIds_;
  /** List of free CPUs ids.*/
  std::vector<int> freeIds_;
  /** number of lp ids served.*/
  int numNodeIds_;
  /** The maximum number of lp ids that can be served */
  int maxNodeIds_;

  /** At most this fraction of the total number of ids can be used as a
   * nodeid. */
  double maxNodeIdRatio_;
  /** At most this many ids can be used as a nodeid. This is a parameter to
      the class. The true max is stored in \c maxNodeIds_ */
  int    maxNodeIdNum_;
  /** Upper threshold for IDs returned at a request. If more than that number
      are requested (and are available) we still return this many. */
  int    maxSbIds_;
  /** Lower threshold for IDs returned at a request.  If less than
      that number are requested, we still return less. */
  int    minSbIds_;

  /** overestimation factor for static strategy */
  double rho_static_;
  /** percentage threshold to swtich to rate-based strategy */
  double switch_thresh_;
  /** Number of seconds in time horizon for rate computation. */
  int numSecRateInterval_;
  /** vector for counting id requests per time unit */
  std::vector<int> request_counts_;
  /** total number of requests in considered time interval */
  int request_counts_tot_;
  /** vector for counting released sb id requests per time unit */
  std::vector<int> release_counts_;
  /** total number of releases in considered time interval */
  int release_counts_tot_;
  /** Array counter */
  int counts_ptr_;
  /** Time stamp of last request or release */
  time_t time_last_action_;
  /** overestimation factor for rate-based strategy */
  double rho_rate_;
  /** flag indicating whether we are in the static or the rate-based
   *  phase */
  bool static_;
  /** flag indicating whether we have rate information (i.e., the time
      horizon has passed at least once) */
  bool have_rates_;
};

#endif
