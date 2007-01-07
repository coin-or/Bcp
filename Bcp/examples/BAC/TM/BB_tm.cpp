// Last edit: 1/6/07
//
// Name:     BB_tm.cpp
// Author:   Francois Margot
//           Tepper School of Business
//           Carnegie Mellon University, Pittsburgh, PA 15213
//           email: fmargot@andrew.cmu.edu
// Date:     12/28/03
//-----------------------------------------------------------------------------
// Copyright (C) 2003, Francois Margot, International Business Machines
// Corporation and others.  All Rights Reserved.

#include<iomanip>

#include <CoinHelperFunctions.hpp>
#include <CoinFileIO.hpp>
#include <OsiClpSolverInterface.hpp>

#include "BB_init.hpp"
#include "BCP_tm.hpp"
#include "BB_tm.hpp"
#include "BB_cut.hpp"
#include "BB_user_data.hpp"

using namespace std;

/*************************************************************************/

int main(int argc, char* argv[])
{
	BB_init bb_init;
	return bcp_main(argc, argv, &bb_init);
}

/*************************************************************************/
void
BB_tm::readInput(const char* filename)

   // Initialize the data members of class BB_prob

{
   int i;

   // Try to read in the problem from file bac.lp. 
   // If that fails then build the problem from scratch.
   // Note: Do not try to replace the file bac.lp by another 
   // lp file. The code, most likely, will crash.

   bool found_file = false;
   CoinFileInput* fi = NULL;

   OsiClpSolverInterface solver;  // defined in class BCP_tm_user

   if (!found_file) {
     // Try bac.lp
     try {
       found_file = true;
       fi = CoinFileInput::create("bac.lp");
     }
     catch (CoinError& err) {
       // bac.lp not found
       found_file = false;
     }
     if (found_file) {
       solver.readLp("bac.lp");
     }
   }

   // For MPS files, use the following:
   /**********
   if (!found_file) {
     // Try bac.mps
     try {
       found_file = true;
       fi = CoinFileInput::create("bac.mps");
     }
     catch (CoinError& err) {
       // bac.lp not found
       found_file = false;
     }
     if (found_file) {
	   solver.readMps("bac.mps");
     }
   }
   **********/

   if (found_file) {
     
     const int rownum = solver.getNumRows();
     const int colnum = solver.getNumCols();
     desc.rownum = rownum;
     desc.colnum = colnum;
     
     // extract integrality information
     
     desc.integer = new bool[colnum];
     for (i = 0; i < colnum; ++i) {
       desc.integer[i] = solver.isInteger(i);
     }
     
     // extract variable bounds and objective
     
     desc.clb = new double[colnum];
     CoinDisjointCopyN(solver.getColLower(), colnum, desc.clb);
     
     desc.cub = new double[colnum];
     CoinDisjointCopyN(solver.getColUpper(), colnum, desc.cub);
     
     desc.obj = new double[colnum];
     CoinDisjointCopyN(solver.getObjCoefficients(), colnum, desc.obj);
     
     const CoinPackedMatrix* byRow = solver.getMatrixByRow();
     
     int core_size = 10;
     int *core_ind = new int[core_size];
     desc.rlb_core = new double[core_size];
     desc.rub_core = new double[core_size];
     
     int indexed_size = 6;
     int *indexed_ind = new int[indexed_size];
     desc.rlb_indexed = new double[indexed_size];
     desc.rub_indexed = new double[indexed_size];
     
     
     for (i=0; i<core_size; i++) {
       core_ind[i] = i;
     }
     for (i=core_size; i<rownum; i++) {
       indexed_ind[i-core_size] = i;
     }
     
     cout << "readInput(): core size: " << core_size << "   indexed size: " 
	  << indexed_size << endl;
     
     // make a copy of rlb/rub in the appropriate order
     
     const double* solver_rlb = solver.getRowLower();
     const double* solver_rub = solver.getRowUpper();
     
     for (i=0; i<core_size; ++i) {
       desc.rlb_core[i] = solver_rlb[core_ind[i]];
       desc.rub_core[i] = solver_rub[core_ind[i]];
     }
     
     for (i=0; i<indexed_size; ++i) {
       desc.rlb_indexed[i] = solver_rlb[indexed_ind[i]];
       desc.rub_indexed[i] = solver_rub[indexed_ind[i]];
     }
     
     // split the byRow matrix into core and indexed constraints
     // these two variables are part of the BB_tm class
     
     desc.core = new CoinPackedMatrix(false, 0.0, 0.0);
     desc.core->submatrixOf(*byRow, core_size, core_ind);
     
     desc.indexed = new CoinPackedMatrix(false, 0.0, 0.0);
     desc.indexed->submatrixOf(*byRow, indexed_size, indexed_ind);
     
     delete[] core_ind;
     delete[] indexed_ind;
     
   } else {
     // create the problem from scratch 
     
     desc.colnum = 10;
     int colnum = desc.colnum;
     desc.rownum = 16;
     
     desc.obj = new double[colnum];
     desc.clb = new double[colnum];
     desc.cub = new double[colnum];
     desc.integer = new bool[colnum];
     
     for(i=0; i<colnum; i++) {
       desc.obj[i] = -1;
       desc.clb[i] = 0;
       desc.cub[i] = 1;
       desc.integer[i] = true;
     }
     
     desc.core = new CoinPackedMatrix(false, 0.0, 0.0);
     desc.rlb_core = new double[10];
     desc.rub_core = new double[10];
     
     double* cutcoef = new double[colnum];
     int *cutind = new int[colnum];
     int j, cutnz;
     
     // core constraints
     
     cutcoef[0] = 1;
     cutcoef[1] = 1;
     cutnz = 2;
     
     for(i=0; i<10; i++) {
       desc.rlb_core[i] = 0;
       desc.rub_core[i] = 1;
       j = (i+1) % colnum;
       cutind[0] = i;
       cutind[1] = j;
       desc.core->appendRow(cutnz, cutind, cutcoef);
     }
     desc.rlb_core[0] = 1; // first constraint is an equality
     
     desc.indexed = new CoinPackedMatrix(false, 0.0, 0.0);
     desc.rlb_indexed = new double[6];
     desc.rub_indexed = new double[6];
     
     // indexed constraints
     
     for(i=0; i<6; i++) {
       desc.rlb_indexed[i] = -DBL_MAX;
       desc.rub_indexed[i] = 1;
     }
     
     cutcoef[0] = 1;
     cutcoef[1] = 1;
     cutcoef[2] = 1;
     cutnz = 3;
     
     cutind[1] = 1;
     cutind[2] = 3;
     cutind[0] = 9;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);
     cutind[0] = 0;
     cutind[1] = 2;
     cutind[2] = 4;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);
     cutind[0] = 0;
     cutind[1] = 3;
     cutind[2] = 7;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);
     cutind[0] = 1;
     cutind[1] = 4;
     cutind[2] = 5;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);
     cutind[0] = 5;
     cutind[1] = 6;
     cutind[2] = 7;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);
     cutind[0] = 0;
     cutind[1] = 6;
     cutind[2] = 8;
     desc.indexed->appendRow(cutnz, cutind, cutcoef);   
     
     delete[] cutcoef;
     delete[] cutind;
   }
}

