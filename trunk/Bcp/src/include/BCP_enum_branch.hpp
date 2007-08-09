// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_ENUM_BRANCH_H
#define _BCP_ENUM_BRANCH_H

// This file is fully docified.

//-----------------------------------------------------------------------------

/** This enumerative constant describes which child should be kept after the
    branching object is selected and the children are presolved. This constant
    is used by the built-in method for setting the actions for the children of
    the branching object. (<code>set_actions_for_children</code> in
    \URL[<code>BCP_lp_user</code>]{BCP_lp_user.html}). */

enum BCP_child_preference{
   /** Keep the child that has the lowest presolved objective value. */
   BCP_PreferChild_LowBound,
   /** Keep the child that has the highest presolved objective value. */
   BCP_PreferChild_HighBound,
   /** Keep the child that has the most to-be-integer variables at fractional
       level after presolving. */
   BCP_PreferChild_MoreFractional,
   /** Keep the child that has the least to-be-integer variables at fractional
       level after presolving. */
   BCP_PreferChild_LessFractional,
   /** Mainly for binary choices: take the down branch unconditionally */
   BCP_PreferDiveDown,
   /** Mainly for binary choices: take the up branch unconditionally */
   BCP_PreferDiveUp
};

//-----------------------------------------------------------------------------

/** This enumerative constant describes the possible outcomes of a branching
    attempt. Used only by BCP (the return value of the
    <code>BCP_lp_branch()</code> function). */
enum BCP_branching_result{
   /** The node is fathomed, the LP process should wait for a new node.
       Actually, this is a bit of a lie. This will be the return value if
       either the node is really fathomed (i.e., all children can be fathomed)
       or all children are sent back to the Tree Manager process and the LP
       should wait for a new node. */
   BCP_BranchingFathomedThisNode,
   /** Branching happened, one of the children is kept and that's what the LP
       process will continue to work on. */
   BCP_BranchingDivedIntoNewNode,
   /** No branching happend, continue to work on the same node. */
   BCP_BranchingContinueThisNode
};

//-----------------------------------------------------------------------------

/** This enumerative constant is the return value of the
    <code>select_branching_candidates()</code> method in
    \URL[<code>BCP_lp_user</code>]{BCP_lp_user.html} and describes what should
    BCP do. */
enum BCP_branching_decision{
   /** The node should be fathomed without even trying to branch. */
   BCP_DoNotBranch_Fathomed,
   /** BCP should continue to work on this node. */
   BCP_DoNotBranch,
   /** Branching must be done. In this case the method returns the branching
       object candidates in one of the arguments. */
   BCP_DoBranch
};

//-----------------------------------------------------------------------------

/** This enumerative constant is the return value of the
    <code>compare_presolved_branching_objects()</code> method in
    \URL[<code>BCP_lp_user</code>]{BCP_lp_user.html} and describes the
    relation between two presolved branching objects in strong branching. One
    of these branching objects is the "old" object, the one that's considered
    to be the best so far. The other object is the "new" one which has just
    been presolved and now we must decide which one is better. That is, decide
    which object should BCP use as the actual branching object if there were
    no more candidates. */

enum BCP_branching_object_relation{
   /** "old" is better, discard "new".*/
   BCP_OldPresolvedIsBetter,
   /** "new" is better, discard "old".*/
   BCP_NewPresolvedIsBetter,
   /** "new" is better and it's so good that even if there are more candidates
       forget them and use "new" as the branching object. */
   BCP_NewPresolvedIsBetter_BranchOnIt
};

//-----------------------------------------------------------------------------

/** This enumerative constant is used in the built-in method for comparing
    presolved branching object candidates
    (<code>compare_presolved_branching_objects()</code> in
    \URL[<code>BCP_lp_user</code>]{BCP_lp_user.html}) and specifies how the
    comparison should be done. The last four bits of the constant is used if
    the comparison is done based on the objective values (or, lower bounds on
    the LP relaxation value) of the presolved children. See the header file
    for the constant values. */

enum BCP_branching_object_comparison{
   // We have info about objval branching on the last four bits.
   // Of those, the last shows that this is objval branching, the three
   // before that show the various types.
   BCP_Comparison_Objval =        0x01,
   BCP_LowestLowObjval =          0x00 | BCP_Comparison_Objval,
   BCP_HighestLowObjval =         0x02 | BCP_Comparison_Objval,
   BCP_LowestHighObjval =         0x04 | BCP_Comparison_Objval,
   BCP_HighestHighObjval =        0x06 | BCP_Comparison_Objval,
   BCP_LowestAverageObjval =      0x08 | BCP_Comparison_Objval,
   BCP_HighestAverageObjval =     0x0a | BCP_Comparison_Objval
   // We have info about fracnum_branching on the second to last four bits.
   // Of those, the last shows that this is fracnum branching, the three
   // before that show the various types.
   //   BCP_Comparison_FracNum =   0x10,
   //   BCP_LowestLowFracNum =     0x00 | BCP_Comparison_FracNum,
   //   BCP_HighestLowFracNum =    0x20 | BCP_Comparison_FracNum,
   //   BCP_HighestLowFracNum =    0x40 | BCP_Comparison_FracNum,
   //   BCP_HighestHighFracNum =   0x60 | BCP_Comparison_FracNum,

};

//-----------------------------------------------------------------------------

/** This enumerative constant describes the possible values the
    <code>set_actions_for_children()</code> method of
    \URL[<code>BCP_lp_user</code>]{BCP_lp_user.html} can set for each child.
*/
enum BCP_child_action{
   /** This child should be fathomed. */
   BCP_FathomChild,
   /** This child should be returned to the Tree Manager for later processing.
    */
   BCP_ReturnChild,
   /** This child should be kept and dived into (provided diving is decided
       on. Of course, there can be at most one child that has this flag set as
       preferred action.) */
   BCP_KeepChild
};

//-----------------------------------------------------------------------------

/** This enumerative constant describes the diving status of the search tree
    node processed by the LP process. */
enum BCP_diving_status{
   /** Nonexistent diving status. Never happens, but there needs to be some
       default, which should indicate that there is a bug somewhere... */
   BCP_UnknownDivingStatus,
   /** After branching all children must be returned to the Tree Manager and
       the LP process should wait for a new node. */
   BCP_DoNotDive,
   /** After branching the LP process is free to decide whether it keeps a
       child to dive into. */
   BCP_DoDive,
   /** After branching the LP process must inquire of the Tree Manager whether
       it can dive into one of the children. */
   BCP_TestBeforeDive
};

#endif
