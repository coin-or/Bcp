// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cfloat>
#include <fstream>
#include <algorithm>

#include "CoinMpsIO.hpp"

#include "MKC_knapsack.hpp"

#include "MKC_init.hpp"
#include "MKC_tm.hpp"
#include "MKC_lp.hpp"
#include "MKC_var.hpp"

//#############################################################################

USER_initialize *
BCP_user_init()
{
   return new MKC_initialize;
}

//#############################################################################
//#############################################################################

static inline bool
MKC_ks_entry_color_comp(const MKC_knapsack_entry& order0,
			const MKC_knapsack_entry& order1)
{
   return order0.color < order1.color;
}

//#############################################################################

void
MKC_read_mps_file(MKC_tm& mkc);
void
MKC_read_var_file(const MKC_knapsack_set& kss, const char * fname,
		  BCP_vec<MKC_var*>& vars);

//#############################################################################
//#############################################################################

BCP_lp_user *
MKC_initialize::lp_init(BCP_lp_prob& p)
{
   return new MKC_lp;
}

//#############################################################################

BCP_tm_user *
MKC_initialize::tm_init(BCP_tm_prob& p,
			const int argnum, const char * const * arglist)
{
   const char * paramfile = arglist[1];
   MKC_tm* mkc = new MKC_tm;

   mkc->tm_par.read_from_file(paramfile);
   mkc->lp_par.read_from_file(paramfile);

   MKC_read_mps_file(*mkc);

   if (mkc->tm_par.entry(MKC_tm_par::ReadFromVarFile))
      MKC_read_var_file(mkc->kss,
			mkc->tm_par.entry(MKC_tm_par::InputVarFile).c_str(),
			mkc->input_vars);
   if (mkc->tm_par.entry(MKC_tm_par::DeleteOrdersNotInVarFile)) {
      int i, j, k;
      MKC_knapsack_set& kss = mkc->kss;
      const int ks_num = kss.ks_num;
      const int order_num = kss.order_num;
      char * flag = new char[order_num];
      memset(flag, false, order_num);
      for (i = mkc->input_vars.size() - 1; i >= 0; --i) {
	 const MKC_var* var = mkc->input_vars[i];
	 const int* var_entries = var->entries;
	 for (j = 1; j < var->entry_num; ++j)
	    flag[var_entries[j] - ks_num] = true;
      }
      for (k = 0, i = 0; i < order_num; ++i) {
	 if (flag[i])
	    ++k;
      }
      printf("\nMKC: The input variables cover %i orders out of %i.\n\n",
	     k, order_num);
      for (i = 0; i < ks_num; ++i) {
	 MKC_knapsack& ks = kss.ks_list[i];
	 MKC_knapsack_entry* entries = ks.entries;
	 for (k = 0, j = 0; j < ks.size; ++j) {
	    if (flag[entries[j].index])
	       entries[k++] = entries[j];
	 }
	 printf("MKC: knapsack %i has %i entries left out of %i",
		i, k, ks.size);
	 ks.size = k;
	 delete[] ks.color_beg;
	 int color_num = ks.color_num;
	 if (ks.size > 0) {
	    ks.color_beg = new int[ks.color_num + 1]; // an upper bound
	    ks.color_beg[0] = 0;
	    for (k = 0, j = 1; j < ks.size; ++j) {
	       if (entries[j - 1].color != entries[j].color)
		  ks.color_beg[++k] = j;
	    }
	    ks.color_beg[++k] = ks.size;
	    ks.color_num = k;
	    for (j = 0; j < ks.color_num; ++j) {
	       for (k = ks.color_beg[j]; k < ks.color_beg[j+1]; ++k)
		  entries[k].color = j;
	    }
	 } else {
	    ks.color_beg = 0;
	    ks.color_num = 0;
	 }
	 printf(" (%i colors out of %i).\n", ks.color_num, color_num);
      }
   }
   return mkc;
}

//#############################################################################
//#############################################################################

//#############################################################################

static inline bool
MKC_var_comp(const MKC_var * var0, const MKC_var * var1)
{
   return
      std::lexicographical_compare(var0->entries,
				   var0->entries + var0->entry_num,
				   var1->entries,
				   var1->entries + var1->entry_num);
}

static inline bool
MKC_var_equal(const MKC_var * var0, const MKC_var * var1)
{
   return (var0->entry_num == var1->entry_num &&
	   std::equal(var0->entries, var0->entries + var0->entry_num,
		      var1->entries));
}

//#############################################################################

