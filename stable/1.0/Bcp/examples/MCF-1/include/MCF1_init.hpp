#ifndef MCF1_init_hpp
#define MCF1_init_hpp

#include "BCP_USER.hpp"

class MCF1_init : public USER_initialize {
public:
    virtual BCP_tm_user * tm_init(BCP_tm_prob& p,
				  const int argnum,
				  const char * const * arglist);
    virtual BCP_lp_user * lp_init(BCP_lp_prob& p);
};

#endif
