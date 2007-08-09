// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_PARAM_H
#define _BCP_LP_PARAM_H

// This file is fully docified.

//-----------------------------------------------------------------------------

/** Parameters used in the LP process. These parameters can be set
    in the original parameter file by including the following line: <br>
    <code>BCP_{parameter name}  {parameter value}</code>. */

struct BCP_lp_par{
   /** Character parameters. All of these variables are used as booleans
       (true = 1, false = 0). */
   enum chr_params{
      /** If true, BCP supports branching on cuts by providing potential
	  branching candidates for the user. These are cuts that were added to
	  the formulation at some point but became slack later in
	  subsequent LP relaxations. <br>
	  Values: true (1), false (0). Default: 0. */ 
      BranchOnCuts,
      /** If true then the LP process will check each newly received cut
	  whether it already exists in the local cut pool or not. <br>
	  Values: true (1), false (0). Default: 1. */
      CompareNewCutsToOldOnes,
      /** If true then the LP process will check each newly arrived variable
	  whether it already exists in the local variable pool or not. <br> 
	  Values: true (1), false (0). Default: 1. */
      CompareNewVarsToOldOnes,
      /** If true the BCP will attempt to do reduced cost fixing only for
	  variables currently at zero. <br>
	  Values: true (1), false (0). Default: 1. */
      DoReducedCostFixingAtZero,
      /** If true the BCP will attempt to do reduced cost fixing for any
	  variable, no matter what is their current value. <br>
	  Values: true (1), false (0). Default: 1. */
      DoReducedCostFixingAtAnything,
      /** Indicate whether BCP is supposed to track the indexed variables yet
	  to be priced out. */
      MaintainIndexedVarPricingList,
      /** Indicates whether message passing is serial (all processes are on
	  the same processor) or not. <br>
	  Values: true (1), false (0). Default: 1. */
      MessagePassingIsSerial,
      /** Print out a message when the default version of an overridable
	  method is executed. Default: 1. */
      ReportWhenDefaultIsExecuted,
      /** Whether to send back the description of fathomed search tree nodes
	  to the Tree Manager. (Might be needed to write proof of optimality.)
	  <br>
	  Values: true (1), false (0). Default: 1. */
      SendFathomedNodeDesc,
      /** Whether we should refrain from compressing the problem description
	  right before a fathomed node's description is sent back to the tree
	  manager. <br>
	  Values: true (1), false (0). Default: 0. */
      NoCompressionAtFathom,

