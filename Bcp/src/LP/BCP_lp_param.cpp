// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_lp_param.hpp"
#include "BCP_parameters.hpp"

#include "BCP_enum.hpp"
#include "BCP_enum_branch.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<BCP_lp_par>::create_keyword_list() {

    obsolete_keys.push_back(BCP_string("BCP_FixVarsBeforeFathom"));

    // Create the list of keywords for parameter file reading
    //-------------------------------------------------------------------------
    // CharPar
    keys.push_back(make_pair(BCP_string("BCP_BranchOnCuts"),
			     BCP_parameter(BCP_CharPar, 
					   BranchOnCuts)));
    keys.push_back(make_pair(BCP_string("BCP_CompareNewCutsToOldOnes"),
			     BCP_parameter(BCP_CharPar, 
					   CompareNewCutsToOldOnes)));
    keys.push_back(make_pair(BCP_string("BCP_CompareNewVarsToOldOnes"),
			     BCP_parameter(BCP_CharPar, 
					   CompareNewVarsToOldOnes)));
    keys.push_back(make_pair(BCP_string("BCP_DoReducedCostFixingAtZero"),
			     BCP_parameter(BCP_CharPar, 
					   DoReducedCostFixingAtZero)));
    keys.push_back(make_pair(BCP_string("BCP_DoReducedCostFixingAtAnything"),
			     BCP_parameter(BCP_CharPar, 
					   DoReducedCostFixingAtAnything)));
    keys.push_back(make_pair(BCP_string("BCP_DoReducedCostFixing"),
			     BCP_parameter(BCP_CharPar, 
					   DoReducedCostFixingAtAnything)));
    keys.push_back(make_pair(BCP_string("BCP_MaintainIndexedVarPricingList"),
			     BCP_parameter(BCP_CharPar, 
					   MaintainIndexedVarPricingList)));
    keys.push_back(make_pair(BCP_string("BCP_ReportWhenDefaultIsExecuted"),
			     BCP_parameter(BCP_CharPar,
					   ReportWhenDefaultIsExecuted)));
    keys.push_back(make_pair(BCP_string("BCP_NoCompressionAtFathom"),
			     BCP_parameter(BCP_CharPar, 
					   NoCompressionAtFathom)));
    keys.push_back(make_pair(BCP_string("BCP_SendFathomedNodeDesc"),
			     BCP_parameter(BCP_CharPar, 
					   SendFathomedNodeDesc)));
    //-------------------------------------------------------------------------
    // BoolArrayPar
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_AddedCutCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_AddedCutCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_AddedVarCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_AddedVarCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ChildrenInfo"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ChildrenInfo)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ColumnGenerationInfo"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ColumnGenerationInfo)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_CutsToCutPoolCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_CutsToCutPoolCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_VarsToVarPoolCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_VarsToVarPoolCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_FathomInfo"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_FathomInfo)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_IterationCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_IterationCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_RelaxedSolution"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_RelaxedSolution)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_FinalRelaxedSolution"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_FinalRelaxedSolution)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_LpMatrixSize"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_LpMatrixSize)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_LpSolutionValue"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_LpSolutionValue)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_MatrixCompression"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_MatrixCompression)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_NodeTime"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_NodeTime)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_PresolvePositions"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_PresolvePositions)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_PresolveResult"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_PresolveResult)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ProcessedNodeIndex"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ProcessedNodeIndex)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ReportCutGenTimeout"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ReportCutGenTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ReportVarGenTimeout"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ReportVarGenTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ReportLocalCutPoolSize"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ReportLocalCutPoolSize)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_ReportLocalVarPoolSize"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_ReportLocalVarPoolSize)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_RepricingResult"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_RepricingResult)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_RowEffectivenessCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_RowEffectivenessCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_VarTightening"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_VarTightening)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_StrongBranchPositions"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_StrongBranchPositions)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_StrongBranchResult"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_StrongBranchResult)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_GeneratedCutCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_GeneratedCutCount)));
    keys.push_back(make_pair(BCP_string("BCP_LpVerb_GeneratedVarCount"),
			     BCP_parameter(BCP_CharPar,
					   LpVerb_GeneratedVarCount)));

    //    keys.push_back(make_pair(BCP_string("BCP_"),
    // 			      BCP_parameter(BCP_CharPar, 
    // 					    )));

    //-------------------------------------------------------------------------
    // IntPar
    keys.push_back(make_pair(BCP_string("BCP_NiceLevel"),
			     BCP_parameter(BCP_IntPar, 
					   NiceLevel)));
    keys.push_back(make_pair(BCP_string("BCP_ScaleMatrix"),
			     BCP_parameter(BCP_IntPar, 
					   ScaleMatrix)));
    keys.push_back(make_pair(BCP_string("BCP_IndexedToPriceStorageSize"),
			     BCP_parameter(BCP_IntPar, 
					   IndexedToPriceStorageSize)));

    keys.push_back(make_pair(BCP_string("BCP_SlackCutDiscardingStrategy"),
			     BCP_parameter(BCP_IntPar, 
					   SlackCutDiscardingStrategy)));
    keys.push_back(make_pair(BCP_string("BCP_CutEffectiveCountBeforePool"),
			     BCP_parameter(BCP_IntPar, 
					   CutEffectiveCountBeforePool)));
    keys.push_back(make_pair(BCP_string("BCP_CutPoolCheckFrequency"),
			     BCP_parameter(BCP_IntPar, 
					   CutPoolCheckFrequency)));
    keys.push_back(make_pair(BCP_string("BCP_VarPoolCheckFrequency"),
			     BCP_parameter(BCP_IntPar, 
					   VarPoolCheckFrequency)));
    keys.push_back(make_pair(BCP_string("BCP_IneffectiveConstraints"),
			     BCP_parameter(BCP_IntPar, 
					   IneffectiveConstraints)));
    keys.push_back(make_pair(BCP_string("BCP_IneffectiveBeforeDelete"),
			     BCP_parameter(BCP_IntPar, 
					   IneffectiveBeforeDelete)));

    keys.push_back(make_pair(BCP_string("BCP_MaxNonDualFeasToAdd_Min"),
			     BCP_parameter(BCP_IntPar, 
					   MaxNonDualFeasToAdd_Min)));
    keys.push_back(make_pair(BCP_string("BCP_MaxNonDualFeasToAdd_Max"),
			     BCP_parameter(BCP_IntPar, 
					   MaxNonDualFeasToAdd_Max)));
    keys.push_back(make_pair(BCP_string("BCP_MaxIndexedToPriceToAdd_Min"),
			     BCP_parameter(BCP_IntPar, 
					   MaxIndexedToPriceToAdd_Min)));
    keys.push_back(make_pair(BCP_string("BCP_MaxIndexedToPriceToAdd_Max"),
			     BCP_parameter(BCP_IntPar, 
					   MaxIndexedToPriceToAdd_Max)));

    keys.push_back(make_pair(BCP_string("BCP_CutViolationNorm"),
			     BCP_parameter(BCP_IntPar,
					   CutViolationNorm)));

    keys.push_back(make_pair(BCP_string("BCP_MaxCutsAddedPerIteration"),
			     BCP_parameter(BCP_IntPar, 
					   MaxCutsAddedPerIteration)));
    keys.push_back(make_pair(BCP_string("BCP_MaxVarsAddedPerIteration"),
			     BCP_parameter(BCP_IntPar, 
					   MaxVarsAddedPerIteration)));

    keys.push_back(make_pair(BCP_string("BCP_MaxLeftoverCutNum"),
			     BCP_parameter(BCP_IntPar, 
					   MaxLeftoverCutNum)));

    keys.push_back(make_pair(BCP_string("BCP_DeletedColToCompress_Min"),
			     BCP_parameter(BCP_IntPar, 
					   DeletedColToCompress_Min)));
    keys.push_back(make_pair(BCP_string("BCP_DeletedRowToCompress_Min"),
			     BCP_parameter(BCP_IntPar, 
					   DeletedRowToCompress_Min)));

    keys.push_back(make_pair(BCP_string("BCP_MaxPresolveIter"),
			     BCP_parameter(BCP_IntPar, 
					   MaxPresolveIter)));
    keys.push_back(make_pair(BCP_string("BCP_StrongBranch_CloseToHalfNum"),
			     BCP_parameter(BCP_IntPar, 
					   StrongBranch_CloseToHalfNum)));
    keys.push_back(make_pair(BCP_string("BCP_StrongBranch_CloseToOneNum"),
			     BCP_parameter(BCP_IntPar, 
					   StrongBranch_CloseToOneNum)));
    keys.push_back(make_pair(BCP_string("BCP_BranchingObjectComparison"),
			     BCP_parameter(BCP_IntPar, 
					   BranchingObjectComparison)));
    keys.push_back(make_pair(BCP_string("BCP_ChildPreference"),
			     BCP_parameter(BCP_IntPar, 
					   ChildPreference)));

    keys.push_back(make_pair(BCP_string("BCP_FeasibilityTest"),
			     BCP_parameter(BCP_IntPar, 
					   FeasibilityTest)));
    keys.push_back(make_pair(BCP_string("BCP_InfoForCG"),
			     BCP_parameter(BCP_IntPar, 
					   InfoForCG)));
    keys.push_back(make_pair(BCP_string("BCP_InfoForVG"),
			     BCP_parameter(BCP_IntPar, 
					   InfoForVG)));
    //    keys.push_back(make_pair(BCP_string("BCP_"),
    // 			      BCP_parameter(BCP_IntPar, 
    // 					    )));
    //--------------------------------------------------------------------------
    // DoublePar
    keys.push_back(make_pair(BCP_string("BCP_Granularity"),
			     BCP_parameter(BCP_DoublePar, 
					   Granularity)));
    keys.push_back(make_pair(BCP_string("BCP_DeletedColToCompress_Frac"),
			     BCP_parameter(BCP_DoublePar, 
					   DeletedColToCompress_Frac)));
    keys.push_back(make_pair(BCP_string("BCP_DeletedRowToCompress_Frac"),
			     BCP_parameter(BCP_DoublePar, 
					   DeletedRowToCompress_Frac)));
    keys.push_back(make_pair(BCP_string("BCP_MaxNonDualFeasToAdd_Frac"),
			     BCP_parameter(BCP_DoublePar, 
					   MaxNonDualFeasToAdd_Frac)));
    keys.push_back(make_pair(BCP_string("BCP_MaxIndexedToPriceToAdd_Frac"),
			     BCP_parameter(BCP_DoublePar, 
					   MaxIndexedToPriceToAdd_Frac)));
    keys.push_back(make_pair(BCP_string("BCP_MaxLeftoverCutFrac"),
			     BCP_parameter(BCP_DoublePar, 
					   MaxLeftoverCutFrac)));
    keys.push_back(make_pair(BCP_string("BCP_IntegerTolerance"),
			     BCP_parameter(BCP_DoublePar, 
					   IntegerTolerance)));

    keys.push_back(make_pair(BCP_string("BCP_FirstLP_FirstCutTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   FirstLP_FirstCutTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LaterLP_FirstCutTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   LaterLP_FirstCutTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_FirstLP_AllCutsTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   FirstLP_AllCutsTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LaterLP_AllCutsTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   LaterLP_AllCutsTimeout)));

    keys.push_back(make_pair(BCP_string("BCP_FirstLP_FirstVarTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   FirstLP_FirstVarTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LaterLP_FirstVarTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   LaterLP_FirstVarTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_FirstLP_AllVarsTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   FirstLP_AllVarsTimeout)));
    keys.push_back(make_pair(BCP_string("BCP_LaterLP_AllVarsTimeout"),
			     BCP_parameter(BCP_DoublePar, 
					   LaterLP_AllVarsTimeout)));
    //    keys.push_back(make_pair(BCP_string("BCP_"),
    // 			      BCP_parameter(BCP_DoublePar, 
    // 					    )));

    //-------------------------------------------------------------------------
    // StringPar
    keys.push_back(make_pair(BCP_string("BCP_LogFileName"),
			     BCP_parameter(BCP_StringPar, 
					   LogFileName)));
    //    keys.push_back(make_pair(BCP_string("BCP_"),
    // 			    BCP_parameter(BCP_StringPar, 
    // 					  )));
}

//#############################################################################

template <>
void BCP_parameter_set<BCP_lp_par>::set_default_entries() {
    //-------------------------------------------------------------------------
    // CharPar
    set_entry(BranchOnCuts, false);
    set_entry(CompareNewCutsToOldOnes, true);
    set_entry(CompareNewVarsToOldOnes, true);
    set_entry(DoReducedCostFixingAtZero, true);
    set_entry(DoReducedCostFixingAtAnything, true);
    set_entry(MaintainIndexedVarPricingList, true);
    set_entry(MessagePassingIsSerial, false);
    set_entry(ReportWhenDefaultIsExecuted, true);
    set_entry(NoCompressionAtFathom, false);
    set_entry(SendFathomedNodeDesc, true);
    //-------------------------------------------------------------------------
    set_entry(LpVerb_AddedCutCount, true);
    set_entry(LpVerb_AddedVarCount, true);
    set_entry(LpVerb_ChildrenInfo, true);
    set_entry(LpVerb_ColumnGenerationInfo, true);
    set_entry(LpVerb_CutsToCutPoolCount, true);
    set_entry(LpVerb_VarsToVarPoolCount, true);
    set_entry(LpVerb_FathomInfo, true);
    set_entry(LpVerb_IterationCount, true);
    set_entry(LpVerb_RelaxedSolution, true);
    set_entry(LpVerb_FinalRelaxedSolution, true);
    set_entry(LpVerb_LpMatrixSize, true);
    set_entry(LpVerb_LpSolutionValue, true);
    set_entry(LpVerb_MatrixCompression, true);
    set_entry(LpVerb_NodeTime, false);
    set_entry(LpVerb_PresolvePositions, true);
    set_entry(LpVerb_PresolveResult, true);
    set_entry(LpVerb_ProcessedNodeIndex, true);
    set_entry(LpVerb_ReportCutGenTimeout, true);
    set_entry(LpVerb_ReportVarGenTimeout, true);
    set_entry(LpVerb_ReportLocalCutPoolSize, true);
    set_entry(LpVerb_ReportLocalVarPoolSize, true);
    set_entry(LpVerb_RepricingResult, true);
    set_entry(LpVerb_RowEffectivenessCount, true);
    set_entry(LpVerb_VarTightening, true);
    set_entry(LpVerb_StrongBranchPositions, true);
    set_entry(LpVerb_StrongBranchResult, true);
    set_entry(LpVerb_GeneratedCutCount, true);
    set_entry(LpVerb_GeneratedVarCount, true);
    //-------------------------------------------------------------------------
    // IntPar
    set_entry(NiceLevel, 0);

    set_entry(ScaleMatrix, 0);
    set_entry(IndexedToPriceStorageSize, 1000);

    set_entry(SlackCutDiscardingStrategy, BCP_DiscardSlackCutsAtNewIteration);
    set_entry(CutEffectiveCountBeforePool, 1000);
    set_entry(CutPoolCheckFrequency, 10);
    set_entry(VarPoolCheckFrequency, 10);
    set_entry(IneffectiveConstraints, BCP_IneffConstr_ZeroDualValue);
    set_entry(IneffectiveBeforeDelete, 1);

    set_entry(MaxNonDualFeasToAdd_Min, 5);
    set_entry(MaxNonDualFeasToAdd_Max, 200);
    set_entry(MaxIndexedToPriceToAdd_Min, 5);
    set_entry(MaxIndexedToPriceToAdd_Max, 200);

    set_entry(CutViolationNorm, BCP_CutViolationNorm_Plain);

    set_entry(MaxCutsAddedPerIteration, 100000);
    set_entry(MaxVarsAddedPerIteration, 100000);

    set_entry(MaxLeftoverCutNum, 10000000); 

    set_entry(DeletedColToCompress_Min, 10);
    set_entry(DeletedRowToCompress_Min, 10);

    set_entry(MaxPresolveIter, 100000);
    set_entry(StrongBranch_CloseToHalfNum, 3);
    set_entry(StrongBranch_CloseToOneNum, 3);
    set_entry(BranchingObjectComparison, BCP_HighestLowObjval);
    set_entry(ChildPreference, BCP_PreferChild_LowBound);

    set_entry(FeasibilityTest, BCP_FullTest_Feasible);
    set_entry(InfoForCG, BCP_PrimalSolution_Nonzeros);
    set_entry(InfoForVG, BCP_DualSolution_Full);
    //-------------------------------------------------------------------------
    // DoublePar
    set_entry(Granularity, 1e-8);
    set_entry(DeletedColToCompress_Frac, 0.0);
    set_entry(DeletedRowToCompress_Frac, 0.0);
    set_entry(MaxNonDualFeasToAdd_Frac, 0.05);
    set_entry(MaxIndexedToPriceToAdd_Frac, 0.05);
    set_entry(MaxLeftoverCutFrac, 1.0);
    set_entry(IntegerTolerance, 1e-5);

    set_entry(FirstLP_FirstCutTimeout, -1.0);
    set_entry(LaterLP_FirstCutTimeout, -1.0);
    set_entry(FirstLP_AllCutsTimeout, -1.0);
    set_entry(LaterLP_AllCutsTimeout, -1.0);

    set_entry(FirstLP_FirstVarTimeout, -1.0);
    set_entry(LaterLP_FirstVarTimeout, -1.0);
    set_entry(FirstLP_AllVarsTimeout, -1.0);
    set_entry(LaterLP_AllVarsTimeout, -1.0);

    //-------------------------------------------------------------------------
    // StringPar
    set_entry(LogFileName,"");
}
