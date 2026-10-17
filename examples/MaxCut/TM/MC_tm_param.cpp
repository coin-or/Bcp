// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "MC_tm_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<MC_tm_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("MC_DisplaySolutionSignature"),
			    BCP_parameter(BCP_CharPar,
					  DisplaySolutionSignature)));
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));
   //--------------------------------------------------------------------------
   // IntPar
   keys.push_back(make_pair(BCP_string("MC_DigitsToLose"),
			    BCP_parameter(BCP_IntPar,
					  DigitsToLose)));
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));
   //--------------------------------------------------------------------------
   // DoublePar
//    keys.push_back(make_pair(BCP_string("MC_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
   keys.push_back(make_pair(BCP_string("MC_FeasSolFile"),
			    BCP_parameter(BCP_StringPar,
					  FeasSolFile)));
   keys.push_back(make_pair(BCP_string("MC_InputFile"),
			    BCP_parameter(BCP_StringPar,
					  InputFile)));
   keys.push_back(make_pair(BCP_string("MC_SolutionFile"),
			    BCP_parameter(BCP_StringPar,
					  SolutionFile)));
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
//    keys.push_back(make_pair(BCP_string("MC_TmVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_TmVerb_PrunedNodeInfo)));
}

//#############################################################################

template <>
void BCP_parameter_set<MC_tm_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(DisplaySolutionSignature, false);
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(DigitsToLose, 0);
   //--------------------------------------------------------------------------
   // DoublePar
   //--------------------------------------------------------------------------
   // StringPar
   set_entry(FeasSolFile, "");
   set_entry(InputFile, "graph.in");
   set_entry(SolutionFile, "");
   //--------------------------------------------------------------------------
   // StringArrayPar
   //--------------------------------------------------------------------------
   // BooleanListPar
}