void
MKC_read_mps_file(MKC_tm& mkc)
{
   MKC_knapsack_set& kss = mkc.kss;

   // read in the mps file and store it in mkc.matrix
   mkc.clp = new OsiClpSolverInterface;

   CoinMpsIO mpsio;
   mpsio.readMps(mkc.tm_par.entry(MKC_tm_par::InputFile).c_str(), "");

   const CoinPackedMatrix* rowMatrix = mpsio.getMatrixByRow();
   const CoinPackedMatrix* colMatrix = mpsio.getMatrixByCol();

   const int nrow = mpsio.getNumRows();
   const int ncol = mpsio.getNumCols();
   const double * objective = mpsio.getObjCoefficients();
   const double * upper = mpsio.getRowUpper();

   const double * elemrc = rowMatrix->getElements();
   const int * rowrc = rowMatrix->getVectorStarts();
   const int * colrc = rowMatrix->getIndices();

   const int * colcc = colMatrix->getVectorStarts();
   const int * rowcc = colMatrix->getIndices();

   int M = 0;
   int i, j, k, irow, ii, jj;

   // copy over the original names to be used in the final solution display
   kss.orig_var_num = ncol;
   kss.orig_name_list = new char*[ncol];
   for (i = 0; i < ncol; ++i) {
      const int len = strlen(mpsio.columnName(i));
      kss.orig_name_list[i] = new char[len+1];
      strcpy(kss.orig_name_list[i], mpsio.columnName(i));
   }

   /* type will be 3 for keep in final model */
   /* 2 for knapsack */
   /* 1 for y<=2 */
   /* 0 for x<=y */
   int * type = new int[nrow];
   for (i=0; i<nrow; i++) {
      type[i] = 3;
   }
   type[0] = 4; // the first two, the phony rows
   type[1] = 4;

   /* find out how many Knapsacks and set the types of the rows */
   for (i=0; i<ncol; ++i) {
      if (mpsio.columnName(i)[0] == 'z') {
	 const int len = colMatrix->getVectorSize(i);
	 for (j=colcc[i]; j<colcc[i]+len; ++j) {
	    irow = rowcc[j];
	    if (irow>1) { /* knapsack row */
	       type[irow]=2;
	       break;
	    }
	 }
	 M++;
      }
   }
   int order_num = 0;
   for (i=0; i<nrow; ++i) {
      if (type[i] != 2 && type[i] != 4) {
	 if (upper[i] == 0) { // x <= y
	    type[i] = 0;
	 } else if (upper[i] == 1) { // \sum x <= 1
	    type[i] = 3;
	    ++order_num;
	 } else { // must be y <= 2
	    type[i] = 1;
	 }
      }
   }

   // for each variable decide its order index
   int * order_index = new int[ncol]; // more than needed
   k = 0;
   for (i=0; i<nrow; ++i) {
      if (type[i] == 3) { // \sum x <= 1
	 const int len = rowMatrix->getVectorSize(i);
	 for (j=rowrc[i]; j<rowrc[i]+len; ++j)
	    order_index[colrc[j]] = k;
	 ++k;
      }
   }

   // build up the knapsacks
   int * colorclass = new int[ncol];

   kss.ks_num = M;
   kss.ks_list = new MKC_knapsack[M];
   kss.order_num = order_num;

   M = 0;
   for (i=0; i<nrow; i++) {
      if (type[i] == 2) {
	 MKC_knapsack& ks = kss.ks_list[M++];
	 const int len = rowMatrix->getVectorSize(i);
	 ks.size = len - 1;
	 ks.entries = new MKC_knapsack_entry[ks.size];
	 int nextcolor = 0;
	 for (j = 0; j < ncol; ++j)
	    colorclass[j] = -1;
	 for (k=0, j=rowrc[i]; j<rowrc[i]+len; ++j) {
	    const int icol = colrc[j];
	    switch (mpsio.columnName(icol)[0]) {
	     case 'x':
	       {
		  MKC_knapsack_entry& entry = ks.entries[k++];
		  entry.index = order_index[icol];
		  entry.orig_cost = objective[icol];
		  entry.orig_index = icol;
		  entry.weight = elemrc[j];
		  const int len_ii = colMatrix->getVectorSize(icol);
		  for (ii = colcc[icol]; ii < colcc[icol]+len_ii; ++ii) {
		     const int jrow = rowcc[ii];
		     if (type[jrow] == 0) {
			const int len_jj = rowMatrix->getVectorSize(jrow);
			for (jj=rowrc[jrow]; jj<rowrc[jrow]+len_jj; ++jj) {
			   const int iicol = colrc[jj];
			   if (mpsio.columnName(iicol)[0] == 'y') {
			      if (colorclass[iicol] == -1) {
				 colorclass[iicol] = nextcolor++;
			      }
			      entry.color = colorclass[iicol];
			      break;
			   }
			}
			if (jj == rowrc[jrow+1])
			   abort();
			break;
		     }
		  }
		  if (ii == colcc[icol+1])
		     abort();
	       }
	       break;
	     case 'z':
	       ks.cost = objective[icol];
	       ks.capacity = -elemrc[j];
	       ks.orig_index = icol;
	       break;
	     default:
	       abort();
	    }
	 }
	 ks.color_num = nextcolor;
	 if (k != ks.size)
	    abort();
      }
   }

   delete[] colorclass;

   for (i=0; i<M; ++i) {
      MKC_knapsack& ks = kss.ks_list[i];
      MKC_knapsack_entry* entries = ks.entries;
      std::sort(entries, entries + ks.size, MKC_ks_entry_color_comp);
      ks.color_beg = new int[ks.color_num + 1];
      ks.color_beg[0] = 0;
      for (k = 0, j = 1; j < ks.size; ++j) {
	 if (entries[j - 1].color != entries[j].color)
	    ks.color_beg[++k] = j;
      }
      ks.color_beg[++k] = ks.size;
      if (k != ks.color_num)
	 abort();
      for (k = 0; k < ks.color_num; ++k) {
	 for (int l = ks.color_beg[k]; l < ks.color_beg[k+1]; ++l) {
	    if (entries[l].color != k)
	       abort();
	 }
      }
   }

   kss.orig_row_0 = new double[kss.orig_var_num];
   kss.orig_row_1 = new double[kss.orig_var_num];
   std::fill(kss.orig_row_0, kss.orig_row_0+kss.orig_var_num, 0.0);
   std::fill(kss.orig_row_1, kss.orig_row_1+kss.orig_var_num, 0.0);

   {
      const int len = rowMatrix->getVectorSize(0);
      for (j=rowrc[0]; j<rowrc[0]+len; ++j)
	 kss.orig_row_0[colrc[j]] = elemrc[j];
   }
   {
      const int len = rowMatrix->getVectorSize(1);
      for (j=rowrc[1]; j<rowrc[1]+len; ++j)
	 kss.orig_row_1[colrc[j]] = elemrc[j];
   }

   if (mkc.tm_par.entry(MKC_tm_par::SolveLpForLB) ||
       mkc.tm_par.entry(MKC_tm_par::TestFinalSolution)) {
      mkc.clp->loadProblem(*colMatrix,
			   mpsio.getColLower(), mpsio.getColUpper(),
			   mpsio.getObjCoefficients(),
			   mpsio.getRowLower(), mpsio.getRowUpper());
   }

   if (mkc.tm_par.entry(MKC_tm_par::SolveLpForLB)) {
      mkc.clp->initialSolve();
      mkc.lp_par.set_entry(MKC_lp_par::LowerBound, mkc.clp->getObjValue());
   }

   if (mkc.lp_par.entry(MKC_lp_par::LowerBound) < -DBL_MAX / 2) {
      printf("MKC: Global lower bound is minus infinity.\n");
   } else {
      printf("MKC: Global lower bound is %f.\n",
	     mkc.lp_par.entry(MKC_lp_par::LowerBound));
   }

   // get rid of the model
   if (! mkc.tm_par.entry(MKC_tm_par::TestFinalSolution)) {
      delete mkc.clp;
      mkc.clp = NULL;
   } else {
      // change the lower/upper bound of all x/z variables to 0
      int * ind = new int[ncol];
      int cnt = 0;
      for (i = 0; i < ncol; ++i) {
	 if (mpsio.columnName(i)[0] == 'x' || mpsio.columnName(i)[0] == 'z') {
	    ind[cnt++] = i;
	 }
      }
      if (cnt > 0) {
	 double * bnd = new double[2*cnt];
	 std::fill(bnd, bnd + 2*cnt, 0.0);
	 mkc.clp->setColSetBounds(ind, ind+cnt, bnd);
	 delete[] bnd;
      }
      delete[] ind;
   }

   delete[] order_index;
   delete[] type;
}

