// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "MC_lp_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<MC_lp_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("MC_DoEdgeSwitchHeur"),
			    BCP_parameter(BCP_CharPar,
					  DoEdgeSwitchHeur)));
   keys.push_back(make_pair(BCP_string("MC_ReportAllSPCycleCuts"),
			    BCP_parameter(BCP_CharPar,
					  ReportAllSPCycleCuts)));
   keys.push_back(make_pair(BCP_string("MC_ExplicitSlacksInOpt"),
			    BCP_parameter(BCP_CharPar,
					  ExplicitSlacksInOpt)));
   keys.push_back(make_pair(BCP_string("MC_OnceOptAlwaysOpt"),
			    BCP_parameter(BCP_CharPar,
					  OnceOptAlwaysOpt)));
   keys.push_back(make_pair(BCP_string("MC_SwitchToSimplex"),
 			    BCP_parameter(BCP_CharPar,
 					  SwitchToSimplex)));
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));

   //--------------------------------------------------------------------------
   // IntPar
   keys.push_back(make_pair(BCP_string("MC_LpSolver"),
			    BCP_parameter(BCP_IntPar,
					  LpSolver)));
   keys.push_back(make_pair(BCP_string("MC_MaxDepth"),
			    BCP_parameter(BCP_IntPar,
					  MaxDepth)));
   keys.push_back(make_pair(BCP_string("MC_MstCycleCutGeneration"),
			    BCP_parameter(BCP_IntPar,
					  MstCycleCutGeneration)));
   keys.push_back(make_pair(BCP_string("MC_SPCycleCutGeneration"),
			    BCP_parameter(BCP_IntPar,
					  SPCycleCutGeneration)));
   keys.push_back(make_pair(BCP_string("MC_StructureSwitchHeur"),
			    BCP_parameter(BCP_IntPar,
					  StructureSwitchHeur)));
   keys.push_back(make_pair(BCP_string("MC_MstHeurNum"),
			    BCP_parameter(BCP_IntPar,
					  MstHeurNum)));
   keys.push_back(make_pair(BCP_string("MC_CycleCutHeurNum"),
			    BCP_parameter(BCP_IntPar,
					  CycleCutHeurNum)));
   keys.push_back(make_pair(BCP_string("MC_MaxCycleCutNum"),
			    BCP_parameter(BCP_IntPar,
					  MaxCycleCutNum)));
   keys.push_back(make_pair(BCP_string("MC_SB_CandidateNum"),
			    BCP_parameter(BCP_IntPar,
					  SB_CandidateNum)));
   keys.push_back(make_pair(BCP_string("MC_HeurSwitchImproveRound"),
			    BCP_parameter(BCP_IntPar,
					  HeurSwitchImproveRound)));
   keys.push_back(make_pair(BCP_string("MC_TailoffGapRelMinItcount"),
			    BCP_parameter(BCP_IntPar,
					  TailoffGapRelMinItcount)));
   keys.push_back(make_pair(BCP_string("MC_TailoffLbAbsMinItcount"),
			    BCP_parameter(BCP_IntPar,
					  TailoffLbAbsMinItcount)));
   keys.push_back(make_pair(BCP_string("MC_TailoffLbRelMinItcount"),
			    BCP_parameter(BCP_IntPar,
					  TailoffLbRelMinItcount)));
   keys.push_back(make_pair(BCP_string("MC_MaxCutsAddedPerIterVol"),
			    BCP_parameter(BCP_IntPar,
					  MaxCutsAddedPerIterVol)));
   keys.push_back(make_pair(BCP_string("MC_MaxCutsAddedPerIterSim"),
			    BCP_parameter(BCP_IntPar,
					  MaxCutsAddedPerIterSim)));
   keys.push_back(make_pair(BCP_string("MC_MaxPresolveIterVol"),
			    BCP_parameter(BCP_IntPar,
					  MaxPresolveIterVol)));
   keys.push_back(make_pair(BCP_string("MC_MaxPresolveIterSim"),
			    BCP_parameter(BCP_IntPar,
					  MaxPresolveIterSim)));
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
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));
   //--------------------------------------------------------------------------
   // DoublePar
   keys.push_back(make_pair(BCP_string("MC_IntegerTolerance"),
			    BCP_parameter(BCP_DoublePar,
					  IntegerTolerance)));
   keys.push_back(make_pair(BCP_string("MC_MinIsingCutViolation"),
			    BCP_parameter(BCP_DoublePar,
					  MinIsingCutViolation)));
   keys.push_back(make_pair(BCP_string("MC_MinMstCycleCutViolation"),
			    BCP_parameter(BCP_DoublePar,
					  MinMstCycleCutViolation)));
   keys.push_back(make_pair(BCP_string("MC_MinSPCycleCutViolation"),
			    BCP_parameter(BCP_DoublePar,
					  MinSPCycleCutViolation)));
   keys.push_back(make_pair(BCP_string("MC_MaxPerturbInMstCycleCutGen"),
			    BCP_parameter(BCP_DoublePar,
					  MaxPerturbInMstCycleCutGen)));
   keys.push_back(make_pair(BCP_string("MC_MaxPerturbInMstHeur"),
			    BCP_parameter(BCP_DoublePar,
					  MaxPerturbInMstHeur)));
   keys.push_back(make_pair(BCP_string("MC_TailoffGapRelMinImprovement"),
			    BCP_parameter(BCP_DoublePar,
					  TailoffGapRelMinImprovement)));
   keys.push_back(make_pair(BCP_string("MC_TailoffLbAbsMinImprovement"),
			    BCP_parameter(BCP_DoublePar,
					  TailoffLbAbsMinImprovement)));
   keys.push_back(make_pair(BCP_string("MC_TailoffLbRelMinImprovement"),
			    BCP_parameter(BCP_DoublePar,
					  TailoffLbRelMinImprovement)));
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
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   //--------------------------------------------------------------------------
   // StringArrayPar
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_StringArrayPar,
// 					  )));
   //--------------------------------------------------------------------------
   // BooleanListPar
