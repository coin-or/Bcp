// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _CSP_TM_PARAM_H
#define _CSP_TM_PARAM_H

struct CSP_tm_par{
   enum chr_params{
      combineExclusionConstraints,
      addKnapsackMirConstraints,
      addKnifeMirConstraints,
      //
      end_of_chr_params
   };
   enum int_params{
      int_dummy,
      //
      end_of_int_params
   };
   enum dbl_params{
      dbl_dummy,
      //
      end_of_dbl_params
   };
   enum str_params{
      InputFile,
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
