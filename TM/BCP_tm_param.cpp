// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include "BCP_tm_param.hpp"
#include "BCP_parameters.hpp"

#include "BCP_enum_tm.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<BCP_tm_par>::create_keyword_list() {
	// Create the list of keywords for parameter file reading
	//-------------------------------------------------------------------------
	// CharPar
	keys.push_back(make_pair(BCP_string("BCP_DebugLpProcesses"),
							 BCP_parameter(BCP_CharPar,
										   DebugLpProcesses)));
	keys.push_back(make_pair(BCP_string("BCP_DebugCgProcesses"),
							 BCP_parameter(BCP_CharPar,
										   DebugCgProcesses)));
	keys.push_back(make_pair(BCP_string("BCP_DebugVgProcesses"),
							 BCP_parameter(BCP_CharPar,
										   DebugVgProcesses)));
	keys.push_back(make_pair(BCP_string("BCP_DebugCpProcesses"),
							 BCP_parameter(BCP_CharPar,
										   DebugCpProcesses)));
	keys.push_back(make_pair(BCP_string("BCP_DebugVpProcesses"),
							 BCP_parameter(BCP_CharPar,
										   DebugVpProcesses)));
	keys.push_back(make_pair(BCP_string("BCP_DoBranchAndCut"),
							 BCP_parameter(BCP_CharPar,
										   DoBranchAndCut)));
	keys.push_back(make_pair(BCP_string("BCP_AlgorithmicVariablesAreGenerated"),
							 BCP_parameter(BCP_CharPar,
										   AlgorithmicVariablesAreGenerated)));
	keys.push_back(make_pair(BCP_string("BCP_IndexedVariablesAreGenerated"),
							 BCP_parameter(BCP_CharPar,
										   IndexedVariablesAreGenerated)));
	keys.push_back(make_pair(BCP_string("BCP_LpValueIsTrueLowerBound"),
							 BCP_parameter(BCP_CharPar,
										   LpValueIsTrueLowerBound)));
	keys.push_back(make_pair(BCP_string("BCP_PriceInRootBeforePhase2"),
							 BCP_parameter(BCP_CharPar,
										   PriceInRootBeforePhase2)));
	keys.push_back(make_pair(BCP_string("BCP_ReportWhenDefaultIsExecuted"),
							 BCP_parameter(BCP_CharPar,
										   ReportWhenDefaultIsExecuted)));
	keys.push_back(make_pair(BCP_string("BCP_TrimTreeBeforeNewPhase"),
							 BCP_parameter(BCP_CharPar,
										   TrimTreeBeforeNewPhase)));
	keys.push_back(make_pair(BCP_string("BCP_RemoveExploredBranches"),
							 BCP_parameter(BCP_CharPar,
										   RemoveExploredBranches)));

	//-------------------------------------------------------------------------
	keys.push_back(make_pair(BCP_string("BCP_VerbosityShutUp"),
							 BCP_parameter(BCP_CharPar,
										   VerbosityShutUp)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_AllFeasibleSolutionValue"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_AllFeasibleSolutionValue)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_AllFeasibleSolution"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_AllFeasibleSolution)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_BetterFeasibleSolutionValue"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_BetterFeasibleSolutionValue)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_BetterFeasibleSolution"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_BetterFeasibleSolution)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_BestFeasibleSolution"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_BestFeasibleSolution)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_NewPhaseStart"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_NewPhaseStart)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_PrunedNodeInfo"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_PrunedNodeInfo)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_TimeOfImprovingSolution"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_TimeOfImprovingSolution)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_TrimmedNum"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_TrimmedNum)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_FinalStatistics"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_FinalStatistics)));
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_ReportDefault"),
							 BCP_parameter(BCP_CharPar,
										   TmVerb_ReportDefault)));

	//    keys.push_back(make_pair(BCP_string("BCP_"),
	// 			    BCP_parameter(BCP_CharPar,
	// 					  )));

	//-------------------------------------------------------------------------
	// IntPar
	keys.push_back(make_pair(BCP_string("BCP_TmVerb_SingleLineInfoFrequency"),
							 BCP_parameter(BCP_IntPar,
										   TmVerb_SingleLineInfoFrequency)));
	keys.push_back(make_pair(BCP_string("BCP_TreeSearchStrategy"),
							 BCP_parameter(BCP_IntPar,
										   TreeSearchStrategy)));
	keys.push_back(make_pair(BCP_string("BCP_NiceLevel"),
							 BCP_parameter(BCP_IntPar,
										   NiceLevel)));
	keys.push_back(make_pair(BCP_string("BCP_LpProcessNum"),
							 BCP_parameter(BCP_IntPar,
										   LpProcessNum)));
	keys.push_back(make_pair(BCP_string("BCP_CgProcessNum"),
							 BCP_parameter(BCP_IntPar,
										   CgProcessNum)));
	keys.push_back(make_pair(BCP_string("BCP_CpProcessNum"),
							 BCP_parameter(BCP_IntPar,
										   CpProcessNum)));
	keys.push_back(make_pair(BCP_string("BCP_VgProcessNum"),
							 BCP_parameter(BCP_IntPar,
										   VgProcessNum)));
	keys.push_back(make_pair(BCP_string("BCP_VpProcessNum"),
							 BCP_parameter(BCP_IntPar,
										   VpProcessNum)));
	keys.push_back(make_pair(BCP_string("BCP_TmTimeout"),
							 BCP_parameter(BCP_IntPar,
										   TmTimeout)));
	//    keys.push_back(make_pair(BCP_string("BCP_"),
	// 			    BCP_parameter(BCP_IntPar,
	// 					  )));

	//-------------------------------------------------------------------------
	// DoublePar
	keys.push_back(make_pair(BCP_string("BCP_UnconditionalDiveProbability"),
							 BCP_parameter(BCP_DoublePar,
										   UnconditionalDiveProbability)));
	keys.push_back(make_pair(BCP_string("BCP_QualityRatioToAllowDiving_HasUB"),
							 BCP_parameter(BCP_DoublePar,
										   QualityRatioToAllowDiving_HasUB)));
	keys.push_back(make_pair(BCP_string("BCP_QualityRatioToAllowDiving_NoUB"),
							 BCP_parameter(BCP_DoublePar,
										   QualityRatioToAllowDiving_NoUB)));
	keys.push_back(make_pair(BCP_string("BCP_Granularity"),
							 BCP_parameter(BCP_DoublePar,
										   Granularity)));
	keys.push_back(make_pair(BCP_string("BCP_MaxRunTime"),
							 BCP_parameter(BCP_DoublePar,
										   MaxRunTime)));
	keys.push_back(make_pair(BCP_string("BCP_UpperBound"),
							 BCP_parameter(BCP_DoublePar,
										   UpperBound)));
	keys.push_back(make_pair(BCP_string("BCP_TerminationGap_Absolute"),
							 BCP_parameter(BCP_DoublePar,
										   TerminationGap_Absolute)));
	keys.push_back(make_pair(BCP_string("BCP_TerminationGap_Relative"),
							 BCP_parameter(BCP_DoublePar,
										   TerminationGap_Relative)));
	//    keys.push_back(make_pair(BCP_string("BCP_"),
	// 			    BCP_parameter(BCP_DoublePar,
	// 					  )));

	//-------------------------------------------------------------------------
	// StringPar
	keys.push_back(make_pair(BCP_string("BCP_ReadRootCutsFrom"),
							 BCP_parameter(BCP_StringPar,
										   ReadRootCutsFrom)));
	keys.push_back(make_pair(BCP_string("BCP_SaveRootCutsTo"),
							 BCP_parameter(BCP_StringPar,
										   SaveRootCutsTo)));
	keys.push_back(make_pair(BCP_string("BCP_ExecutableName"),
							 BCP_parameter(BCP_StringPar,
										   ExecutableName)));
	keys.push_back(make_pair(BCP_string("BCP_LogFileName"),
							 BCP_parameter(BCP_StringPar,
										   LogFileName)));
	//    keys.push_back(make_pair(BCP_string("BCP_"),
	// 			    BCP_parameter(BCP_StringPar,
	// 					  )));
   
	//-------------------------------------------------------------------------
	// StringArrayPar
	keys.push_back(make_pair(BCP_string("BCP_LpMachine"),
							 BCP_parameter(BCP_StringArrayPar,
										   LpMachines)));
	keys.push_back(make_pair(BCP_string("BCP_CgMachine"),
							 BCP_parameter(BCP_StringArrayPar,
										   CgMachines)));
	keys.push_back(make_pair(BCP_string("BCP_VgMachine"),
							 BCP_parameter(BCP_StringArrayPar,
										   VgMachines)));
	keys.push_back(make_pair(BCP_string("BCP_CpMachine"),
							 BCP_parameter(BCP_StringArrayPar,
										   CpMachines)));
	keys.push_back(make_pair(BCP_string("BCP_VpMachine"),
							 BCP_parameter(BCP_StringArrayPar,
										   VpMachines)));
	//    keys.push_back(make_pair(BCP_string("BCP_"),
	// 			    BCP_parameter(BCP_StringArrayPar,
	// 					  )));
}

