#ifndef MCF2_par_hpp
#define MCF2_par_hpp

// These are the parameters for the MCF2 code
    
class MCF2_par {
public:
    enum chr_params {
	AddDummySourceSinkArcs,
        //
        end_of_chr_params
    };
    enum int_params {
        //
        end_of_int_params
    };
    enum dbl_params {
        //
        end_of_dbl_params
    };
    enum str_params {
        InputFilename,
        //
        end_of_str_params
    };
    enum str_array_params {
        //
        end_of_str_array_params
    };
};

#endif
