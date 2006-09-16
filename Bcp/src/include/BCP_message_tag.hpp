// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MESSAGE_TAG_H
#define _BCP_MESSAGE_TAG_H

// This file is fully docified.

/** This enumerative constant describes the message tags different
    processes of BCP understand. */

enum BCP_message_tag{
  /**@name Messages between the Configurator and the Tree Manager.

     The Configurator is a stand-alone program that is able to communicate
     with the Tree Manager and instruct it to make changes in the machine
     configuration. At the moment there is a configurator for PVM only. */
  /*@{*/
  /** Configurator to TM: machine configuration has changed. TM will
      unpack the changes from the buffer and then apply them. See the possible
      changes in the documentation of the configurator in the Config
      directory. */
   BCP_CONFIG_CHANGE = 1,       // config -> TM
   /** TM to configurator: error occured while TM tried to implement
       the changes communicated by the configurator. */
   BCP_CONFIG_ERROR = 2,        // TM ->config
   /** TM to configurator: TM successfully finished implementing the
       changes communicated by the configurator. */
   BCP_CONFIG_OK = 3,           // TM ->config
   /** Configurator to all processes: are you the Tree Manager? */
   BCP_ARE_YOU_TREEMANAGER = 4, // config -> all 
   /** TM to configurator: I am the Tree Manager. */
   BCP_I_AM_TREEMANAGER = 5,    // TM ->config
   /*@}*/

   /**@name Messages from any process to any other process
      These are the <em>wildcards</em>. */
   /*@{*/
   /** Used to indicate that there is no message in the buffer of a process.
       See \URL[<code>BCP_buffer</code>]{BCP_buffer.html}.) */
   BCP_Msg_NoMessage,
   /** Used when receiving, message with any message tag will be received. */
   BCP_Msg_AnyMessage,
   /** Used by the user to send a message to the user portion of the other
       process. Note that the other process will NOT be interrupted to process
       the message. Message processing will happen when that process decides
       to check for messages on its own. */
   BCP_Msg_User,
   /*@}*/

   /**@name Messages between any process and the Tree Manager */
   /*@{*/
   /** Any process to TM: a process has died. */
   BCP_Msg_SomethingDied, 
   /** Any process to TM or TM to any process: a new upper bound found. */
   BCP_Msg_UpperBound,        
   /*@}*/

   /**@name Messages from the Tree Manager to any slave process */
   /*@{*/
   /** BCP has finished. The slave process receiving this message will
       send back statistics to the TM and then terminate. TM will wait
       for all other processes to terminate. */
   BCP_Msg_FinishedBCP,       // TM -> slaves
   /** The TM sends the process type to the process (LP, Cut
       Generator, etc.) */
   BCP_Msg_ProcessType,       // TM -> slaves
   /** The TM sends the appropriate parameters to the slave process. */
   BCP_Msg_ProcessParameters, // TM -> slaves
   /** The TM sends the description of the core formulation to the
       slave process. */
   BCP_Msg_CoreDescription,   // TM -> slaves
   /** The TM sends the initial user packed information to the slave
       process. */
   BCP_Msg_InitialUserInfo,   // TM -> slaves
   /*@}*/

   /**@name Messages from the Tree Manager to an LP process */
   /*@{*/
   /** Repricing. Does not quite work yet... */
   BCP_Msg_RootToPrice,       // TM -> LP
   /** TM sends the description of a new search tree node. */   
   BCP_Msg_ActiveNodeData,    // TM -> LP
   /** TM warns an LP process that the second phase will start. (An LP
       process may use different branching, cut generation and pricing
       strategies in different phases.) */
   BCP_Msg_NextPhaseStarts,   // TM -> LP
   /** TM sends diving information. */
   BCP_Msg_DivingInfo,        // TM -> LP
   /** Send index set for cuts to be generated in the future. */
   BCP_Msg_CutIndexSet,        // TM -> LP
   /** Send index set for variables to be generated in the future. */
   BCP_Msg_VarIndexSet,        // TM -> LP
   /*@}*/