      /**@name Verbosity Parameters.

	 These are all true/false parameters; if the parameter value is set
	 then the corresponding information is printed. */
      /*@{*/
        /** Just a marker for the first LpVerb */
        LpVerb_First,
        /** Print the number of cuts added from the local cut pool in the
	    current iteration. (BCP_lp_main_loop)*/
        LpVerb_AddedCutCount,
        /** Print the number of variables added from the local variable pool
	    in the curent iteration. (BCP_lp_main_loop) */
        LpVerb_AddedVarCount,
        /** After a branching object is selected print what happens to the
	    presolved children (e.g., fathomed). (BCP_print_brobj_stat) */
        LpVerb_ChildrenInfo,
        /** Print the number of variables generated before resolving the Lp ir
	    fathoming a node. (BCP_lp_fathom) */
        LpVerb_ColumnGenerationInfo,
        /** Print the number of cuts sent from the LP to the cut pool.
	    (BCP_lp_send_cuts_to_cp) */
        LpVerb_CutsToCutPoolCount,
        /** **** This parameter is not used anywhere!!! **** */
        LpVerb_VarsToVarPoolCount,
        /** Print information related to fathoming. (BCP_lp_main_loop,
	    BCP_lp_perform_fathom, BCP_lp_branch)
	 (BCP_lp_fathom) */
        LpVerb_FathomInfo,
        /** Print the "Starting iteration x" line. (BCP_lp_main_loop)
	 */
        LpVerb_IterationCount,
        /** Turn on the user hook "display_lp_solution". (BCP_lp_main_loop) */
        LpVerb_RelaxedSolution,
        /** Turn on the user hook "display_lp_solution" for the last LP
	    relaxation solved at a search tree node. (BCP_lp_main_loop) */
        LpVerb_FinalRelaxedSolution,
        /** **** This parameter is not used anywhere!!! **** */
        LpVerb_LpMatrixSize,
        /** Print the size of the problem matrix and the LP solution value
	    after resolving the LP. (BCP_lp_main_loop) */
        LpVerb_LpSolutionValue,
        /** Print the number of columns and rows that were deleted during
	    matrix compression. (BCP_lp_delete_cols_and_rows) */
        LpVerb_MatrixCompression,
	    /** For each tree node print out how much time was spent on it */
	    LpVerb_NodeTime,
        /** Print detailed information about all the branching candidates
	    during strong branching. LpVerb_PresolveResult must be set for
	    this parameter to have an effect.
	    (BCP_lp_perform_strong_branching) */ 
        LpVerb_PresolvePositions,
        /** Print information on the presolved branching candidates during
	    strong branching. (BCP_lp_perform_strong_branching) */
        LpVerb_PresolveResult,
        /** Print the "Processing NODE x on LEVEL y" line. (BCP_lp-main_loop)
	 */
        LpVerb_ProcessedNodeIndex,
        /** Print information if receiving cuts is timed out.
	    (BCP_lp_generate_cuts) */
        LpVerb_ReportCutGenTimeout,
        /** Print information if receiving variables is timed out.
	    (BCP_lp_generate_vars) */
        LpVerb_ReportVarGenTimeout,
        /** Print the current number of cuts in the cut pool. This number is
	    printed several times: before and after generating columns at the
	    current iteration, after removing non-essential cuts, etc.
	    (BCP_lp_generate_cuts) */
        LpVerb_ReportLocalCutPoolSize,
        /** Similar as above for variables. (BCP_lp_generate_vars) */
        LpVerb_ReportLocalVarPoolSize,
        /** **** This parameter is not used anywhere!!! **** */
        LpVerb_RepricingResult,
        /** Print the number of variables whose bounds have been changed by
	    reduced cost fixing or logical fixing. (BCP_lp_fix_vars) */
        LpVerb_VarTightening,
        /** Print the number of ineffective rows in the current problem. The
	    definition of what rows are considered ineffective is determined
	    by the paramter IneffectiveConstraints.
	    (BCP_lp_adjust_row_effectiveness) */
        LpVerb_RowEffectivenessCount,
        /** Print detailed information on the branching candidate selected by
	    strong branching. LpVerb_StrongBranchResult must be set fo this
	    parameter to have an effect. (BCP_print_brobj_stat) */ 
        LpVerb_StrongBranchPositions,
        /** Print information on the branching candidate selected by strong
	    branching. (BCP_print_brobj_stat) */
        LpVerb_StrongBranchResult,
        /** Print the number of cuts generated during this iteration (since
	    the LP was resolved last time). (BCP_lp_main_loop) */
        LpVerb_GeneratedCutCount,
        /** Print the number of variables generated during this iteration.
	    (BCP_lp_main_loop) */
        LpVerb_GeneratedVarCount,
        /** Just a marker for the last LpVerb */
        LpVerb_Last,
      /*@}*/ 
      //
      end_of_chr_params
   };

   /** Integer parameters. */
   enum int_params{
      /** What should be the "niceness" of the LP process. In a *nix
	  environment the LP process will be reniced to this level. <br>
	  Values: whatever the operating system accepts. Default: 0. */
      NiceLevel,

      /** Indicates how matrix scaling should be performed. This parameter is
	  directly passed to the LP solver's <code>load_lp</code> member
	  function. <br>
	  Values: whatever is valid for the LP solver that the user has passed
	  on to BCP. Default: 0. */
      ScaleMatrix,
      /** Indicates the number of entries stored in a
	  <code>BCP_indexed_pricing_list</code>. <br>
	  Values: any positive. Default: 1000. */
      IndexedToPriceStorageSize,

