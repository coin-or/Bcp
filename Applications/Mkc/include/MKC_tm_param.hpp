// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_TM_PARAM_H
#define _MKC_TM_PARAM_H

struct MKC_tm_par{
   enum chr_params{
      DetailedFeasibleSolution,
      SolveLpForLB,
      TestFinalSolution,

      CreateRootFromInputVars,
      DeleteOrdersNotInVarFile,
      ReadFromVarFile,
      //
      end_of_chr_params
   };
   enum int_params{
      int_dummy,
      //
      end_of_int_params
   };
   enum dbl_params{
      StartingDualValue,
      //
      end_of_dbl_params
   };
   enum str_params{
      InputFile,
      InputVarFile,
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
