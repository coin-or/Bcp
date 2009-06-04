// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "CoinHelperFunctions.hpp"

#include "MKC_tm.hpp"
#include "MKC_init.hpp"
#include "MKC_var.hpp"
#include "MKC_solution.hpp"
#include "MKC_vargen.hpp"

//#############################################################################

int main(int argc, char* argv[])
{
    MKC_initialize mkc_init;
    return bcp_main(argc, argv, &mkc_init);
}

//#############################################################################

void
MKC_tm::pack_module_data(BCP_buffer& buf, BCP_process_t ptype)
{
   lp_par.pack(buf);
   kss.pack(buf);
   const int varnum = input_vars.size();
   buf.pack(varnum);
   for (int i = 0; i < varnum; ++i)
      input_vars[i]->pack(buf);
}

//#############################################################################

BCP_solution*
MKC_tm::unpack_feasible_solution(BCP_buffer& buf)
{
   return new MKC_solution(buf);
}

//#############################################################################

void
MKC_tm::initialize_core(BCP_vec<BCP_var_core*>& vars,
			BCP_vec<BCP_cut_core*>& cuts,
			BCP_lp_relax*& matrix)
{
   // There are no base vars (hence no base matrix) only base cuts: one for
   // each KS and one for each order
   const int bcutnum = kss.ks_num + kss.order_num;
   cuts.reserve(bcutnum);
   for (int i = 0; i < bcutnum; ++i)
      cuts.unchecked_push_back(new BCP_cut_core(-1e+40, 1.0));
}

//#############################################################################

void
MKC_tm::create_root(BCP_vec<BCP_var*>& added_vars,
		    BCP_vec<BCP_cut*>& added_cuts,
		    BCP_user_data*& user_data)
{
   const int bcutnum = kss.ks_num + kss.order_num;

   if (tm_par.entry(MKC_tm_par::CreateRootFromInputVars)) {
      const int varnum = input_vars.size();
      added_vars.reserve(varnum);
      for (int i = 0; i < varnum; ++i)
	 added_vars.unchecked_push_back(new MKC_var(*input_vars[i]));
      input_vars.clear();
   } else {
      double* dual = new double[bcutnum];
      CoinFillN(dual, bcutnum, 0.0);
      for (int i = 0; i < kss.ks_num; ++i)
	 dual[i] = tm_par.entry(MKC_tm_par::StartingDualValue);

      MKC_knapsack_fixing* ks_fixings = new MKC_knapsack_fixing[kss.ks_num];
      for (int i = kss.ks_num - 1; i >= 0; --i) {
	 ks_fixings[i].fixing.insert(ks_fixings[i].fixing.end(),
				     kss.order_num, 1);
      }
      // here we look only for vars with negative red cost.
      bool for_all_knapsack = lp_par.entry(MKC_lp_par::ParallelVargenForAllKS);
      bool all_encountered_var = lp_par.entry(MKC_lp_par::AddAllGeneratedVars);
      bool print_best_dj = lp_par.entry(MKC_lp_par::PrintBestDj);
      bool exact_fallback = lp_par.entry(MKC_lp_par::ExactFallbackAtVargen);

      MKC_generate_variables(kss, ks_fixings,
			     0, 0, /* no enumerated KS's yet */
			     dual, 0.0, added_vars, 0.01,
			     for_all_knapsack, all_encountered_var,
			     print_best_dj, exact_fallback, 0, 0);
      delete[] ks_fixings;
      delete[] dual;
   }
}

//#############################################################################

