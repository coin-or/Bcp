// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MC_TM_PARAM_H
#define _MC_TM_PARAM_H

struct MC_tm_par {
  enum chr_params{
    DisplaySolutionSignature,
    //
    end_of_chr_params
  };
  enum int_params{
    // the dummy is needed so the allocation won't try for 0 entries
    DigitsToLose,
    //
    end_of_int_params
  };
  enum dbl_params{
    dbl_dummy,
    //
    end_of_dbl_params
  };
  enum str_params{
    FeasSolFile,
    InputFile,
    SolutionFile,
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