/*************************************************************************/
void
BB_tm::pack_module_data(BCP_buffer& buf, BCP_process_t ptype)
{
  // possible process types looked up in BCP_enum_process_t.hpp
  
  switch (ptype) {
  case BCP_ProcessType_LP:
    {
      // Pack a pointer; does not work for parallel machines
      
      buf.pack(&desc); 
    }
    break;
  default:
    abort();
  }
}

/*************************************************************************/
void
BB_tm::pack_cut_algo(const BCP_cut_algo* cut, BCP_buffer& buf)
{
  const BB_cut *bb_cut = dynamic_cast<const BB_cut*>(cut);
  if (!bb_cut)
    throw BCP_fatal_error("BB_lp::pack_cut_algo() : unknown cut type!\n");
  
  bb_cut->pack(buf);
  return;
}

/*************************************************************************/
BCP_cut_algo*
BB_tm::unpack_cut_algo(BCP_buffer& buf)
{
  return new BB_cut(buf);
}

/*************************************************************************/
void
BB_tm::initialize_core(BCP_vec<BCP_var_core*>& vars,
		       BCP_vec<BCP_cut_core*>& cuts,
		       BCP_lp_relax*& matrix)
  
// Transmit the core constraints and variables to BCP
  
{
  int i;
  
  for (i=0; i<desc.colnum; ++i) {
    if (desc.integer[i]) {
      if (desc.clb[i] == 0.0 && desc.cub[i] == 1.0) {
	vars.push_back(new BCP_var_core(BCP_BinaryVar, desc.obj[i], 0, 1));
      }
      else {
	vars.push_back(new BCP_var_core(BCP_IntegerVar, desc.obj[i],
					desc.clb[i], desc.cub[i]));
      }
    } 
    else {
      vars.push_back(new BCP_var_core(BCP_ContinuousVar, desc.obj[i],
				      desc.clb[i], desc.cub[i]));
    }
  }
  
  const int corerows = desc.core->getNumRows();
  for (i=0; i<corerows; ++i) {
    cuts.push_back(new BCP_cut_core(desc.rlb_core[i], desc.rub_core[i]));
  }
  
  matrix = new BCP_lp_relax;
  
  matrix->copyOf(*desc.core, desc.obj, desc.clb, desc.cub,
		 desc.rlb_core, desc.rub_core);
}

