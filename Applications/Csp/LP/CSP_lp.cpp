// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <algorithm>

#include <CoinHelperFunctions.hpp>

#include <OsiClpSolverInterface.hpp>
#include <OsiVolSolverInterface.hpp>

#ifdef CSP_UseCpx
#include <OsiCpxSolverInterface.hpp>
#endif

#ifdef CSP_UseCbc
#include <CbcModel.hpp>
#endif

#include <CglOddHole.hpp>
#include <CglGomory.hpp>
#include <CglKnapsackCover.hpp>
#include <CglLiftAndProject.hpp>
#include <CglProbing.hpp>
#include <CglSimpleRounding.hpp>

#include "BCP_lp.hpp"
#include "BCP_vector.hpp"

#include "CSP.hpp"
#include "CSP_lp.hpp"
#include "CSP_var.hpp"

//#############################################################################
//#############################################################################

// CSP: forget FakeColumnGeneration. Just use TrueColumnGeneration

void
CSP_lp::unpack_module_data(BCP_buffer& buf)
{
  par.unpack(buf); // this is template. don't have to write
  csproblem = new CSPROBLEM();
  csproblem->unpack(buf); // need to write this

  // .. but didn't we say that there was no more condense way
  // to pack the solution, so we'd take the default (?) 

  // "false" tells the destructor of the <what?>
  // not to delete pointers that are passed via a
  // setCsp() method.
   
  colgen = new CSP_colgen(false);
  // creates base matrix & rims
  colgen->setCsp(csproblem,
		 par.entry(CSP_lp_par::PerturbFactor),
		 par.entry(CSP_lp_par::PerturbNum));
}

//#############################################################################
// Only using the simplex solver for now. 
// Note: FCC code overrides the initializer so the user 
// can choose between vol and simplex at runtime.

OsiSolverInterface *
CSP_lp::initialize_solver_interface()
{
  OsiSolverInterface* solver;
  if (par.entry(CSP_lp_par::LpSolver) == CSP_UseSimplex) {
    solver = new OsiClpSolverInterface;
    solver->messageHandler()->setLogLevel(0);
  } else { // CSP_UseVolume
    solver = new OsiVolSolverInterface;
  }
  return solver;
}

//#############################################################################
// Opportunity to reset things before optimization
void
CSP_lp::modify_lp_parameters(OsiSolverInterface* lp, const int changeType,
			     bool in_strong_branching)
{
  // char name[1000];
  // sprintf(name, "xxx-%i-%i", current_index(), current_iteration());
  // lp->writeMps(name);
  OsiVolSolverInterface* vollp = dynamic_cast<OsiVolSolverInterface*>(lp);
  if (vollp) {
    VOL_parms& vpar = vollp->volprob()->parm;
    vpar.lambdainit = par.entry(CSP_lp_par::Vol_lambdaInit); 
    vpar.alphainit =  par.entry(CSP_lp_par::Vol_alphaInit);
    vpar.alphamin = par.entry(CSP_lp_par::Vol_alphaMin);
    vpar.alphafactor = par.entry(CSP_lp_par::Vol_alphaFactor);
    vpar.primal_abs_precision = par.entry(CSP_lp_par::Vol_primalAbsPrecision);
    vpar.gap_abs_precision = par.entry(CSP_lp_par::Vol_gapAbsPrecision);
    vpar.gap_rel_precision = par.entry(CSP_lp_par::Vol_gapRelPrecision);
    vpar.granularity = par.entry(CSP_lp_par::Vol_granularity);
    vpar.minimum_rel_ascent = par.entry(CSP_lp_par::Vol_minimumRelAscent);
    vpar.ascent_first_check = par.entry(CSP_lp_par::Vol_ascentFirstCheck);
    vpar.ascent_check_invl = par.entry(CSP_lp_par::Vol_ascentCheckInterval);
    vpar.maxsgriters = par.entry(CSP_lp_par::Vol_maxSubGradientIterations);
    vpar.printflag =  par.entry(CSP_lp_par::Vol_printFlag);
    vpar.printinvl =  par.entry(CSP_lp_par::Vol_printInterval);
    vpar.greentestinvl = par.entry(CSP_lp_par::Vol_greenTestInterval);
    vpar.yellowtestinvl =  par.entry(CSP_lp_par::Vol_yellowTestInterval);
    vpar.redtestinvl = par.entry(CSP_lp_par::Vol_redTestInterval);
    vpar.alphaint =  par.entry(CSP_lp_par::Vol_alphaInt);
  }
}