void
MKC_tm::display_feasible_solution(const BCP_solution* soln)
{
   // the cast must succeed
   const MKC_solution* sol = dynamic_cast<const MKC_solution*>(soln);
   int i, j;
   int loc;
   const BCP_vec<MKC_var*>& vars = sol->_vars;

   if (tm_par.entry(MKC_tm_par::TestFinalSolution)) {
      const int ncols = clp->getNumCols();
      int * ind = new int[ncols];
      int cnt = 0;
      for (i = vars.size() - 1; i >= 0; --i) {
	 MKC_var* var = vars[i];
	 const int * entries = var->entries;
	 const MKC_knapsack& ks = kss.ks_list[entries[0]];
	 for (j = 1; j < var->entry_num; ++j) {
	    const int index = entries[j] - kss.ks_num;
	    for (loc = 0; loc < ks.size; ++loc)
	       if (ks.entries[loc].index == index)
		  break;
	    if (loc == ks.size)
	       abort();
	    ind[cnt++] = ks.entries[loc].orig_index;
	 }
	 ind[cnt++] = ks.orig_index;
      }
      if (cnt > 0) {
	 double * bnd = new double[2*cnt];
	 std::fill(bnd, bnd + 2*cnt, 1.0);
	 clp->setColSetBounds(ind, ind+cnt, bnd);
	 delete[] bnd;
      }
      clp->initialSolve();
      if (! clp->isProvenOptimal())
	 abort();
      cnt = 0;
      for (i = vars.size() - 1; i >= 0; --i) {
	 MKC_var* var = vars[i];
	 const int * entries = var->entries;
	 const MKC_knapsack& ks = kss.ks_list[entries[0]];
	 for (j = 1; j < var->entry_num; ++j) {
	    const int index = entries[j] - kss.ks_num;
	    for (loc = 0; loc < ks.size; ++loc)
	       if (ks.entries[loc].index == index)
		  break;
	    if (loc == ks.size)
	       abort();
	    ind[cnt++] = ks.entries[loc].orig_index;
	 }
	 ind[cnt++] = ks.orig_index;
      }
      if (cnt > 0) {
	 double * bnd = new double[2*cnt];
	 std::fill(bnd, bnd + 2*cnt, 0.0);
	 clp->setColSetBounds(ind, ind+cnt, bnd);
	 delete[] bnd;
      }
      delete[] ind;
   }

   // display the solution in two different ways
   // first which orders are produced from which slab
   const bool detailed = tm_par.entry(MKC_tm_par::DetailedFeasibleSolution)!=0;
   if (detailed) {
      printf("MKC: Displaying solution =========== part 1 ==============\n\n");
      for (i = vars.size() - 1; i >= 0; --i) {
	 MKC_var* var = vars[i];
	 printf("MKC: From slab %i the following orders are produced:\n     ",
		var->entries[0]);
	 const int * entries = var->entries;
	 for (j = 1; j < var->entry_num; ++j) {
	    if (j % 8 == 0)
	       printf("\n");
	    printf("%9i", entries[j] - kss.ks_num);
	 }
	 printf("\n");
      }
      printf("\n");
   }

   if (detailed) {
      printf("MKC: Displaying solution =========== part 2 ==============\n\n");
      printf("MKC: The following x variables are at level 1:\n");
   }
   double a_mps = 0.0; // see mkc.mps for a_mps and p_mps
   double p_mps = 0.0;
   for (i = vars.size() - 1; i >= 0; --i) {
      MKC_var* var = vars[i];
      const MKC_knapsack& ks = kss.ks_list[var->entries[0]];
      const int * entries = var->entries;
      for (j = 1; j < var->entry_num; ++j) {
	 if (detailed)
	    if (j % 8 == 0)
	       printf("\n");
	 const int index = entries[j] - kss.ks_num;
	 for (loc = 0; loc < ks.size; ++loc)
	    if (ks.entries[loc].index == index)
	       break;
	 if (loc == ks.size)
	    abort();
	 if (detailed) {
	    const char * name = kss.orig_name_list[ks.entries[loc].orig_index];
	    printf("%9s", name);
	 }
	 a_mps += kss.orig_row_0[ks.entries[loc].orig_index];
      }
      p_mps += kss.orig_row_1[ks.orig_index];
      if (detailed)
	 printf("\n");
   }
   if (detailed)
      printf("\n");

   printf("MKC: Displaying solution =========== part 3 ==============\n\n");
   printf("     The objective value is: %f\n",
	  sol->objective_value());
   printf("     The total weight of the satisfied orders is: %f\n",
	  a_mps);
   printf("     The total unused weight of the used slabs is: %f\n",
	  p_mps - a_mps);
   printf("\n");
   printf("MKC: End of solution =====================================\n\n");
}

//#############################################################################

void
MKC_tm::init_new_phase(int phase,
		       BCP_column_generation& colgen,
		       CoinSearchTreeBase*& candidates)
{
   colgen = BCP_GenerateColumns;
}
