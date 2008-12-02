// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_LP_PARAM_H
#define _MKC_LP_PARAM_H

struct MKC_lp_par{
   enum chr_params{
      CheckForTailoff,
      ComputeLpLowerBound,
      DoLogicalFixing,
      HeurFixByReducedCost,

      AddAllGeneratedVars,
      ExactFallbackAtVargen,
      ExactFallbackAtLowerBd,
      ParallelVargenForAllKS,
      PrintBestDj,

      WriteToVarFile,

      UseVolume,
      UseClp,
      //
      end_of_chr_params
   };
   enum int_params{
      LpSolverMessageSuppression,
      MaxEnumeratedSize,
      TailoffLength,

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
      LowerBound,
      TailoffIncrease,

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
      OutputVarFile,
      //
      end_of_str_params
   };
   enum str_array_params{
      str_array_dummy,
      //
      end_of_str_array_params
   };
};

#endif
