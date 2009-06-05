// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include <fstream>
#include <algorithm>

#include "CSP_init.hpp"
#include "CSP_tm.hpp"
#include "CSP_lp.hpp"
#include "CSP_var.hpp"

//#############################################################################

USER_initialize *
BCP_user_init()
{
   return new CSP_initialize;
}

//#############################################################################

BCP_user_pack*
CSP_initialize::packer_init(BCP_user_class* p)
{
  return new CSP_packer;
}

//-----------------------------------------------------------------------------

void
CSP_packer::pack_var_algo(const BCP_var_algo* var, BCP_buffer& buf)
{
  CSP_var_pack(var, buf);
}

//-----------------------------------------------------------------------------

BCP_var_algo*
CSP_packer::unpack_var_algo(BCP_buffer& buf)
{
  return CSP_var_unpack(buf);
}

//#############################################################################
//#############################################################################

BCP_lp_user *
CSP_initialize::lp_init(BCP_lp_prob& p)
{
   return new CSP_lp;
}

//#############################################################################

// This method reads the parameter file 
// and sets up the basic data structure, a CSPROBLEM instance

BCP_tm_user *
CSP_initialize::tm_init(BCP_tm_prob& p,
			const int argnum, const char * const * arglist)
{
   const char * paramfile = arglist[1];
   CSP_tm* csp = new CSP_tm;

   csp->tm_par.read_from_file(paramfile);
   csp->lp_par.read_from_file(paramfile);

   std::ifstream is(csp->tm_par.entry(CSP_tm_par::InputFile).c_str());

   // sdv add errorcondition whether file-opening was successfull
   if ( !is ){
      std::string err("Input file: ");
      err += std::string(csp->tm_par.entry(CSP_tm_par::InputFile).c_str())
	 +"  \nis unreadable.\n";
      throw BCP_fatal_error(err.c_str());//"Input file is unreadable.\n");
   }

   // calling contructor
   csp->csproblem = new CSPROBLEM(is);

   csp->csproblem->setCombineExclusionConstraints
      (csp->tm_par.entry(CSP_tm_par::combineExclusionConstraints));
   csp->csproblem->setAddKnapsackMirConstraints
      (csp->tm_par.entry(CSP_tm_par::addKnapsackMirConstraints));
   csp->csproblem->setAddKnifeMirConstraints
      (csp->tm_par.entry(CSP_tm_par::addKnifeMirConstraints));      

   //cout << *csp->auction;
   is.close();

   return csp;
}
