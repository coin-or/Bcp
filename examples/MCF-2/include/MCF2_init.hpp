#ifndef MCF2_init_hpp
#define MCF2_init_hpp

#include "BCP_USER.hpp"

class MCF2_init : public USER_initialize {
public:
  virtual BCP_user_pack* packer_init(BCP_user_class* p);
  virtual BCP_tm_user* tm_init(BCP_tm_prob& p,
			       const int argnum, const char * const * arglist);
  virtual BCP_lp_user* lp_init(BCP_lp_prob& p);
};

#endif
