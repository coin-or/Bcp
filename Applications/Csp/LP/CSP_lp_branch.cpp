// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include <CoinHelperFunctions.hpp>

#include <OsiClpSolverInterface.hpp>

#include "BCP_lp.hpp"

#include "CSP.hpp"
#include "CSP_lp.hpp"
#include "CSP_var.hpp"

// Can only inline a function if it's defined before it's used.

inline double evaluate(const BCP_var* var, const double sol){
  double value= -DBL_MAX;
  // these ub's and lb;s should be updated, but beware.
  const double ub = var->ub();
  const double lb = var->lb();
  const double obj = var->obj();
  double frac = sol-floor(sol);
  
  if (frac>1e-6 && frac<1-1e-6){
    value = obj*(ub-sol)*((sol-lb)/((ub-lb)*(ub-lb)))*(1-frac)*(frac);
  }
  
  return value;
}
  

//#############################################################################
//#############################################################################

BCP_lp_branching_object*
CSP_lp::branch_on_half(const BCP_lp_result& lpres,
		       const BCP_vec<BCP_var*>& vars)
{
  const int varnum = vars.size();
  const double* x  = lpres.x ();

  // right now the objective function values are all 1
  // in the future this might change.
  // we want to branch on a var that's close to the half
  // and has a large objective function value
  // and is in the middle of it's [lb, ub] range. 

  // We'll define a "scoring function" and 
  // branch on the var with the  maximum function  
   
  int bestIndex=-1;
  double bestValue = -DBL_MAX; // evaluation value

  for (int i = 0; i < varnum; ++i){

    // this function evaluates the i'th variable
    // compares it to the current best,
    // and puts the better of the two in index & value

    const double value = evaluate(vars[i], x[i]);

    // compare
    if (value>bestValue){
      bestValue=value;
      bestIndex=i;
    }
  }

  BCP_vec<int> positions; // index of solution variable I select

  BCP_vec<BCP_lp_branching_object*> candidates;

  if (bestIndex == -1){
    throw BCP_fatal_error("Crud! We can't select a branching variable\n");
  }

  positions.push_back(bestIndex);

  // this populates the BCP_lp_branching_object
  // the only branching objects that BCP will create are
  // ones that split into two pieces...
  // So, I have a "zero" and "one" child for each
  // position I've passed in
  append_branching_vars(x,vars,positions,candidates);

  // the compiler is free to destroy local 
  // objects on the return line.
  // So, to make sure the compiler doesn't destroy
  // the object before doing the return, we are not 
  // just saying 
  // return candidates[0];
  // probably it would be okay but....it's "safer"

  BCP_lp_branching_object * can = candidates[0];
  return can;
}


//#############################################################################

BCP_branching_decision
CSP_lp::select_branching_candidates(const BCP_lp_result& lpres,
				    const BCP_vec<BCP_var*>& vars,
				    const BCP_vec<BCP_cut*>& cuts,
				    const BCP_lp_var_pool& local_var_pool,
				    const BCP_lp_cut_pool& local_cut_pool,
				    BCP_vec<BCP_lp_branching_object*>& cands,
				    bool force_branch)
{
  // For starters we'll select one variable near 1/2 and branch on it. 

  // if there is any column generated, then do not branch
  if (local_var_pool.size() > 0)
    return BCP_DoNotBranch;

  // This is one global setting 
  switch (par.entry(CSP_lp_par::BranchingStrategy)) {
    // RLH: for the future, think about branch on bits
    // nice, becuase no cuts addedd but how do you select the bit?
  case 0: // branch on var nearest 1/2
    cands.push_back(branch_on_half(lpres, vars)); 
    break;
  default:
    throw BCP_fatal_error("\
unkown branching strategy requested,\n\
see:Parameter CSP_BranchingStrategy\n");
  }

  if (cands.size() == 0) {
    throw BCP_fatal_error("forgot to branch?\n");
  }

  return BCP_DoBranch;
}

//#############################################################################

// RLH: what do we want to do for each child. 
// By default the "best" (lowest obj value) one is kept. 
// What would be a good rule for us?
void
CSP_lp::set_actions_for_children(BCP_presolved_lp_brobj* best)
{
  // by default every action is set to BCP_ReturnChild
  // this means return child to TM,
  // Other options are BCP_KeepChild or something like BCP_DiscardChild
  // BCP_KeepChild means you'll dive...

  // Change action[1] to BCP_KeepChild, this means you dive on the 
  // up branch. (this may or maynot be documents).


  // action[0] refers to what to do to the "zero" child.

  // best is presolved lp action, action is what to do for each child
  // (documention let's know it's already allocated)
  // BCP_vec it's self is a template
  // why a ref? Laci prefers them if he knows the obj already exists
  // when to use ptr over ref?
  // use ptr if you want to change the object, or don't know if obj exist
  // use ref if you want short hand
  // ref has to be set at declaration WHICH MUST EXIST AND IT CAN"T BE CHANGED
  BCP_vec<BCP_child_action>& action = best->action();

  // Supose the branching just has two entries
  // Dive on the "1" branch.
  action[1] = BCP_KeepChild;

  // CSP: we'll have two children
  // Unless upper end is infeasible (? is this possible)
  // Keep the "high-end" child, the one that says you have to make so many
  // Jon's "L" problem.
}