//#############################################################################

void
MKC_read_var_file(const MKC_knapsack_set& kss, const char * fname,
		  BCP_vec<MKC_var*>& vars)
{
   std::ifstream infile(fname);

   if (infile == 0)
      throw BCP_fatal_error("MKC: Can't open InputVarFile.\n");

   const int ks_num = kss.ks_num;
   int i, loc;
   int entry_num;
   int * var_entries;
   double cost;
   int clr[2];

   while (infile) {
      infile >> clr[0] >> clr[1] >> entry_num;
      if (entry_num == -1)
	 continue;
      var_entries = new int[entry_num];
      for (i = 0; i < entry_num; ++i)
	 infile >> var_entries[i];
      const MKC_knapsack& ks = kss.ks_list[var_entries[0]];
      for (cost = ks.cost, i = 1; i < entry_num; ++i) {
	 const int index = var_entries[i] - ks_num;
	 for (loc = 0; loc < ks.size; ++loc)
	    if (ks.entries[loc].index == index)
	       break;
	 if (loc == ks.size)
	    abort();
	 cost += ks.entries[loc].orig_cost;
      }
      vars.push_back(new MKC_var(cost, entry_num, var_entries, clr));
   }
   std::sort(vars.begin(), vars.end(), MKC_var_comp);
   int varnum = vars.size();
   for (i = 1, loc = 0; i < varnum; ++i) {
      if (! MKC_var_equal(vars[loc], vars[i])) {
	 vars[++loc] = vars[i];
      } else {
	 delete vars[i];
      }
   }
   vars.erase(vars.entry(loc+1), vars.end());

   infile.close();
}