//    keys.push_back(make_pair(BCP_string("MC_LpVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_LpVerb_PrunedNodeInfo)));
}

//#############################################################################

template <>
void BCP_parameter_set<MC_lp_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(DoEdgeSwitchHeur, true);
   set_entry(ReportAllSPCycleCuts, true);
   set_entry(ExplicitSlacksInOpt, false);
   set_entry(OnceOptAlwaysOpt, false);
   set_entry(SwitchToSimplex, true);
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(LpSolver, MC_UseVol | MC_UseClp);
   set_entry(MaxDepth, 10000000);
   set_entry(MstCycleCutGeneration, MC_AlwaysGenerateMstCycleCuts);
   set_entry(SPCycleCutGeneration, MC_AlwaysGenerateSPCycleCuts);
   set_entry(StructureSwitchHeur, INT_MAX);
   set_entry(MstHeurNum, 3);
   set_entry(CycleCutHeurNum, 3);
   set_entry(MaxCycleCutNum, 1000);
   set_entry(SB_CandidateNum, 1);
   set_entry(HeurSwitchImproveRound, 10);
   set_entry(TailoffGapRelMinItcount, -1); // no this kind of tailoff
   set_entry(TailoffLbAbsMinItcount, -1);  // no this kind of tailoff
   set_entry(TailoffLbRelMinItcount, -1);  // no this kind of tailoff

   set_entry(MaxCutsAddedPerIterVol, 1500);
   set_entry(MaxCutsAddedPerIterSim, 1000);
   set_entry(MaxPresolveIterVol, -1);
   set_entry(MaxPresolveIterSim, 0);

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
   set_entry(IntegerTolerance, .001);
   set_entry(MinIsingCutViolation, .02);
   set_entry(MinMstCycleCutViolation, .02);
   set_entry(MinSPCycleCutViolation, .02);
   set_entry(MaxPerturbInMstCycleCutGen, .03);
   set_entry(MaxPerturbInMstHeur, .1);
   set_entry(TailoffGapRelMinImprovement, .02);
   set_entry(TailoffLbAbsMinImprovement, .02);
   set_entry(TailoffLbRelMinImprovement, .02);

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
   //--------------------------------------------------------------------------
   // StringArrayPar
   //--------------------------------------------------------------------------
   // BooleanListPar
}