//#############################################################################

void
CSP_lp::initialize_new_search_tree_node(const BCP_vec<BCP_var*>& vars,
					const BCP_vec<BCP_cut*>& cuts,
					const BCP_vec<BCP_obj_status>& vstat,
					const BCP_vec<BCP_obj_status>& cstat,
					BCP_vec<int>& var_changed_pos,
					BCP_vec<double>& var_new_bd,
					BCP_vec<int>& cut_changed_pos,
					BCP_vec<double>& cut_new_bd)
{
  // Here is where we add the excluded pattern info to the searchtree node
  // problem

  int i;
  const int varnum = vars.size();
  const int size = csproblem->getM();
  double * fullVec = new double[size];

  // scan through the vars to pick up branching variables
  // this was specific to auction problem.
  std::vector<const PATTERN *> excl_pat;
  printf("CSP: Excluded patterns:\n");
  for (i=varnum-1; i>=0; --i){
    if (vars[i]->is_non_removable()){
      // collect them (vec of ptrs to vars)
      const PATTERN* pat = dynamic_cast<const PATTERN*>(vars[i]);
      excl_pat.push_back(pat);
      const CSP_packedVector&  pv = pat->getWidths();
      printf("%.0f,%.0f : ", vars[i]->lb(), vars[i]->ub());
      CoinFillN(fullVec, size, 0.0);
      for (int j=0; j<pv.getSize(); ++j){
	fullVec[pv.getIndices()[j]]=pv.getElements()[j];
      }
      for (int j=0; j<size; ++j){
	printf("%4.0f", fullVec[j]);
      }
      printf("\n");
    }
  }
  colgen->applyExclusions(excl_pat);
  delete[] fullVec;
}

//#############################################################################

double
CSP_lp::compute_lower_bound(const double old_lower_bound,
			    const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts)
{
  // this is for safety.
  // to update lower bounds we need the new columns
  // the new cols are saved in "improving_patterns"
  // here we add the new imp_patterns to the vec
  // and in gen_vars_in_lp, we take the imp_patterns
  // off this vector and use them.


  // It's not necessaryily true that gen_vars_in_lp will be called.
  // We might be able to fathom, etc.
  // So when we get the next search tree node, the info might still be there.
  // To be safe, we clean before we add new impr_cols.


  for_each(improving_patterns_.begin(), improving_patterns_.end(),
	   DELETEOBJECT());
  improving_patterns_.erase(improving_patterns_.begin(),
			    improving_patterns_.end());

  // if primal infeasbile, return -infinity
  if (lpres.termcode()==BCP_ProvenPrimalInf)
    return -DBL_MAX;

  const double* pi = lpres.pi();

  improving_patterns_ =
    colgen->generateColumns(pi, lpres.dualTolerance(),
			    true /* primal feasible */);

  // A stab at Farley's lower bound
  const int numimp = improving_patterns_.size();
  if (numimp == 0) {
    return lpres.objval();
  } else {
    return old_lower_bound;
    double solColGenSubProb = 0;
    for (int i = 1; i < numimp; ++i) {
      const PATTERN& impPat = *improving_patterns_[i];
      const CSP_packedVector& impPatWidth = impPat.getWidths();
      double sum = 0;
      const int size = impPatWidth.getSize();
      const int* ind = impPatWidth.getIndices();
      const double* elem = impPatWidth.getElements();
      for (int i=0; i<size; ++i) {
	sum += pi[ind[i]]*elem[i];
      }
      if (sum > solColGenSubProb)
	solColGenSubProb = sum;
    }
    // no need to take the ceiling since bcp takes care of the granularity
    // of the problem.
    const double candLowerBound = lpres.objval()/solColGenSubProb;
    return std::max(old_lower_bound, candLowerBound);
  }

  // fake return
  return old_lower_bound;
}

//#############################################################################

void
CSP_lp::generate_vars_in_lp(const BCP_lp_result& lpres,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    const bool before_fathom,
			    BCP_vec<BCP_var*>& new_vars,
			    BCP_vec<BCP_col*>& new_cols)
{
  if (!improving_patterns_.empty()) {
    const int numimp = improving_patterns_.size(); 
    for (int i = 0; i < numimp; ++i) {

      // when BCP gets these, it computes the rc just to be
      // sure they're improving.  And then they're added to the 
      // local_var_pool.  And if you decided not to branch,
      // then the best so many of these will be added to the 
      // formultion.
      // This is why in select_branching_candidates method
      // we check the local_var_pool.
      new_vars.push_back(new CSP_var(*improving_patterns_[i]));
    }
  }

  if (!new_vars.empty())
    vars_to_cols(new_vars, new_cols);
}