      /** The slack cut discarding strategy used in the default version of the
	  function <code>purge_slack_pool()</code>. <br> 
	  Values: \link BCP_slack_cut_discarding.<br>
	  Default: <code>BCP_DiscardSlackCutsAtNewIteration</code> */
      SlackCutDiscardingStrategy,
      /** A cut has to remain effective through this many iterations in the LP
	  before it is sent to the Cut Pool process. <br>
	  The default 1000 effectively says that only those cuts are sent to
	  the pool which are effective at branching. <br>
	  Values: any positive number. Default: 1000. */
      CutEffectiveCountBeforePool,
      /** The Cut Pool is queried for violated valid inequalities after the
	  first LP relaxation is solved and then after every this many
	  iterations. <br>
	  Values: any positive number. Default: 10. */
      CutPoolCheckFrequency,
      /** The Variable Pool is queried for columns that improve the
	  formulation after the first LP realxation is solved, and then after
	  every this many iterations. <br>
	  Values: any positive number. Default: 10. */
      VarPoolCheckFrequency,
      /** Indicates which constraints should be considered ineffective. <br>
	  Values: \link BCP_IneffectiveConstraints. <br>
	  Default: <code>BCP_IneffConstr_ZeroDualValue</code>. */
      IneffectiveConstraints,
      /** How many times in a row a constraint must be found ineffective
	  before it is marked for deletion. <br>
	  Values: any positive number. Default: 1. */
      IneffectiveBeforeDelete,

      /** The number of non dual-feasible colums that can be added at a time
	  to the formulation is a certain fraction
	  (<code>MaxNonDualFeasToAdd_Frac</code>) of the number of columns in
	  the current formulation. However, if the computed number is outside
	  of the range
	  <code>[MaxNonDualFeasToAdd_Min, MaxNonDualFeasToAdd_Max]</code> then
	  it will be set to the appropriate endpoint of the range. <br>
	  Values: . Default: 5. */
      MaxNonDualFeasToAdd_Min,
      /** See the description of the previous parameter. <br>
	  Values: . Default: 200. */
      MaxNonDualFeasToAdd_Max,
      /** When the tableau is total dual feasible some of variables from
	  the list of the not yet fixed indexed variables are added to the
	  formulation so that the list can be shorter in the descendant nodes
	  (or may become empty). These parameters play the same role in
	  limiting the number of such variables as the two previous parameters
	  for the non dual feasible variables. <br>
	  Values: . Default: 5. */
      MaxIndexedToPriceToAdd_Min,
      /** See the description of the previous parameter. <br>
	  Values: . Default: 200. */
      MaxIndexedToPriceToAdd_Max,

      /** How cut violation should be computed.<br>
	  Values: \link BCP_CutViolationNorm. <br>
	  Default: <code>BCP_CutViolationNorm_Plain</code> (violation in the
	  usual sense). <br>
	  (The other option is <code>BCP_CutViolationNorm_Distance</code>: the
	  distance of the fractional point from the cut. */
      CutViolationNorm,

      /** The maximum number of violated valid inequalities that can be added
	  per iteration. <br>
	  Values: . Default: 100,000. */
      MaxCutsAddedPerIteration,
      /** The maximum number of variables that can be added per iteration.
	  <br>
	  Values: . Default: 100,000. */
      MaxVarsAddedPerIteration,

      /** The maximum number of violated but not added cuts to be kept from
	  one iteration to the next. Also see the MaxLeftoverCutFrac
	  parameter.*/
      MaxLeftoverCutNum,

      /** The number of columns that must be marked for deletion before
	  matrix compression can occur. Matrix compressions also subject to a
	  minimum number of marked columns as a fraction of the current number
	  of columns (see <code>DeletedColToCompress_Frac</code>). <br>
	  Values: positive integer. Default: 10. */
      DeletedColToCompress_Min,
      /** The number of rows that must be marked for deletion before
	  matrix compression can occur. Matrix compressionis also subject to a
	  minimum number of marked columns as a fraction of the current number
	  of columns (see <code>DeletedRowToCompress_Frac</code>). <br>
	  Values: positive integer. Default: 10. */
      DeletedRowToCompress_Min,

