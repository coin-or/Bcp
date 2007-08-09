// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include "BcpConfig.h"

#include "BCP_error.hpp"
bool BCP_fatal_error::abort_on_error = true;

#include "BCP_USER.hpp"
#include "BCP_message_single.hpp"
#include "BCP_message_mpi.hpp"
#include "BCP_message_pvm.hpp"

//-----------------------------------------------------------------------------

BCP_message_environment *
USER_initialize::msgenv_init(int argc, char* argv[]) {
    int procid = -1; // assume no parallel environment
#if defined(COIN_HAS_MPI)
    procid = BCP_mpi_environment::is_mpi(argc, argv);
    if (procid >= 0) {
	return new BCP_mpi_environment(argc, argv);
    }
#endif
#if defined(COIN_HAS_PVM)
    procid = BCP_pvm_myid();
    if (procid >= 0) {
	return new BCP_pvm_environment;
    }
#endif
    // procid must still be -1, so execute serial environment
    return new BCP_single_environment;
}
