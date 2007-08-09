// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_LP_PARAM_H
#define _MC_LP_PARAM_H

enum MC_SPCycleCutGen {
  MC_NeverGenerateSPCycleCuts = 0,
  MC_GenerateSPCycleCutsAsLastResort = 1,
  MC_AlwaysGenerateSPCycleCuts = 2
};

enum MC_MstCycleCutGen {
  MC_NeverGenerateMstCycleCuts = 0,
  MC_GenerateMstCycleCutsAsLastResort = 1,
  MC_AlwaysGenerateMstCycleCuts = 2
};

enum MC_LpSolver {
   MC_UseVol = 0x01,
   MC_UseClp = 0x02
};

struct MC_lp_par {
   enum chr_params{
      DoEdgeSwitchHeur,
      ReportAllSPCycleCuts,

      SwitchToSimplex,
      ExplicitSlacksInOpt,
      OnceOptAlwaysOpt,
      //
      end_of_chr_params
   };
   enum int_params{
      LpSolver,

      MaxDepth,

      MstCycleCutGeneration,
      SPCycleCutGeneration,
     
      StructureSwitchHeur,
      MstHeurNum,
      CycleCutHeurNum,
      MaxCycleCutNum,
      SB_CandidateNum,
      HeurSwitchImproveRound,
      TailoffGapRelMinItcount,
      TailoffLbAbsMinItcount,
      TailoffLbRelMinItcount,

      MaxCutsAddedPerIterVol,
      MaxCutsAddedPerIterSim,
      MaxPresolveIterVol,
      MaxPresolveIterSim,

      Vol_ascentFirstCheck,
      Vol_ascentCheckInterval,
      Vol_printFlag,
      Vol_printInterval,
      Vol_greenTestInterval,
      Vol_yellowTestInterval,
      Vol_redTestInterval,
      Vol_alphaInt,
      Vol_maxSubGradientIterations,
      //
      end_of_int_params
   };
   enum dbl_params{
      IntegerTolerance,

      MinIsingCutViolation,
      MinMstCycleCutViolation,
      MinSPCycleCutViolation,

      MaxPerturbInMstCycleCutGen,
      MaxPerturbInMstHeur,
      TailoffGapRelMinImprovement,
      TailoffLbAbsMinImprovement,
      TailoffLbRelMinImprovement,

      Vol_lambdaInit,
      Vol_alphaInit,
      Vol_alphaFactor,
      Vol_alphaMin,
      Vol_primalAbsPrecision,
      Vol_gapAbsPrecision,
      Vol_gapRelPrecision,
      Vol_granularity,
      Vol_minimumRelAscent,
      //
      end_of_dbl_params
   };
   enum str_params{
      str_dummy,
      //
      end_of_str_params
   };
   enum str_array_params{
      // the dummy is needed so the allocation won't try for 0 entries
      str_array_dummy,
      //
      end_of_str_array_params
   };
};


#endif