      /** Upper limit on the number of iterations performed in each of the
	  children of the search tree node when presolving branching
	  candidates. This parameter is passed onto the LP solver. If the
	  parameter is set to -1 then the branching candidates are not
	  presolved and the first branching candidate is chosen (if there is
	  any).<br>
	  Note that for different LP solvers (e.g., simplex based algorithm or
	  the Volume Algorithm) the meaning of an iteration is different, thus
	  this parameter will be set differently. <br>
	  Values: . Default: 100,000. */
      MaxPresolveIter,
      /** Specifies how many branching variables with values close to half
	  between two integers should be chosen by the built-in branching
	  variable selection routine
	  <code>select_branching_candidates()</code> (if this routine is used
	  at all). This number is passed onto the
	  <code>branch_close_to_half()</code> helper function. <br> 
	  Values: . Default: 3. */
      StrongBranch_CloseToHalfNum,
      /** Specifies how many branching variables with values close to an
	  integer should be chosen by the built-in branching
	  variable selection routine
	  <code>select_branching_candidates()</code> (if this routine is used
	  at all). This number is passed onto the
	  <code>branch_close_to_one()</code> helper function. <br> 
	  Values: . Default: 3. */
      // *FIXME* change name to close to integer (change fn name as well!)
       StrongBranch_CloseToOneNum,
      /** Specifies the rule used for built-in branching object comparison (if
	  the buit-in routine is used at all). <br>
	  Values: \link BCP_branching_object_comparison.<br>
	  Default: <code>BCP_HighestLowObjval</code>. */
      BranchingObjectComparison,
      /** Specifies the rule used for selecting one of the children of the
	  search tree node for diving. <br>
	  Values: \link BCP_child_preference. <br>
	  Default: <code>BCP_PreferChild_LowBound</code>. */
      ChildPreference,

      /** Specifies which built-in MIP feasibility testing routine should be
	  invoked (if a buit-in routine is used at all). <br>
	  Values: \link BCP_feasibility_test.<br>
	  Default: <code>BCP_FullTest_Feasible</code> */
      FeasibilityTest,
      /** Indicates what part of the primal solution is sent to the Cut
	  Generator process if the BCP_lp_user::pack_primal_solution() method
	  is not overridden.<br>
	  Values: See \link BCP_primal_solution_description.<br>
	  Default: <code>BCP_PrimalSolution_Nonzeros</code>. */
      InfoForCG,
      /** Indicates what part of the dual solution is sent to the Variable
	  Generator process if the BCP_lp_user::pack_dual_solution() method
	  is not overridden.<br>
	  Values: See \link BCP_dual_solution_description.<br>
	  Default: <code>BCP_DualSolution_Full</code>. */
      InfoForVG,
      //
      end_of_int_params
   };

   /** Double parameters. */
   enum dbl_params{
      /** The minimum difference between the objective value of any two
	  feasible solution (with different objective values). <br>
	  Values: . Default: .*/
      Granularity,
      /** The fraction of columns that must be marked for deletion before
	  matrix compression can occur. Matrix compression is also subject to
	  a minimum number of marked columns (see
	  <code>DeletedColToCompress_Min</code>). <br> 
	  Values: . Default: <code>1e-8</code>.*/
      DeletedColToCompress_Frac,
      /** The fraction of rows that must be marked for deletion before
	  matrix compression can occur. Matrix compression is also subject to
	  a minimum number of marked rows (see
	  <code>DeletedRowToCompress_Min</code>). <br> 
	  Values: . Default: <code>1e-8</code>.*/
      DeletedRowToCompress_Frac,
      /** The number of non dual-feasible colums that can be added to the
	  formulation at a time cannot exceed this fraction of the the number
	  of columns in the curent formulation. However, this limit can be
	  overruled by the hard bounds 
	  <code>MaxIndexedToPriceToAdd_Min</code> and
	  <code>MaxIndexedToPriceToAdd_Max</code> (see above). <br>
	  Values: . Default: 0.05.*/
      MaxNonDualFeasToAdd_Frac,
      /** When the tableau is total dual feasible and variables from the list
	  of the not yet fixed indexed variables are added to the formulation,
	  the number of such columns is limited to this fraction of the
	  columns currently in the formulation. However, this limit might be
	  overruled by the hard bounds 
	  <code>MaxIndexedToPriceToAdd_Min</code> and
	  <code>MaxIndexedToPriceToAdd_Max</code> (see above). <br>
	  Values: . Default: 0.05. */
      MaxIndexedToPriceToAdd_Frac,
      /** The maximum fraction of the violated but not added cuts to be kept
	  from one iteration to the next. Also see the MaxLeftoverCutNum
	  parameter.*/
      MaxLeftoverCutFrac,
      /** Values not further from an integer value than the value of this
	  parameter are considered to be integer. <br>
	  Values: . Default: .*/
      IntegerTolerance,

