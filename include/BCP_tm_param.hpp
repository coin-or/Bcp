// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TM_PARAM_H
#define _BCP_TM_PARAM_H

/** Parameters used in the Tree Manager process. These parameters can be set
    in the original parameter file by including the following line: <br>
    <code>BCP_{parameter name}  {parameter value}</code> */

struct BCP_tm_par{
  /** Character parameters. Most of these variables are used as booleans
      (true = 1, false = 0). */
  enum chr_params{
    /** Indicates whether algorithmic variables will be generated or not.
	Values: 1 (true), 0 (false). Default: 0. */
    AlgorithmicVariablesAreGenerated,
    /** Indicates whether indexed variables will be generated or not. Values:
	1 (true), 0 (false). Default: 0. */
    IndexedVariablesAreGenerated,
    /** Indicates whether to debug LP processes or not. Values: 1 (true), 0
	(false). Default: 0. */
    DebugLpProcesses,
    /** Indicates whether to debug Cut Generator processes or not. Values: 1
	  (true), 0 (false). Default: 0. */
    DebugCgProcesses,
    /** Indicates whether to debug Variable Generator processes or not.
	Values: 1 (true), 0 (false). Default: 0. */
    DebugVgProcesses,
    /** Indicates whether to debug Cut Pool processes or not. Values: 1
	(true), 0 (false). Default: 0. */
    DebugCpProcesses,
    /** Indicates whether to debug Variable Pool processes or not. Values: 1
	(true), 0 (false). Default: 0. */
    DebugVpProcesses,
      /** Indicates whether to do branching or not. Values: 1 (true), 0
	  (false). Default: 0. */
    // *FIXME* name of parameter is not the best. 
    DoBranchAndCut,
    /** Indicator whether the LP computed objective value is a true lower
	bound on the node. This should be set to false if column generation is
	done. Values: 1 (true), 0 (false). Default: 1. */
    LpValueIsTrueLowerBound,
    /** Indicates whether message passing is serial (all processes are on
	the same processor) or not. Values: 1 (true), 0 (false). Default: 0.
    */
    MessagePassingIsSerial,
    /** Indicates whether to do pricing (variable generation) in the root
	before the second phase of the algorithm. Values: 1 (true), 0
	(false). Default: 0. */
    PriceInRootBeforePhase2,
    /** Print out a message when the default version of an overridable
	  method is executed. Default: 1. */
    ReportWhenDefaultIsExecuted,
    /** Indicates whether to trim the search tree before a new phase.
	Values: 1 (true), 0 (false). Default: 0. */
    TrimTreeBeforeNewPhase,
    /** Verbosity flags for the tree manager */
    /*@{*/
      /** Print the value of *any* integer feasible solution found.
	  (BCP_tm_prob::process_message) */
      TmVerb_AllFeasibleSolutionValue,
      /** Invoke the user-written "display_feasible_solution" function if
       *any* feasible solution is found. (BCP_tm_prob::process_message) */
      TmVerb_AllFeasibleSolution,
      /** Print the value of the integer solution when a solution better than
	  the current best solution is found. (BCP_tm_prob::process_message) */
      TmVerb_BetterFeasibleSolutionValue,
      /** Invoke the user-written "display_feasible_solution" function if a
	  better integral feasible solution is found.
	  (BCP_tm_prob::process_message) */
      TmVerb_BetterFeasibleSolution,
      /** Print the value of the best feasible solution found after the entire
	  serach tree is processed. (BCP_tm_wrapup) */
      TmVerb_BestFeasibleSolutionValue,
      /** Invoke "display_feasible_solution" user routine for the best
	  feasible solution after the entire tree is processed.
	  (BCP_tm_wrapup) */
      TmVerb_BestFeasibleSolution,
      /** Print the "Starting phase x" line. (BCP_tm_tasks_before_new_phase) */
      TmVerb_NewPhaseStart,
      /** Print information about nodes that are pruned by bound in the tree
	  manager. These nodes would be returned by the LP if they were sent
	  there, so prune them in the TM instead. (BCP_tm_start_one_node) */
      TmVerb_PrunedNodeInfo,
      /** Print the time (and the solution value and solution if the above
	  paramters are set appropriately) when any/better solution is
	  found. (BCP_tm_prob::process_message) */  
      TmVerb_TimeOfImprovingSolution,
      /** Print the number of nodes trimmed between phases.
	  (BCP_tm_trim_tree_wrapper) */
      TmVerb_TrimmedNum,
      /** Total running time. */
      TmVerb_RunningTime,
      /** Print size and max depth of search tree. */
      TmVerb_LpStatistics,
      /** Print timing information about the lp process(es). */
      TmVerb_TreeStatistics,
      /** Print out a message when the default version of an overridable
	  method is executed. Default: 1. */
      TmVerb_ReportDefault,
    /*@}*/
    //
    end_of_chr_params
  };
   
  /** Integer parameters. */
  enum int_params{
    /** Which search tree enumeration strategy should be used.
	Values: 0 (BCP_BestFirstSearch), 1 (BCP_BreadthFirstSearch),
	2 (BCP_DepthFirstSearch). Default: 0 */
    TreeSearchStrategy,
    /** How resource-hog the processes should be. Interpretation is system
	dependent, and the value is passed directly to the renice() function.
	Usually the bigger the number the less demanding the processes will
	be. */
    NiceLevel,
    /** The number of LP processes that should be spawned. */
    LpProcessNum,
    /** The number of Cut Generator processes that should be spawned. */
    CgProcessNum,
    /** The number of Cut Pool processes that should be spawned. Values: */
    CpProcessNum,
    /** The number of Variable Generator processes that should be spawned. */
    VgProcessNum,
    /** The number of Variable Pool processes that should be spawned. */
    VpProcessNum,
    /** ??? */
    TmTimeout,
    //
    end_of_int_params
  };

  /** Double parameters. */
  enum dbl_params{
    /** The probability with which the LP process is directed to dive.
	Values: Default: */
    UnconditionalDiveProbability,
    /** The LP process is allowed to dive if the ratio between the quality
	(for now the presolved objective value) of the child to be kept and
	the best quality among the candidate nodes is not larger the
	this parameter. This ratio is applied if an upper bound already
	exists. Values: Default: */
    QualityRatioToAllowDiving_HasUB,
    /** Same as above, but this value is used if an upper bound does not
	exist yet. Values: Default: */
    QualityRatioToAllowDiving_NoUB,
    /** ??? Values: Default: */
    Granularity,
    /** Maximum allowed running time */
    MaxRunTime,
    /** ??? Values: Default: */
    TerminationGap_Absolute,
    /** ??? Values: Default: */
    TerminationGap_Relative,
    /** ??? Values: Default: */
    UpperBound,
    //
    end_of_dbl_params
  };
  
  /** String parameters. */
  enum str_params{
    /** The name of the file where those cuts should be saved that were in the
	root node in the 0-th phase at the end of processing the root node. */
    SaveRootCutsTo,
    /** The name of the file where cuts to be added to the root description
	should be read ot from. */
    ReadRootCutsFrom,
    /** The name of the executable that's running (and that should be
	spawned on the other processors. */
    ExecutableName,
    /** ??? */
    LogFileName,
    //
    end_of_str_params
  };

  /** ??? */
  enum str_array_params{
    /** ??? */
    LpMachines,
    /** ??? */
    CgMachines,
    /** ??? */
    VgMachines,
    /** ??? */
    CpMachines,
    /** ??? */
    VpMachines,
    //
    end_of_str_array_params
  };

};

#endif
