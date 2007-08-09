// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_enum.hpp"
#include "MKC_tm_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<MKC_tm_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("MKC_DetailedFeasibleSolution"),
			    BCP_parameter(BCP_CharPar,
					  DetailedFeasibleSolution)));
   keys.push_back(make_pair(BCP_string("MKC_SolveLpForLB"),
			    BCP_parameter(BCP_CharPar,
					  SolveLpForLB)));
   keys.push_back(make_pair(BCP_string("MKC_TestFinalSolution"),
			    BCP_parameter(BCP_CharPar,
					  TestFinalSolution)));

   keys.push_back(make_pair(BCP_string("MKC_CreateRootFromInputVars"),
			    BCP_parameter(BCP_CharPar,
					  CreateRootFromInputVars)));
   keys.push_back(make_pair(BCP_string("MKC_DeleteOrdersNotInVarFile"),
			    BCP_parameter(BCP_CharPar,
					  DeleteOrdersNotInVarFile)));
   keys.push_back(make_pair(BCP_string("MKC_ReadFromVarFile"),
			    BCP_parameter(BCP_CharPar,
					  ReadFromVarFile)));
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));

   //--------------------------------------------------------------------------
   // IntPar
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));

   //--------------------------------------------------------------------------
   // DoublePar
   keys.push_back(make_pair(BCP_string("MKC_StartingDualValue"),
			    BCP_parameter(BCP_DoublePar,
					  StartingDualValue)));
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
   keys.push_back(make_pair(BCP_string("MKC_InputFile"),
			    BCP_parameter(BCP_StringPar,
					  InputFile)));
   keys.push_back(make_pair(BCP_string("MKC_InputVarFile"),
			    BCP_parameter(BCP_StringPar,
					  InputVarFile)));
//    keys.push_back(make_pair(BCP_string("MKC_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   
   //--------------------------------------------------------------------------
   // BooleanListPar
//    keys.push_back(make_pair(BCP_string("MKC_TmVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_TmVerb_PrunedNodeInfo)));
}

//#############################################################################


template <>
void BCP_parameter_set<MKC_tm_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(DetailedFeasibleSolution, true);
   set_entry(SolveLpForLB, true);
   set_entry(TestFinalSolution, true);

   set_entry(CreateRootFromInputVars, false);
   set_entry(DeleteOrdersNotInVarFile, false);
   set_entry(ReadFromVarFile, false);
   //--------------------------------------------------------------------------
   // IntPar
   //--------------------------------------------------------------------------
   // DoublePar
   set_entry(StartingDualValue, 0.0);
   //--------------------------------------------------------------------------
   // StringPar
   set_entry(InputFile,"mkc7.mps");
   set_entry(InputVarFile,"mkc7.var");
   //--------------------------------------------------------------------------
   // BooleanListPar
}