      /** This and the following three parameters control how long the LP
	  process waits for generated cuts. The parameters specify waiting
	  times (in seconds) separately for the first and later LP
	  relaxations at a search tree node, and also the time to receive the
	  first cut vs all the cuts. <br>
	  Note that it might make sense to set <code>AllCutsTimeout</code> for
	  a <em>shorter</em> time than the first cut time out: "We are willing
	  to wait 5 seconds to receive a cut, but if we do receive a cut the
	  total time we wait is only 2 seconds." <br>
	  This parameter specifies the time to wait for the first generated
	  cut at the first LP relaxation at a search tree node. <br>
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      FirstLP_FirstCutTimeout,
      /** This parameter specifies the time to wait for the first generated
	  cut at iterations that are not the first at a search tree node. See
	  the remarks at <code>FirstLP_FirstCutTimeout</code> as well. <br>
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      LaterLP_FirstCutTimeout,
      /** This parameter specifies the time to wait for cuts at the first LP
	  relaxation at a search tree node. See the remarks at <code>
	  FirstLP_FirstCutTimeout</code> as well. <br> 
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      FirstLP_AllCutsTimeout,
      /** This parameter specifies the time to wait for cuts at iterations
	  that are not the first at a search tree node. See the remarks at
	  <code>FirstLP_FirstCutTimeout</code> as well. <br> 
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      LaterLP_AllCutsTimeout,

      /** This and the following three parameters control how long the LP
	  process waits for generated variables. These parameters are
	  analogous to the ones that control the waiting time for generated
	  cuts. See the remarks at <code>FirstLP_FirstCutTimeout</code> as
	  well. <br> 
	  This parameter specifies the time to wait for the first generated
	  variable at the first LP relaxation at a search tree node. <br> 
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      FirstLP_FirstVarTimeout,
      /** This parameter specifies the time to wait for the first generated
	  variable at iterations that are not the first at a search tree node.
	  See the remarks at <code>FirstLP_FirstVarTimeout</code> as well. <br>
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      LaterLP_FirstVarTimeout,
      /** This parameter specifies the time to wait for variables at the first
	  LP relaxation at a search tree node. See the remarks at <code>
	  FirstLP_FirstVarTimeout</code> as well. <br> 
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      FirstLP_AllVarsTimeout,
      /** This parameter specifies the time to wait for variables at iterations
	  that are not the first at a search tree node. See the remarks at
	  <code>FirstLP_FirstVarTimeout</code> as well. <br> 
	  Values: Any non-negative number represents a waiting time in
	  seconds. Negative numbers indicate no time limit on
	  waiting. Default: -1. */
      LaterLP_AllVarsTimeout,

      //
      end_of_dbl_params
   };

   /** String parameters. */
   enum str_params{
      /** The filename all the output should go to. */
      LogFileName,
      //
      end_of_str_params
   };

   /** There are no string array parameters. */
   enum str_array_params{
      // the dummy is needed so the allocation won't try for 0 entries
      str_array_dummy,
      //
      end_of_str_array_params
   };

};

#endif
