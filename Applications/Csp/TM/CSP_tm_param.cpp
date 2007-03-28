// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_enum.hpp"
#include "CSP.hpp"
#include "CSP_tm_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<CSP_tm_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("CSP_combineExclusionConstraints"),
			    BCP_parameter(BCP_CharPar,
					  combineExclusionConstraints)));
   keys.push_back(make_pair(BCP_string("CSP_addKnapsackMirConstraints"),
			    BCP_parameter(BCP_CharPar,
					  addKnapsackMirConstraints)));
   keys.push_back(make_pair(BCP_string("CSP_addKnifeMirConstraints"),
			    BCP_parameter(BCP_CharPar,
					  addKnifeMirConstraints)));
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));

   //--------------------------------------------------------------------------
   // IntPar
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));

   //--------------------------------------------------------------------------
   // DoublePar
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
   keys.push_back(make_pair(BCP_string("CSP_InputFile"),
			    BCP_parameter(BCP_StringPar,
					  InputFile)));
//    keys.push_back(make_pair(BCP_string("CSP_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   
   //--------------------------------------------------------------------------
   // BooleanListPar
//    keys.push_back(make_pair(BCP_string("CSP_TmVerbosity_PrunedNodeInfo"),
// 			    BCP_parameter(BCP_BooleanListPar,
// 					  Verbosity,
// 					  BCP_TmVerb_PrunedNodeInfo)));
}

//#############################################################################


template <>
void BCP_parameter_set<CSP_tm_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(combineExclusionConstraints, true);
   set_entry(addKnapsackMirConstraints, true);
   set_entry(addKnifeMirConstraints, true);
   //--------------------------------------------------------------------------
   // IntPar
  //   set_entry(ForcedBid, -1);
//     set_entry(BranchingStrategy,-1);
//     set_entry(ColumnStrategy,-1);
  
   //--------------------------------------------------------------------------
   // DoublePar
   //--------------------------------------------------------------------------
   // StringPar
   set_entry(InputFile,"defaultInput.txt");
   //--------------------------------------------------------------------------
   // BooleanListPar
}
