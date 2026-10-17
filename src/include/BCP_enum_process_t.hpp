// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_ENUM_PROCESS_T_H
#define _BCP_ENUM_PROCESS_T_H

//-----------------------------------------------------------------------------

/** This enumerative constant describes the various process types. */

enum BCP_process_t {
   /** */
   BCP_ProcessType_Any,
   /** */
   BCP_ProcessType_TM,
   /** */
   BCP_ProcessType_TS, // TM storage
   /** */
   BCP_ProcessType_LP,
   /** */
   BCP_ProcessType_CG,
   /** */
   BCP_ProcessType_VG,
   /** */
   BCP_ProcessType_CP,
   /** */
   BCP_ProcessType_VP,
   /** */
   BCP_ProcessType_EndProcess
};

#endif