/**************************************************************************/
void
BB_tm::create_root(BCP_vec<BCP_var*>& added_vars,
                   BCP_vec<BCP_cut*>& added_cuts,
		   BCP_user_data*& user_data,
                   BCP_pricing_status& pricing_status)
{
  
#ifdef USER_DATA
  user_data = new MY_user_data(desc.colnum);
#endif
}

/*************************************************************************/
void
BB_tm::display_feasible_solution(const BCP_solution *soln)
{
  int i;
  const BCP_solution_generic *gsol = 
                      dynamic_cast<const BCP_solution_generic*>(soln);
  
  // put together the solution vector
  
  const int colnum = desc.colnum;
  double* sol = new double[colnum];
  
  CoinFillN(sol, colnum, 0.0);
  
  const int size = gsol->_vars.size();
  for (i=0; i<size; ++i) {
    sol[gsol->_vars[i]->bcpind()] = gsol->_values[i];
  }
  
  cout << "Customized display of the feasible solution:" << endl << endl;
  cout.setf(ios::fixed);
  cout.precision(2);
  
  for(i=0; i<colnum; i++) {
    cout << sol[i] << " ";
    
    if(i % 10 == 9) {
      cout << endl;
    }
  }
  cout << endl;
  
  
  cout << "Default BCP display of the feasible solution:" << endl << endl;
  BCP_tm_user::display_feasible_solution(soln);
  
  delete[] sol;
}

/***************************************************************************/
void
BB_tm::pack_user_data(const BCP_user_data* ud, BCP_buffer& buf)
{
  // Normally, no modifications required.
  
  const MY_user_data *mud = dynamic_cast<const MY_user_data*> (ud);
  if(!mud)
    throw BCP_fatal_error("BB_lp::pack_user_data() : unknown data type!\n");
  
  printf("BB_tm::pack_user_data:\n");
  mud->print();
  mud->pack(buf);
} /* pack_user_data */

/***************************************************************************/
BCP_user_data*
BB_tm::unpack_user_data(BCP_buffer& buf)
{
  // Normally, no modifications required.
  
  MY_user_data *p_ud = new MY_user_data(buf);
  
  printf("BB_tm::unpack_user_data:\n");
  p_ud->print();
  
  if(p_ud->is_processed) {
    p_ud->p_rud = NULL;
    delete(p_ud);
    p_ud = NULL;
    printf("user_data deleted\n");
  }
  
  return(p_ud);
} /* unpack_user_data */

