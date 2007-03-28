// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <iomanip>

#include "CSP_init.hpp"
#include "CSP_tm.hpp"
#include "CSP_var.hpp"
#include "CoinHelperFunctions.hpp"


//#############################################################################

int main(int argc, char* argv[])
{
    CSP_initialize csp_init;
    return bcp_main(argc, argv, &csp_init);
}

//#############################################################################

void
CSP_tm::pack_module_data(BCP_buffer& buf, BCP_process_t ptype)
{
   lp_par.pack(buf);
   csproblem->pack(buf);
}

//#############################################################################

void
CSP_tm::initialize_core(BCP_vec<BCP_var_core*>& vars,
			BCP_vec<BCP_cut_core*>& cuts,
			BCP_lp_relax*& matrix)
{
   // CSP: There are no base vars (hence no base matrix) only base cuts: one for
   // each demand constraint.

  // reserve sufficient space; one for each demand
  // "cuts" are all the constraints in BCP-land.
  const int cutnum = csproblem->getM();
  cuts.reserve(cutnum);

  int i;
  const int* demand = csproblem->getDemand();
  for (i = 0; i < cutnum; ++i) {
    // unchecked is fast, if you're sure how much 
    // you're putting in (and you've reserved sufficient space)
    cuts.unchecked_push_back(new BCP_cut_core(demand[i],
					      DBL_MAX));

  }
}

//#############################################################################

void
CSP_tm::create_root(BCP_vec<BCP_var*>& added_vars,
					BCP_vec<BCP_cut*>& added_cuts,
					BCP_user_data*& user_data,
					BCP_pricing_status& pricing_status)
{
  // CSP: we're starting with no core variables, but we're 
  // not starting out the root empty.
  // Fillup the added variables

  // there is a enum that says what you want to do with the 
  // different types of vars. For every node there is a status flag
  // one of which is what kind of vars still need to be priced
  // out to find a solution
  // price index vars, or algo vars, or neither, etc. 

  // This says to price out algorithmic vars
   pricing_status = BCP_PriceAlgoVars;

   // BCP will
   // First create core of problem - that's the previous func.
   // We specify core constr but there are no core vars.
   // Here we specify the additional ones that go in the root
   // but aren't part of "core", so they can be deleted from the 
   // formulation.
   int i;
   const int rollWidth = csproblem->getL();
   const int *  itemWidths = csproblem->getW();
   const int   knifes = csproblem->getS();
   const int * demand = csproblem->getDemand();

   for (i = 0; i < csproblem->getM(); ++i) {

     // generate one (regular) column for each item
     // (warm starting was useless in b&p - says LL)
     
     
     // integer division automatically gives you int
     // even though it's assigned to double
     double value = rollWidth/itemWidths[i];

     if (knifes > -1) {
       // now if we want knife constraints...
       // and this value is larger than the number of knifes,
       // we have to constrain it. 
       int scrap = rollWidth - ((int)value)*itemWidths[i];
       if (scrap == 0 && value > knifes + 1){
	 value = knifes;
       }
       //    if (scrap == 0 && value <=  knifes+ 1){
       // we're cool
       //}
       if (scrap > 0 && value > knifes){
	 value = knifes;             
       }
       // if (scrap > 0 && value <= knifes) {
       //cool
       // }
     }

     PATTERN pat(1, &i, &value, ceil(demand[i]/value));
     added_vars.push_back(new CSP_var(pat));
   }
}

//#############################################################################

void
CSP_tm::display_feasible_solution(const BCP_solution* soln)
{
   const BCP_solution_generic* sol =
      dynamic_cast<const BCP_solution_generic*>(soln);
   if (!sol)
      throw BCP_fatal_error("What is this solution?!\n");
   std::cout << std::endl
	     << "CSP: Solution start ========================================="
	     << std::endl;
   std::cout << std::setprecision(0);
   std::cout << "CSP solution value: "
	     << soln->objective_value() << std::endl;

   const BCP_vec<BCP_var*> vars = sol->_vars;
   const int var_num = vars.size();
   
   const int size = csproblem->getM();
   double * fullVec = new double[size];
   
   for (int i = 0; i < var_num; ++i) {
     CoinFillN(fullVec, size, 0.0);

     const PATTERN* pat = dynamic_cast<const PATTERN*>(vars[i]);
     const CSP_packedVector&  pv = pat->getWidths();
     for (int j=0; j<pv.getSize(); ++j){
       fullVec[pv.getIndices()[j]]=pv.getElements()[j];
     }

     printf("%5.0f : ", sol->_values[i]);

     for (int j=0; j<size; ++j){
       printf("%4.0f", fullVec[j]);
     }
     printf("\n");
   }
   delete[] fullVec;

   std::cout << std::endl
	     << "CSP: Solution end ==========================================="
	     << std::endl;

}

//#############################################################################
// have to override if you want to do column generation
void
CSP_tm::init_new_phase(int phase, BCP_column_generation& colgen) {
   colgen = BCP_GenerateColumns;
}
