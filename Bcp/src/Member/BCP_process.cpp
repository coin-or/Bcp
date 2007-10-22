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

#include "CoinTime.hpp"
#include "BCP_process.hpp"

BCP_scheduler::BCP_scheduler():
    totalNumberIds_(0),
    freeIds_(),
    numNodeIds_(0),
    maxNodeIds_(1),
    maxNodeIdRatio_(1.0),
    maxNodeIdNum_(1)
{}

void
BCP_scheduler::setParams(double OverEstimationStatic,
			 double SwitchToRateThreshold,
			 double TimeRootNodeSolve,
			 double FactorTimeHorizon,
			 double OverEstimationRate,
			 double MaxNodeIdRatio,
			 int    MaxNodeIdNum,
			 int    MaxSbIds,
			 int    MinSbIds)
{
  rho_static_ = OverEstimationStatic;
  switch_thresh_ = SwitchToRateThreshold;
  numSecRateInterval_ = (int)ceil(TimeRootNodeSolve*FactorTimeHorizon);
  request_counts_.reserve(numSecRateInterval_+1);
  request_counts_.insert(request_counts_.end(), numSecRateInterval_+1, -1);
  request_counts_tot_ = 0;
  release_counts_.reserve(numSecRateInterval_+1);
  release_counts_.insert(release_counts_.end(), numSecRateInterval_+1, -1);
  release_counts_tot_ = 0;
  counts_ptr_ = 0;
  time_last_action_ = 0;
  static_ = true;
  have_rates_ = false;
  rho_rate_ = OverEstimationRate;
  maxNodeIdRatio_ = MaxNodeIdRatio;
  maxNodeIdNum_ = MaxNodeIdNum;
  maxNodeIds_ = CoinMin((int)floor(maxNodeIdRatio_ * totalNumberIds_),
			maxNodeIdNum_);
  if (maxNodeIds_ == 0) {
    maxNodeIds_ = 1;
  }
  maxSbIds_ = MaxSbIds;
  printf("Setting max SbIds to %i",maxSbIds_);
  minSbIds_ = MinSbIds;
}

//------------------------------------------------------------------------------
void
BCP_scheduler::add_free_ids(int numIds, const int* ids)
{
  const int oldsize = freeIds_.size();
  freeIds_.insert(freeIds_.end(), ids, ids+numIds);
  totalNumberIds_ += numIds;
  maxNodeIds_ = CoinMin((int)floor(maxNodeIdRatio_ * totalNumberIds_),
			maxNodeIdNum_);
  for (int i = 0; i < numIds; ++i) {
    sb_idle_time_[ids[i]] = 0.0;
    node_idle_time_[ids[i]] = 0.0;
    last_release_time_[ids[i]] = 0.0;
    last_release_type_[ids[i]] = 0;
  }
}

//------------------------------------------------------------------------------

int
BCP_scheduler::request_sb_ids(int numIds, int* ids)
{
  // increase the count for requests by one
  update_rates(1, 0);

  numIds = CoinMin(numIds, max_id_allocation(numIds));
  if (numIds==0) return 0;

  const int newsize = freeIds_.size() - numIds;
  CoinDisjointCopyN(&freeIds_[newsize], numIds, ids);
  freeIds_.erase(freeIds_.begin()+newsize, freeIds_.end());
  const double t = CoinWallclockTime();
  for (int i = 0; i < numIds; ++i) {
    const int id = ids[i];
    if (last_release_type_[id] != 2) {
      sb_idle_time_[id] += t - last_release_time_[id];
    } else {
      node_idle_time_[id] += t - last_release_time_[id];
    }
  }
  return numIds;
}

void
BCP_scheduler::release_sb_id(int id)
{
  // increase the count for releases by one
  update_rates(0, 1);
  freeIds_.push_back(id);
  last_release_time_[id] = CoinWallclockTime();
  last_release_type_[id] = 1;
}

//------------------------------------------------------------------------------

/** Request an id for processing nodes.
    \return id number or -1 if none is available. */
int
BCP_scheduler::request_node_id()
{
  if (freeIds_.empty() || numNodeIds_ == maxNodeIds_) return -1;
  numNodeIds_ ++;
  int id = freeIds_.back();
  freeIds_.pop_back();
  return id;
}

void
BCP_scheduler::release_node_id(int id)
{
  // increase the count for releases by one
  update_rates(0, 1);
  freeIds_.push_back(id);
  last_release_time_[id] = CoinWallclockTime();
  last_release_type_[id] = 2;
  numNodeIds_--;
}

//------------------------------------------------------------------------------

void
BCP_scheduler::update_idle_times()
{
  const double t = CoinWallclockTime();
  for (int i = freeIds_.size()-1; i >= 0; --i) {
    const int id = freeIds_[i];
    if (last_release_type_[id] != 2) {
      sb_idle_time_[id] += t - last_release_time_[id];
    } else {
      node_idle_time_[id] += t - last_release_time_[id];
    }
  }
}
    

//------------------------------------------------------------------------------

void
BCP_scheduler::update_rates(int add_req, int add_rel)
{
  // Update the counts for the requests
  time_t time_now = time(NULL);
  if (time_now == time_last_action_) {
    request_counts_[counts_ptr_] += add_req;;
    release_counts_[counts_ptr_] += add_rel;;
  }
  else if (time_last_action_ == 0) {
    counts_ptr_ = 0;
    request_counts_[counts_ptr_] = add_req;
    release_counts_[counts_ptr_] = add_rel;
    time_last_action_ = time_now;
  }
  else {
    while (time_last_action_ < time_now) {
      request_counts_tot_ += request_counts_[counts_ptr_];
      release_counts_tot_ += release_counts_[counts_ptr_];
      counts_ptr_++;
      if (counts_ptr_ > numSecRateInterval_) {
	counts_ptr_ = 0;
	have_rates_ = true;
      }
      if (have_rates_) {
	request_counts_tot_ -= request_counts_[counts_ptr_];
	release_counts_tot_ -= release_counts_[counts_ptr_];
      }
      request_counts_[counts_ptr_] = 0;
      release_counts_[counts_ptr_] = 0;

      time_last_action_++;
    }
    request_counts_[counts_ptr_] = add_req;
    release_counts_[counts_ptr_] = add_rel;

    static_ = (!have_rates_ || freeIds_.size() >= (1.-switch_thresh_)*(totalNumberIds_-numNodeIds_) );
  }
}

//------------------------------------------------------------------------------

int 
BCP_scheduler::max_id_allocation(int numIds)
{
  double dretval;
  int retval;

  /* FIXME: this might be incorrect if in request_node_ids() more than 1 id is
     requested. If only LPs processes are used then it's correct. */

  const int numFree = freeIds_.size();

  double expRate =
    (double)CoinMin(2*numNodeIds_,maxNodeIds_) / (double)numNodeIds_;

  if (static_) {
    dretval = (rho_static_ * (double)(totalNumberIds_-numNodeIds_*expRate) /
	       (double)(numNodeIds_*expRate) );
  }
  else {
    if (request_counts_tot_ == 0) {
      dretval = numFree;
    }
    else {
      dretval = (rho_rate_ * (double)(release_counts_tot_) /
		(double)(request_counts_tot_));
    }
  }
  retval = CoinMin(numFree, (int)floor(dretval));

  if (numIds >= minSbIds_ && retval < minSbIds_) {
    retval = 0;
  }
  if (retval > maxSbIds_) {
    retval = maxSbIds_;
  }

  // At this point, we only want to send an odd number of processors
  if (retval && (retval & 1) == 0) {
    --retval;
  }

  return retval;
}
