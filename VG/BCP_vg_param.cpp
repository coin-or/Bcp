// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_vg_param.hpp"
#include "BCP_parameters.hpp"

using std::make_pair;

template <>
void BCP_parameter_set<BCP_vg_par>::create_keyword_list() {
   // Create the list of keywords for parameter file reading
   //--------------------------------------------------------------------------
   // CharPar
   keys.push_back(make_pair(BCP_string("BCP_MessagePassingIsSerial"),
			    BCP_parameter(BCP_CharPar,
					  MessagePassingIsSerial)));
   keys.push_back(make_pair(BCP_string("BCP_ReportWhenDefaultIsExecuted"),
			    BCP_parameter(BCP_CharPar,
					  ReportWhenDefaultIsExecuted)));
//    keys.push_back(make_pair(BCP_string("BCP_"),
// 			    BCP_parameter(BCP_CharPar,
// 					  )));

   //--------------------------------------------------------------------------
   // IntPar
   keys.push_back(make_pair(BCP_string("BCP_NiceLevel"),
			    BCP_parameter(BCP_IntPar,
					  NiceLevel)));
//    keys.push_back(make_pair(BCP_string("BCP_"),
// 			    BCP_parameter(BCP_IntPar,
// 					  )));

   //--------------------------------------------------------------------------
   // DoublePar
//    keys.push_back(make_pair(BCP_string("BCP_"),
// 			    BCP_parameter(BCP_DoublePar,
// 					  )));

   //--------------------------------------------------------------------------
   // StringPar
   keys.push_back(make_pair(BCP_string("BCP_LogFileName"),
			    BCP_parameter(BCP_StringPar,
					  LogFileName)));
//    keys.push_back(make_pair(BCP_string("BCP_"),
// 			    BCP_parameter(BCP_StringPar,
// 					  )));
   
   //--------------------------------------------------------------------------
   // BoolArrayPar
//    keys.push_back(make_pair(BCP_string("BCP_"),
// 			    BCP_parameter(BCP_BoolArrayPar,
// 					  Verbosity,
// 					  )));
}

//#############################################################################

#ifdef TRUE
#undef TRUE
#endif
#define TRUE static_cast<const char>(true)

#ifdef FALSE
#undef FALSE
#endif
#define FALSE static_cast<const char>(false)

template <>
void BCP_parameter_set<BCP_vg_par>::set_default_entries(){
   //--------------------------------------------------------------------------
   // CharPar
   set_entry(ReportWhenDefaultIsExecuted, TRUE);
   set_entry(MessagePassingIsSerial, TRUE);
   //--------------------------------------------------------------------------
   // IntPar
   set_entry(NiceLevel, 0);
   //--------------------------------------------------------------------------
   // DoublePar
   //--------------------------------------------------------------------------
   // StringPar
   set_entry(LogFileName,"");
   //--------------------------------------------------------------------------
   // BoolArrayPar
}