//#############################################################################
// we've only base constraints, so variable expansion is *really* easy

void
CSP_lp::vars_to_cols(const BCP_vec<BCP_cut*>& cuts, // what to expand on
		     BCP_vec<BCP_var*>& vars,       // what to expand on
		     BCP_vec<BCP_col*>& cols,       // the expanded cols
		     // few things that the user can use for lifting vars if
		     // allowed 
		     const BCP_lp_result& lpres,
		     BCP_object_origin origin, bool allow_multiple)
{
  vars_to_cols(vars, cols);
}

void
CSP_lp::vars_to_cols(BCP_vec<BCP_var*>& vars, // what to expand on
		     BCP_vec<BCP_col*>& cols) // the expanded cols
{
  const int varnum = vars.size();
  if (varnum == 0)
    return; 

  // allocate sufficient space so you never have to reallocate again
  // reserve is a method of BCP_vec
  cols.reserve(varnum);

  double objCoef = 1;
  BCP_col * col = 0;
  
  // I know the "vars" are really PATTERN ptrs
  for (int i = 0; i < varnum; ++i) {   
    const PATTERN* pat = dynamic_cast<const PATTERN*>(vars[i]);
    // if the dynamic cast didn't succeed then pat would be a null ptr
    //   if (pat != 0) {

    const CSP_packedVector & width = pat->getWidths();
    const int size = width.getSize();
    const int *ind = width.getIndices();
    const double *elem = width.getElements();

    // we must use this constructor.
    // it says don't take my storage over, just use it.
    // make sure LL documents or changes this in BCP
    col = new BCP_col(ind,ind+size, elem,elem+size, objCoef,
		      vars[i]->lb(), vars[i]->ub());

    cols.unchecked_push_back(col);
  }
}
//#############################################################################


BCP_solution*
CSP_lp::generate_heuristic_solution(const BCP_lp_result& lpres,
				    const BCP_vec<BCP_var*>& vars,
				    const BCP_vec<BCP_cut*>& cuts)
{
  const int numvars = vars.size();
  const double * lpColSol = lpres.x();
  double * colSol = new double[numvars]; // free?
  const double tol = 1e-6;

  BCP_solution_generic * roundSolution = new BCP_solution_generic;
  BCP_solution_generic * ipSolution = 0;

  // round up and down
  for (int i=0; i<numvars; ++i){
    colSol[i] = floor(lpColSol[i] + (1-tol));

    // if value is nonzero, add it to the roundSolution
    // values are integers stored as doubles, so this test ok 
    if (colSol[i]>0){
      // ask Laci: why do I have to cast? why can't I...
      // roundSolution->add_entry(vars[i], colSol[i]);

      CSP_var* cspVarPtr = dynamic_cast<CSP_var*> (vars[i]);
      CSP_var* myCspVar = new CSP_var(*cspVarPtr);
      roundSolution->add_entry(myCspVar, colSol[i]);
    }
  }
  delete[] colSol;

  const int iter = current_iteration() - 1;
  const int every_so_often = par.entry(CSP_lp_par::HeurIpFrequency);
  if ( (every_so_often > 0 && ((iter % every_so_often) == 0)) ||
       improving_patterns_.empty() ) {
    ipSolution = new BCP_solution_generic;


    int* integerCols = new int[numvars];
    CoinIotaN(integerCols, numvars, 0);
    const double* sol = 0;
    OsiSolverInterface* solver = getLpProblemPointer()->lp_solver;

#ifdef CSP_UseCbc
    OsiClpSolverInterface* clp = dynamic_cast<OsiClpSolverInterface*>(solver);
    const bool solver_is_clp = clp != 0;
    if (!solver_is_clp) {
      clp = new OsiClpSolverInterface();
      clp->loadProblem(*solver->getMatrixByCol(),
		       solver->getColLower(), solver->getColUpper(),
		       solver->getObjCoefficients(),
		       solver->getRowLower(), solver->getRowUpper());
      clp->messageHandler()->setLogLevel(0);
    }
    CbcModel cbc(*clp);
    cbc.messageHandler()->setLogLevel(1);
    cbc.solver()->setInteger(integerCols, numvars);
    CglGomory cg_gomory;
    CglKnapsackCover cg_knapsack;
    CglOddHole cg_oddhole;
    CglSimpleRounding cg_round;
    CglLiftAndProject cg_lift;
    CglProbing cg_probe;
    cbc.addCutGenerator(&cg_gomory, -1);
    cbc.addCutGenerator(&cg_knapsack, -1);
    cbc.addCutGenerator(&cg_oddhole, -1);
    //     cbc.addCutGenerator(&cg_round, -1);
    //     cbc.addCutGenerator(&cg_lift, -1);
    //     cbc.addCutGenerator(&cg_probe, -1);
    cbc.setMaximumNodes(par.entry(CSP_lp_par::HeurIpMaxTreeSize));
    cbc.branchAndBound();
    sol = cbc.bestSolution();
#endif

#ifdef CSP_UseCpx
    OsiCpxSolverInterface cpx;
    cpx.loadProblem(*solver->getMatrixByCol(),
		    solver->getColLower(), solver->getColUpper(),
		    solver->getObjCoefficients(),
		    solver->getRowLower(), solver->getRowUpper());
    cpx.setInteger(integerCols, numvars);
    cpx.branchAndBound();
    sol = cpx.getColSolution();
#endif

    if (sol) {
      for (int i = 0; i < numvars; ++i) {
	// the sol is already rounded
	if (sol[i] > 0) {
	  const CSP_var* cspVarPtr = dynamic_cast<const CSP_var*>(vars[i]);
	  CSP_var* myCspVar = new CSP_var(*cspVarPtr);
	  ipSolution->add_entry(myCspVar, sol[i]);
	}
      }
    } else {
      delete ipSolution;
      ipSolution = 0;
    }
#ifdef CSP_UseCbc
    if (!solver_is_clp) {
      delete clp;
    }
#endif
  }

  if (ipSolution) {
    if (roundSolution->objective_value() > ipSolution->objective_value()) {
      delete roundSolution;
      return ipSolution;
    }
  }

  return roundSolution;
}

