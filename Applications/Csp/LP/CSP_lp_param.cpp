// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include "CSP_lp_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<CSP_lp_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));
   //--------------------------------------------------------------------------
   // IntPar
   keys.push_back(make_pair(BCP_string("CSP_HeurIpFrequency"),
			    BCP_parameter(BCP_IntPar,
					  HeurIpFrequency)));
   keys.push_back(make_pair(BCP_string("CSP_HeurIpMaxTreeSize"),
			    BCP_parameter(BCP_IntPar,
					  HeurIpMaxTreeSize)));
   keys.push_back(make_pair(BCP_string("CSP_LpSolver"),
			    BCP_parameter(BCP_IntPar,
					  LpSolver)));
   keys.push_back(make_pair(BCP_string("CSP_LpSolverMessageSuppression"),
			    BCP_parameter(BCP_IntPar,
					  LpSolverMessageSuppression)));
   keys.push_back(make_pair(BCP_string("CSP_BranchingStrategy"),
			    BCP_parameter(BCP_IntPar,
					  BranchingStrategy)));
   keys.push_back(make_pair(BCP_string("CSP_PerturbNum"),
			    BCP_parameter(BCP_IntPar,
					  PerturbNum)));
   //--------------------------------------------------------------------------
   keys.push_back(make_pair(BCP_string("Vol_ascentFirstCheck"),
			    BCP_parameter(BCP_IntPar,
					  Vol_ascentFirstCheck)));
   keys.push_back(make_pair(BCP_string("Vol_ascentCheckInterval"),
			    BCP_parameter(BCP_IntPar,
					  Vol_ascentCheckInterval)));
   keys.push_back(make_pair(BCP_string("Vol_printFlag"),
			    BCP_parameter(BCP_IntPar,
					  Vol_printFlag)));
   keys.push_back(make_pair(BCP_string("Vol_printInterval"),
			    BCP_parameter(BCP_IntPar,
					  Vol_printInterval)));
   keys.push_back(make_pair(BCP_string("Vol_greenTestInterval"),
			    BCP_parameter(BCP_IntPar,
					  Vol_greenTestInterval)));
   keys.push_back(make_pair(BCP_string("Vol_yellowTestInterval"),
			    BCP_parameter(BCP_IntPar,
					  Vol_yellowTestInterval)));
   keys.push_back(make_pair(BCP_string("Vol_redTestInterval"),
			    BCP_parameter(BCP_IntPar,
					  Vol_redTestInterval)));
   keys.push_back(make_pair(BCP_string("Vol_alphaInt"),
			    BCP_parameter(BCP_IntPar,
					  Vol_alphaInt)));
   keys.push_back(make_pair(BCP_string("Vol_maxSubGradientIterations"),
			    BCP_parameter(BCP_IntPar,
					  Vol_maxSubGradientIterations)));
   //--------------------------------------------------------------------------
   // DblPar
   keys.push_back(make_pair(BCP_string("CSP_PerturbFactor"),
			    BCP_parameter(BCP_DoublePar,
					  PerturbFactor)));
   //--------------------------------------------------------------------------
   keys.push_back(make_pair(BCP_string("Vol_lambdaInit"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_lambdaInit)));
   keys.push_back(make_pair(BCP_string("Vol_alphaInit"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_alphaInit)));
   keys.push_back(make_pair(BCP_string("Vol_alphaFactor"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_alphaFactor)));
   keys.push_back(make_pair(BCP_string("Vol_alphaMin"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_alphaMin)));
   keys.push_back(make_pair(BCP_string("Vol_primalAbsPrecision"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_primalAbsPrecision)));
   keys.push_back(make_pair(BCP_string("Vol_gapAbsPrecision"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_gapAbsPrecision)));
   keys.push_back(make_pair(BCP_string("Vol_gapRelPrecision"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_gapRelPrecision)));
   keys.push_back(make_pair(BCP_string("Vol_granularity"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_granularity)));
   keys.push_back(make_pair(BCP_string("Vol_minimumRelAscent"),
			    BCP_parameter(BCP_DoublePar,
					  Vol_minimumRelAscent)));
   //--------------------------------------------------------------------------
   // StringPar
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   
   //--------------------------------------------------------------------------
   // BooleanListPar
//    keys.push_back(make_pair(BCP_string("CSP_LpVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_LpVerb_PrunedNodeInfo)));
}

//#############################################################################

template <>
void BCP_parameter_set<CSP_lp_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(HeurIpFrequency, 0);
   set_entry(HeurIpMaxTreeSize, 500);
   set_entry(LpSolver, CSP_UseSimplex);
   set_entry(LpSolverMessageSuppression, 3); // ?
   set_entry(BranchingStrategy,0); // we've only devised one
   set_entry(PerturbNum, 30);

   set_entry(Vol_ascentFirstCheck, 500000);
   set_entry(Vol_ascentCheckInterval, 100);
   set_entry(Vol_printFlag, 1);
   set_entry(Vol_printInterval, 3000);
   set_entry(Vol_greenTestInterval, 1);
   set_entry(Vol_yellowTestInterval, 2);
   set_entry(Vol_redTestInterval, 10);
   set_entry(Vol_alphaInt, 50);
   set_entry(Vol_maxSubGradientIterations, 2000);
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(PerturbFactor, .97);

   set_entry(Vol_lambdaInit, 0.1);
   set_entry(Vol_alphaInit, 0.01);
   set_entry(Vol_alphaFactor, 0.5);
   set_entry(Vol_alphaMin, 0.00001);
   set_entry(Vol_primalAbsPrecision, 0.01);
   set_entry(Vol_gapAbsPrecision, 1.0);
   set_entry(Vol_gapRelPrecision, 0.01);
   set_entry(Vol_granularity, 0.0);
   set_entry(Vol_minimumRelAscent, 0.0001);
   //--------------------------------------------------------------------------
   // StringPar
   //--------------------------------------------------------------------------
   // BooleanListPar
}