   /**@name Messages from an LP process to the Tree Manager */
   /*@{*/
   /**@name Sending a search tree node description */
   /*@{*/
   /** The node is discarded (fathomed). */
   BCP_Msg_NodeDescription_Discarded,        // LP -> TM
   /** The lower bound corresponding to the node is above the upper
       bound. The node will be saved for the next phase. */
   BCP_Msg_NodeDescription_OverUB,           // LP -> TM
   /** The node is infeasible. The node will be saved for the next phase. */
   BCP_Msg_NodeDescription_Infeas,           // LP -> TM
   /** The lower bound corresponding to the node is above the upper
       bound. The node is pruned (will not be saved for the next phase). */
   BCP_Msg_NodeDescription_OverUB_Pruned,    // LP -> TM
   /** The node is infeasible. The node is pruned (will not be saved
       for the next phase, compare above). */
   BCP_Msg_NodeDescription_Infeas_Pruned,    // LP -> TM
   /** In addition to the node description, branching information is
       sent as well so that the children of this node can be
       recreated. */
   BCP_Msg_NodeDescriptionWithBranchingInfo, // LP -> TM
   /*@}*/
   /** Repricing??? */
   BCP_Msg_PricedRoot,                       // LP -> TM
   /** The message contains a new MIP feasible solution. */
   BCP_Msg_FeasibleSolution,                 // LP -> TM
   /** The message contains the statistics the LP process collected. */
   BCP_Msg_LpStatistics,                     // LP -> TM

   /** Request an index set for cuts to be generated. Empty message body. */
   BCP_Msg_RequestCutIndexSet, // LP -> TM
   /** Request an index set for variables to be genarated. Empty
       message body. */
   BCP_Msg_RequestVarIndexSet, // LP -> TM
   /*@}*/

   /**@name Messages from an LP process to a Cut Generator or Cut Pool
      process. */
   /*@{*/
   /**@name Message tags indicating how a solution to the LP relaxation is
      packed in the buffer. */
   /*@{*/
   /** Only primal variables currently at nonzero level. */
   BCP_Msg_ForCG_PrimalNonzeros,     // LP -> CG / CP
   /** Only primal variables currently at fractional level. */
   BCP_Msg_ForCG_PrimalFractions,    // LP -> CG / CP
   /** All primal variables. */
   BCP_Msg_ForCG_PrimalFull,         // LP -> CG / CP
   /** The user packed everything. */
   BCP_Msg_ForCG_User,               // LP -> CG / CP
   /*@}*/
   /** The message contains cuts for the Cut Pool process. */
   BCP_Msg_CutsToCutPool,      // LP -> CP
   /*@}*/

   /**@name Messages from an LP process to a Variable Generator or Variable Pool process */
   /*@{*/
   /**@name Message tags indicating how a solution to the LP relaxation is
      packed in the buffer. */
   /*@{*/
   /** Pack only dual variables currently at nonzero level. */
   BCP_Msg_ForVG_DualNonzeros,     // LP -> VG / VP
   /** Pack all dual variables. */
   BCP_Msg_ForVG_DualFull,         // LP -> VG / VP
   /** The user packed everything. */
   BCP_Msg_ForVG_User,             // LP -> VG / VP
   /*@}*/
   /** The message contains variables for the Variable Pool process. */
   BCP_Msg_VarsToVarPool,          // LP -> VP
   /*@}*/

   /**@name Messages from a Cut Genarator or Cut Pool process to the
      corresponding LP process */
   /*@{*/
   /** The message contains the description of a cut. */
   BCP_Msg_CutDescription,     // CG / CP -> LP
   /** No more (violated) cuts could be found. (Message body is empty.) */
   BCP_Msg_NoMoreCuts,         // CG / CP -> LP
   /*@}*/

   /**@name Messages from a Variable Genarator or Variable Pool process to the
      corresponding LP process */
   /*@{*/
   /** The message contains the description of a variable. */
   BCP_Msg_VarDescription,     // VG / VP -> LP
   /** No more (improving) variables could be found. (Message body is
       empty.) */
   BCP_Msg_NoMoreVars          // VG / VP -> LP
   /*@}*/

   //    BCP_Msg_UserPacked,
   //    BCP_Msg_NoMoreMessage
};

#endif

