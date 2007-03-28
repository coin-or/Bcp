// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_LP_PARAM_H
#define _CSP_LP_PARAM_H

enum {
   CSP_UseSimplex,
   CSP_UseVolume
};

struct CSP_lp_par{
   /** Fill out whatever parameters we want to use */
   enum chr_params {
      chr_dummy,
      //
      end_of_chr_params
   };
   enum int_params{
      HeurIpFrequency,
      HeurIpMaxTreeSize,
      LpSolver,
      LpSolverMessageSuppression,
      BranchingStrategy,
      PerturbNum,

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
      // These are parameters passed to the volume algorithm
      PerturbFactor,

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
      str_array_dummy,
      //
      end_of_str_array_params
   };
};

#endif
