// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_ENUM_TM_H
#define _BCP_ENUM_TM_H

//-----------------------------------------------------------------------------

enum BCP_tree_search_method {
  BCP_BestFirstSearch,
  BCP_BreadthFirstSearch,
  BCP_DepthFirstSearch
};

//-----------------------------------------------------------------------------

/** This enumerative constant describes ... */

enum BCP_node_start_result{
   /** */
   BCP_NodeStart_NoNode,
   /** */
   BCP_NodeStart_Error,
   /** */
   BCP_NodeStart_OK
};

#endif
