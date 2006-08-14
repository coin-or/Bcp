// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "CoinSort.hpp"

#include "BCP_matrix.hpp"

#include "MC_cut.hpp"

static inline void
MC_test_ising_four_cycle(const double sum, const double minviol,
			 const int c0, const int c1, const int c2,const int c3,
			 BCP_vec<BCP_cut*>& cuts, BCP_vec<BCP_row*>& rows)
{
  // sort c0, c1, c2
  int cut[4];
  cut[3] = c3;
  if (c0 < c1) {
    if (c0 < c2) {
      cut[0] = c0;
      if (c1 < c2) {
	cut[1] = c1;
	cut[2] = c2;
      } else {
	cut[1] = c2;
	cut[2] = c1;
      }
    } else {
      cut[0] = c2;
      cut[1] = c0;
      cut[2] = c1;
    }
  } else {
    if (c1 < c2) {
      cut[0] = c1;
      if (c0 < c2) {
	cut[1] = c0;
	cut[2] = c2;
      } else {
	cut[1] = c2;
	cut[2] = c0;
      }
    } else {
      cut[0] = c2;
      cut[1] = c1;
      cut[2] = c0;
    }
  }
      
  if (sum > 2 + minviol) {
    cuts.push_back(new MC_cycle_cut(cut, 3, 4));
    double row[4] = {1.0, 1.0, 1.0, -1.0};
    CoinSort_2(cut, cut+4, row);
    rows.push_back(new BCP_row(cut, cut+4, row, row+4, -1e40, 2));
  } else if (sum < - minviol) {
    std::rotate(cut, cut+3, cut+4);
    cuts.push_back(new MC_cycle_cut(cut, 1, 4));
    double row[4] = {1.0, -1.0, -1.0, -1.0};
    CoinSort_2(cut, cut+4, row);
    rows.push_back(new BCP_row(cut, cut+4, row, row+4, -1e40, 0));
  }
}

void
MC_test_ising_four_cycles(const int n, const int* cycles,
			  const double* x, const double minviol,
			  BCP_vec<BCP_cut*>& cuts, BCP_vec<BCP_row*>& rows)
{
  const int* cycle = cycles;
  for (int i = 0; i < n; ++i) {
    const int c0 = cycle[0];
    const int c1 = cycle[1];
    const int c2 = cycle[2];
    const int c3 = cycle[3];
    const double x0 = x[c0];
    const double x1 = x[c1];
    const double x2 = x[c2];
    const double x3 = x[c3];

    MC_test_ising_four_cycle(x0+x1+x2-x3, minviol, c0, c1, c2, c3, cuts, rows);
    MC_test_ising_four_cycle(x1+x2+x3-x0, minviol, c1, c2, c3, c0, cuts, rows);
    MC_test_ising_four_cycle(x2+x3+x0-x1, minviol, c2, c3, c0, c1, cuts, rows);
    MC_test_ising_four_cycle(x3+x0+x1-x2, minviol, c3, c0, c1, c2, cuts, rows);
      
    cycle += 4;
  }
}

//#############################################################################

void
MC_test_ising_triangles(const int n, const int* cycles,
			const double* x, const double minviol,
			BCP_vec<BCP_cut*>& cuts, BCP_vec<BCP_row*>& rows)
{
  const int* cycle = cycles;
  int cut[3];
  for (int i = 0; i < n; ++i) {
    const int c0 = cycle[0];
    const int c1 = cycle[1];
    const int c2 = cycle[2];
    const double x0 = x[c0];
    const double x1 = x[c1];
    const double x2 = x[c2];

    if (x0+x1+x2 > 2 + minviol) {
      cuts.push_back(new MC_cycle_cut(cycle, 3, 3));
      double row[3] = {1.0, 1.0, 1.0};
      rows.push_back(new BCP_row(cycle, cycle+3, row, row+3, -1e40, 2.0));
    }
    if (+ x0 - x1 - x2 > minviol) {
      cuts.push_back(new MC_cycle_cut(cycle, 1, 3));
      double row[3] = {1.0, -1.0, -1.0};
      rows.push_back(new BCP_row(cycle, cycle+3, row, row+3, -1e40, 0.0));
    }
    if (- x0 + x1 - x2 > minviol) {
      cut[0] = c1;
      cut[1] = c0;
      cut[2] = c2;
      cuts.push_back(new MC_cycle_cut(cut, 1, 3));
      double row[3] = {-1.0, 1.0, -1.0};
      rows.push_back(new BCP_row(cycle, cycle+3, row, row+3, -1e40, 0.0));
    }
    if (- x0 - x1 + x2 > minviol) {
      cut[0] = c2;
      cut[1] = c0;
      cut[2] = c1;
      cuts.push_back(new MC_cycle_cut(cut, 1, 3));
      double row[3] = {-1.0, -1.0, 1.0};
      rows.push_back(new BCP_row(cycle, cycle+3, row, row+3, -1e40, 0.0));
    }

    cycle += 3;
  }
}
