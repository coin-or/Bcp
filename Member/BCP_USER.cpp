// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BCP_USER.hpp"

//-----------------------------------------------------------------------------

#if defined(BCP_COMM_PROTOCOL_PVM)

#include "BCP_message_pvm.hpp"
USER_initialize::msgenv_init() {
  return new BCP_pvm_environment;
}

#elif defined(BCP_COMM_PROTOCOL_NONE)

#include "BCP_message_single.hpp"
BCP_message_environment * 
USER_initialize::msgenv_init() {
  return new BCP_single_environment;
}

#endif
