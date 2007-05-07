// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include "MKC_lp_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<MKC_lp_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("MKC_CheckForTailoff"),
			    BCP_parameter(BCP_CharPar,
					  CheckForTailoff)));
   keys.push_back(make_pair(BCP_string("MKC_ComputeLpLowerBound"),
			    BCP_parameter(BCP_CharPar,
					  ComputeLpLowerBound)));
   keys.push_back(make_pair(BCP_string("MKC_DoLogicalFixing"),
			    BCP_parameter(BCP_CharPar,
					  DoLogicalFixing)));
   keys.push_back(make_pair(BCP_string("MKC_HeurFixByReducedCost"),
			    BCP_parameter(BCP_CharPar,
					  HeurFixByReducedCost)));
   keys.push_back(make_pair(BCP_string("MKC_AddAllGeneratedVars"),
			    BCP_parameter(BCP_CharPar,
					  AddAllGeneratedVars)));
   keys.push_back(make_pair(BCP_string("MKC_ExactFallbackAtVargen"),
			    BCP_parameter(BCP_CharPar,
					  ExactFallbackAtVargen)));
   keys.push_back(make_pair(BCP_string("MKC_ExactFallbackAtLowerBd"),
			    BCP_parameter(BCP_CharPar,
					  ExactFallbackAtLowerBd)));
   keys.push_back(make_pair(BCP_string("MKC_ParallelVargenForAllKS"),
			    BCP_parameter(BCP_CharPar,
					  ParallelVargenForAllKS)));
   keys.push_back(make_pair(BCP_string("MKC_PrintBestDj"),
			    BCP_parameter(BCP_CharPar,
					  PrintBestDj)));
   keys.push_back(make_pair(BCP_string("MKC_WriteToVarFile"),
			    BCP_parameter(BCP_CharPar,
					  WriteToVarFile)));
   keys.push_back(make_pair(BCP_string("MC_UseVolume"),
			    BCP_parameter(BCP_CharPar,
					  UseVolume)));
   keys.push_back(make_pair(BCP_string("MC_UseClp"),
			    BCP_parameter(BCP_CharPar,
					  UseClp)));
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));

   //--------------------------------------------------------------------------
   // IntPar
   keys.push_back(make_pair(BCP_string("MKC_LpSolverMessageSuppression"),
			    BCP_parameter(BCP_IntPar,
					  LpSolverMessageSuppression)));
   keys.push_back(make_pair(BCP_string("MKC_MaxEnumeratedSize"),
			    BCP_parameter(BCP_IntPar,
					  MaxEnumeratedSize)));
   keys.push_back(make_pair(BCP_string("MKC_TailoffLength"),
			    BCP_parameter(BCP_IntPar,
					  TailoffLength)));
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
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));
   //--------------------------------------------------------------------------
   // DoublePar
   keys.push_back(make_pair(BCP_string("MKC_LowerBound"),
			    BCP_parameter(BCP_DoublePar,
					  LowerBound)));
   keys.push_back(make_pair(BCP_string("MKC_TailoffIncrease"),
			    BCP_parameter(BCP_DoublePar,
					  TailoffIncrease)));
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
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
   keys.push_back(make_pair(BCP_string("MKC_OutputVarFile"),
			    BCP_parameter(BCP_StringPar,
					  OutputVarFile)));
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   
   //--------------------------------------------------------------------------
   // BooleanListPar
//    keys.push_back(make_pair(BCP_string("MKC_LpVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_LpVerb_PrunedNodeInfo)));
}

//#############################################################################

template <>
void BCP_parameter_set<MKC_lp_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(CheckForTailoff, true);
   set_entry(ComputeLpLowerBound, false);
   set_entry(DoLogicalFixing, true);
   set_entry(HeurFixByReducedCost, true);

   set_entry(AddAllGeneratedVars, true);
   set_entry(ExactFallbackAtVargen, true);
   set_entry(ExactFallbackAtLowerBd, true);
   set_entry(ParallelVargenForAllKS, true);
   set_entry(PrintBestDj, true);

   set_entry(WriteToVarFile, false);

   set_entry(UseVolume, true);
   set_entry(UseClp, false);
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(LpSolverMessageSuppression, 3);
   set_entry(MaxEnumeratedSize, 20);
   set_entry(TailoffLength, 5);

   set_entry(Vol_ascentFirstCheck, 500);
   set_entry(Vol_ascentCheckInterval, 100);
   set_entry(Vol_printFlag, 3);
   set_entry(Vol_printInterval, 50);
   set_entry(Vol_greenTestInterval, 1);
   set_entry(Vol_yellowTestInterval, 400);
   set_entry(Vol_redTestInterval, 10);
   set_entry(Vol_alphaInt, 50);
   set_entry(Vol_maxSubGradientIterations, 2000);
   //--------------------------------------------------------------------------
   // DoublePar
   set_entry(LowerBound, -DBL_MAX);
   set_entry(TailoffIncrease, 0.002);

   set_entry(Vol_lambdaInit, 0.1);
   set_entry(Vol_alphaInit, 0.1);
   set_entry(Vol_alphaFactor, 0.5);
   set_entry(Vol_alphaMin, 0.0001);
   set_entry(Vol_primalAbsPrecision, 0.02);
   set_entry(Vol_gapAbsPrecision, 1.0);
   set_entry(Vol_gapRelPrecision, 0.01);
   set_entry(Vol_granularity, .999);
   set_entry(Vol_minimumRelAscent, 0.0001);
   //--------------------------------------------------------------------------
   // StringPar
   set_entry(OutputVarFile, "mkc7.var");
   //--------------------------------------------------------------------------
   // BooleanListPar
}