//#############################################################################

void
CSP_lp::restore_feasibility(const BCP_lp_result& lpres,
			    const std::vector<double*> dual_rays,
			    const BCP_vec<BCP_var*>& vars,
			    const BCP_vec<BCP_cut*>& cuts,
			    BCP_vec<BCP_var*>& vars_to_add,
			    BCP_vec<BCP_col*>& cols_to_add)
{
  OsiClpSolverInterface* master_solver =
    dynamic_cast<OsiClpSolverInterface*>(getLpProblemPointer()->lp_solver);
  double dual_etol;
  master_solver->getDblParam(OsiDualTolerance, dual_etol);
  std::vector<PATTERN*> pats;

  OsiClpSolverInterface solver(*master_solver);
  const int numcols = vars.size();
  for (int i = 0; i < numcols; ++i) {
    solver.setObjCoeff(i, 0.0);
  }
  const int numrows = cuts.size();
  double value = 1.0;
  int* index = new int[numrows];
  double* obj = new double[numrows];
  double* clb = new double[numrows];
  double* cub = new double[numrows];
  CoinIotaN(index, numrows, 0);
  CoinFillN(obj, numrows, 1.0);
  CoinFillN(clb, numrows, 0.0);
  CoinFillN(cub, numrows, 1e31);

  const CoinPackedVectorBase** cols =
    new const CoinPackedVectorBase*[numrows];
  for (int i = 0; i < numrows; ++i) {
    cols[i] = new CoinShallowPackedVector(1, index + i, &value, false);
  }
  solver.addCols(numrows, cols, clb, cub, obj);
  for (int i = 0; i < numrows; ++i) {
    delete cols[i];
  }
  delete[] cols;

  solver.initialSolve();
  double* pi = new double[numrows];
  CoinDisjointCopyN(solver.getRowPrice(), numrows, pi);
  for (int i = numrows - 1; i >= 0; --i) {
    pi[i] = -pi[i];
  }
  pats =
    colgen->generateColumns(pi, dual_etol, false /* not primal feasible */);
  delete[] pi;

  delete[] obj;
  delete[] index;
  delete[] clb;
  delete[] cub;

  const int numpats = pats.size();
  for (int i = 0; i < numpats; ++i) {
    vars_to_add.push_back(new CSP_var(*pats[i]));
    delete pats[i];
  }

  if (!vars_to_add.empty())
    vars_to_cols(vars_to_add, cols_to_add);
}
