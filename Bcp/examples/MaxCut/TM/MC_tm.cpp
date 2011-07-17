// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <fstream>

#include "BCP_tm.hpp"
#include "MC_tm.hpp"
#include "MC_cut.hpp"
#include "MC_init.hpp"

//#############################################################################

int main(int argc, char* argv[])
{
    MC_initialize mc_init;
    return bcp_main(argc, argv, &mc_init);
}

//#############################################################################

void
MC_tm::pack_module_data(BCP_buffer& buf, BCP_process_t ptype)
{
  switch (ptype) {
  case BCP_ProcessType_LP:
    lp_par.pack(buf);
    break;
  default:
    abort();
  }
  mc.pack(buf);
}

//#############################################################################

BCP_solution*
MC_tm::unpack_feasible_solution(BCP_buffer& buf)
{
   MC_solution* new_sol = new MC_solution;
   new_sol->unpack(buf);
   if (new_sol->objective_value() > best_soln.objective_value())
      best_soln = *new_sol;
   return new_sol;
}

//#############################################################################

void
MC_tm::initialize_core(BCP_vec<BCP_var_core*>& vars,
		       BCP_vec<BCP_cut_core*>& cuts,
		       BCP_lp_relax*& matrix)
{
  int i;
  const int m = mc.num_edges;

  vars.reserve(m);
  for (i = 0; i < m; ++i) {
    vars.unchecked_push_back(new BCP_var_core(BCP_BinaryVar,
					      mc.edges[i].cost, 0, 1));
  }
}

//#############################################################################

void
MC_tm::create_root(BCP_vec<BCP_var*>& added_vars,
		   BCP_vec<BCP_cut*>& added_cuts,
		   BCP_user_data*& user_data)
{
  if (added_cuts.size() > 0)
    return;
  // Do a couple of MST based cut generations with 0 as the primal solution
  // and with slight perturbations on the costs.
  int i, j;
  const int n = mc.num_nodes;
  const int m = mc.num_edges;
  double* x = new double[m];

  // Set x to be the optimal solution to the unconstrainted optimization
  // problem
  const MC_graph_edge* edges = mc.edges;
  for (i = 0; i < m; ++i)
    x[i] = edges[i].cost > 0.0 ?  0.0 : 1.0;

  const int improve_round = lp_par.entry(MC_lp_par::HeurSwitchImproveRound);
  const bool edge_switch  = lp_par.entry(MC_lp_par::DoEdgeSwitchHeur);
  const int struct_switch = ( lp_par.entry(MC_lp_par::StructureSwitchHeur) &
			      ((1 << mc.num_structure_type) - 1) );
  MC_solution* sol = NULL;

  BCP_vec<BCP_row*> new_rows;
  if (mc.ising_four_cycles || mc.ising_triangles) {
    const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
    const int grid_nodes = grid*grid;
    if (mc.ising_four_cycles)
      MC_test_ising_four_cycles(grid_nodes, mc.ising_four_cycles,
				x, 0.9, added_cuts, new_rows);
    if (mc.ising_triangles)
      MC_test_ising_triangles(2*grid_nodes, mc.ising_triangles,
			      x, 0.9, added_cuts, new_rows);
    getTmProblemPointer()->ub(0.0);
    BCP_vec<int> sig(n, 1);
    sol = new MC_solution(sig, mc, improve_round, edge_switch, struct_switch);
  } else {
    // Not an Ising problem, use MST cuts to generate initial cuts;
    sol = MC_mst_cutgen(mc, x, NULL, 1.0, 0.0,
			MC_MstEdgeOrderingPreferExtreme,
			improve_round, edge_switch, struct_switch,
			0.9, COIN_INT_MAX, added_cuts, new_rows);
  }

  // update best upper bound
  getTmProblemPointer()->ub(sol->objective_value());
  getTmProblemPointer()->feas_sol = sol;

  delete[] x;

  // get rid of the row representations
  purge_ptr_vector(new_rows);

  // keep unique cuts only (delete the duplicates)
  std::sort(added_cuts.begin(), added_cuts.end(), MC_cycle_cut_less);
  const int cutnum = added_cuts.size();
  for (j = 0, i = 1; i < cutnum; ++i) {
    if (MC_cycle_cut_equal(added_cuts[j], added_cuts[i])) {
      delete added_cuts[i];
      added_cuts[i] = NULL;
    } else {
      j = i;
    }
  }

  // get rid of the NULL pointers
  for (j = 0, i = 0; i < cutnum; ++i) {
    if (added_cuts[i] != NULL) {
      added_cuts[j++] = added_cuts[i];
    }
  }
  added_cuts.erase(added_cuts.entry(j), added_cuts.end());
}

//#############################################################################

void
MC_tm::display_feasible_solution(const BCP_solution* sol)
{
  const MC_solution* mc_sol = dynamic_cast<const MC_solution*>(sol);
  if (! mc_sol) {
    throw BCP_fatal_error("\
MC_tm::display_feasible_solution() invoked with non-MC_solution.\n");
  }

  if (mc.ising_problem) {
    const int grid = static_cast<int>(sqrt(mc.num_nodes + 1.0));
    const int grid_size = grid * grid;
    const BCP_vec<int>& sig = mc_sol->sig;
    double field = 0;
    for (int i = grid_size - 1; i >= 0; --i) {
      field += sig[i];
    }
    if (field < 0)
      field = -field;
    field /= grid_size;
    const double energy =
      (- mc.sum_edge_weight + 2 * mc_sol->objective_value()) /
      (mc.scaling_factor * grid_size);
    printf("MC: field: %.6f   energy: %.6f\n", field, energy);
  }

  if (tm_par.entry(MC_tm_par::DisplaySolutionSignature)) {
    mc_sol->display(tm_par.entry(MC_tm_par::SolutionFile));
  }
}