//#############################################################################

template <>
void BCP_parameter_set<BCP_tm_par>::set_default_entries(){
	//-------------------------------------------------------------------------
	// CharPar
	set_entry(DebugLpProcesses, false);
	set_entry(DebugCgProcesses, false);
	set_entry(DebugVgProcesses, false);
	set_entry(DebugCpProcesses, false);
	set_entry(DebugVpProcesses, false);
	set_entry(DoBranchAndCut, false);
	set_entry(AlgorithmicVariablesAreGenerated, false);
	set_entry(IndexedVariablesAreGenerated, false);
	set_entry(LpValueIsTrueLowerBound, true);
	set_entry(MessagePassingIsSerial, false);
	set_entry(PriceInRootBeforePhase2, false);
	set_entry(ReportWhenDefaultIsExecuted, true);
	set_entry(TrimTreeBeforeNewPhase, false);
	set_entry(RemoveExploredBranches, false);
	//-------------------------------------------------------------------------
	set_entry(VerbosityShutUp, false);
	set_entry(TmVerb_AllFeasibleSolutionValue, true);
	set_entry(TmVerb_AllFeasibleSolution, false);
	set_entry(TmVerb_BetterFeasibleSolutionValue, true);
	set_entry(TmVerb_BetterFeasibleSolution, false);
	set_entry(TmVerb_BestFeasibleSolution, true);
	set_entry(TmVerb_NewPhaseStart, true);
	set_entry(TmVerb_TrimmedNum, true);
	set_entry(TmVerb_TimeOfImprovingSolution, true);
	set_entry(TmVerb_PrunedNodeInfo, true);
	set_entry(TmVerb_FinalStatistics, true);
	set_entry(TmVerb_ReportDefault, true);
	//-------------------------------------------------------------------------
	// IntPar
	set_entry(TmVerb_SingleLineInfoFrequency, 0);
	set_entry(TreeSearchStrategy, BCP_BestFirstSearch);
	set_entry(NiceLevel, 0);
	set_entry(LpProcessNum, 1);
	set_entry(CgProcessNum, 0);
	set_entry(CpProcessNum, 0);
	set_entry(VgProcessNum, 0);
	set_entry(VpProcessNum, 0);
	set_entry(TmTimeout, -1);
	//-------------------------------------------------------------------------
	// DoublePar
	set_entry(UnconditionalDiveProbability, 0.02);
	set_entry(QualityRatioToAllowDiving_HasUB, 1.2);
	set_entry(QualityRatioToAllowDiving_NoUB, 2.0);
	set_entry(Granularity, 1e-8);
	set_entry(MaxRunTime, 3600.0); // one hour
	set_entry(TerminationGap_Absolute, 0.0);
	set_entry(TerminationGap_Relative, 0.0);
	set_entry(UpperBound, DBL_MAX);
	//-------------------------------------------------------------------------
	// StringPar
	set_entry(ReadRootCutsFrom, "");
	set_entry(SaveRootCutsTo, "");
	set_entry(ExecutableName, "bcpp");
	set_entry(LogFileName,"");
}
