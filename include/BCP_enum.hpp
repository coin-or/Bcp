// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_ENUM_H
#define _BCP_ENUM_H

// This file is fully docified.

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes when to purge the slack cut pool (in the
   LP process). Possible values are:
*/

enum BCP_slack_cut_discarding{
   /** Purge the slack cuts when the LP starts processing a new search tree
       node. */
   BCP_DiscardSlackCutsAtNewNode,
   /** Purge the slack cuts at every iteration while processing search tree
       nodes. (Note that purging will be performed more often in this case.) */
   BCP_DiscardSlackCutsAtNewIteration
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes how to compute the "violation" of a
   generated cut. Possible values are:
*/

enum BCP_CutViolationNorm {
   /** The violation is interpreted in the normal sense, i.e.,
       max(0, max(lb-lhs, lhs-ub)) */
   BCP_CutViolationNorm_Plain,
   /** The violation is the distance of the fractional point from the cut */
   BCP_CutViolationNorm_Distance,
   /** The violation is the directional (in the direction of the objective
       distance of the fractional point from the cut */
   BCP_CutViolationNorm_Directional
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes the possible types of objects
   (variables and cuts). 
*/

enum BCP_object_t{
   /** Base object. */
   BCP_CoreObj,
   /** Indexed object. */
   BCP_IndexedObj,
   /** Algorithmic object. */
   BCP_AlgoObj,
   /** No object type is given. */
   BCP_NoObj
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes what to do when a search tree node
   becomes fathomable for the current set of columns. 
*/

enum BCP_column_generation{
   /** Do fathom the node. */
   BCP_DoNotGenerateColumns_Fathom,
   /** Do not generate columns, but send back the node to the Tree Manager for
    processing in the next phase. */
   BCP_DoNotGenerateColumns_Send,
   /** Attempt column generation. If new columns are found, continue
       processing this search tree node, otherwise fathom it. */
   BCP_GenerateColumns
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes how to store certain data for a search
   tree node. Examples for such data include warmstart information, current
   lower and upper bounds.
*/

enum BCP_storage_t{
   /** No data is stored. */
   BCP_Storage_NoData,
   /** The data stored is an explicit listing of values. */
   BCP_Storage_Explicit,
   /** The data stored is with respect to the same kind of data in the parent
       of the search tree node. (In this case only the changes are stored.) */
   BCP_Storage_WrtParent,
   /** The data stored is with respect to the original description of the base
       problem (as was given by the user). */
   BCP_Storage_WrtCore
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant gives the status of an object (variable or cut).
   It is comprised of some constants representing different properties; the
   actual status is the OR of these constants. 
*/

enum BCP_obj_status{
   /** No special information is given about the object. */
   BCP_ObjNoInfo = 0x00,
   /** The object does not need to be sent to the variable/cut pool. */
   BCP_ObjDoNotSendToPool = 0x01,
   /** The object cannot be branched on. */
   BCP_ObjCannotBeBranchedOn = 0x02,
   /** The object is not removable from the LP formulation, even if the object
       becomes inactive. E.g., every object that has been branched on has this
       flag set. */
   BCP_ObjNotRemovable = 0x04,
   /** The object is to be removed next time when the formulation is
       compressed. For instance, the object can be marked for removal when it
       becomes inactive, or before branching. */
   BCP_ObjToBeRemoved = 0x08,
   /** The object is inactive. Inactivity for variables means that the
       variable is fixed to one of its bounds, for cuts it means that the
       corresponding constraint is free in the formulation. */
   BCP_ObjInactive = 0x10 // fixed for vars, free for cuts
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes how the primal solution should be
   packed into a buffer if the default packing routine is used.
*/

enum BCP_primal_solution_description{
   /** Pack only those variables that are currently at nonzero levels. */
   BCP_PrimalSolution_Nonzeros,
   /** Pack only those variables that are currently at fractional (i.e.,
       non-integral) levels. */
   BCP_PrimalSolution_Fractions,
   /** Pack all primal variables. */
   BCP_PrimalSolution_Full
};

/**
   This enumerative constant describes how the dual solution should be packed
   into a buffer if the default packing routine is used.
*/

enum BCP_dual_solution_description{
   /** Pack only those variables that are currently at nonzero levels. */
   BCP_DualSolution_Nonzeros,
   /** Pack all dual variables. */
   BCP_DualSolution_Full
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes the integrality type of a variable. 
 */

enum BCP_var_t{
   /** Binary (0-1) variable. */
   BCP_BinaryVar,
   /** General integer variable. */
   BCP_IntegerVar,
   /** Continuous variable. */
   BCP_ContinuousVar
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes which constraints should be considered
   ineffective after solving an LP relaxation. 
 */

enum BCP_IneffectiveConstraints{
   /** None of the constraints are ever considered ineffective. That is, once
       a constraint is added to the formulation it will remain there. */
   BCP_IneffConstr_None,
   /** Constraints with nonzero (primal) slack value are considered
       ineffective. */
   BCP_IneffConstr_NonzeroSlack,
   /** Constraints with dual variables at level zero are considered
       ineffective. Note that for any primal-dual solution pair that is
       optimal for the LP relaxation, this set of constraints is a superset of
       the previous set. */
   BCP_IneffConstr_ZeroDualValue
};

//-----------------------------------------------------------------------------

/**
   This enumerative constant describes the possible return codes of the
   function that tests MIP feasibility of a solution to an LP relaxation. 
 */

enum BCP_feasibility{
   /** The solution is not MIP feasible. */
   BCP_NotFeasible,
   /** The solution is MIP feasible. */
   BCP_Feasible,
   /** The solution is not MIP feasible but the user was able to derive a MIP
       feasible solution from it (e.g. by rounding). In this case, the
       feasible solution will be sent to the Tree Manager. */
   BCP_HeuristicFeasible
};

/**
   This enumerative constant describes which built-in feasibility-testing
   routine should be invoked. This assumes the user does not override the
   feasibility-testing method. Note that the last option could be used for any
   problem. However, if we do know the integrality types of the variables then
   specifying the corresponding option leads to a more efficient code.
 */

enum BCP_feasibility_test{
   /** The problem is feasible if all primal variables take values 0 or 1.
       (Use this if and only if every variable is binary.) */
   BCP_Binary_Feasible,
   /** The problem is feasible if all primal variables are integral.
       (Use this if and only if every variable is integral.) */
   BCP_Integral_Feasible,
   /** The problem is feasible if all non-continuous variables are integral. */
   BCP_FullTest_Feasible
};

//-----------------------------------------------------------------------------
// The possible return values of price_variables
// *FIXME*: missing documentation 

enum BCP_dual_status{
   BCP_TotalDualFeasible = 1,
   BCP_TotalDualFeasible_HasAllIndexed = 1,
   BCP_TotalDualFeasible_NotAllIndexed = 2,
   BCP_NotTotalDualFeasible = 0
};

//-----------------------------------------------------------------------------
// *FIXME*: missing documentation 

/**
   This enumerative constant describes what sort of columns to generate in
   case column generation is requested. This is a bitmap, various bits
   indicating whether to generate that type.
 */
enum BCP_pricing_status{
   /** Generate nothing. Sort of stupid to request column generation and set
       this. */
   BCP_PriceNothing = 0x00,
   /** Generate algorithmic variables. */
   BCP_PriceAlgoVars = 0x01,
   /** Generate indexed variables but only check those in the
       yet-to-be-priced-named-variables buffer */
   BCP_PriceUntilLastIndexedToPrice = 0x02,
   /** Generate indexed variables, both those in the
       yet-to-be-priced-indexed-variables buffer, and those with user index
       bigger than the largest user index in the buffer */
   BCP_PriceAfterLastIndexedToPrice = 0x04,
   /** If either one of the previous two is set then we have to price indexed
       variables. */
   BCP_PriceIndexedVars = ( BCP_PriceUntilLastIndexedToPrice |
			    BCP_PriceAfterLastIndexedToPrice )
};

//-----------------------------------------------------------------------------
/**
   This enumerative constant describes the origin (originating process) of an
   object (variable or cut). 
 */

enum BCP_object_origin{
   /** The object was left over in the local variable or cut pool of the LP
       process from the previous iteration. */
   BCP_Object_Leftover,
   /** The object originates from a branching object. (The object is generated
    in the LP process.) */
   BCP_Object_Branching,
   /** The object was generated by a variable or cut generator. */
   BCP_Object_FromGenerator,
   /** The object is from a variable or cut pool. */
   BCP_Object_FromPool,
   /** The object is from the Tree Manager. */
   BCP_Object_FromTreeManager
};

//-----------------------------------------------------------------------------
/**
   This enumerative constant describes the possible outcomes when comparing
   two objects (variables or cuts). Two objects are comparable if they expand
   to the same column/row with the possible exception of their objective
   coefficient/bounds. <br>
   A cut is "better" than another (comparable) cut if its
   bounds define a subinterval of the interval defined by the bounds of the
   comparable cut. <br>
   *THINK*: How to compare variables?
 */

enum BCP_object_compare_result{
   /** The two objects are the same. */
   BCP_ObjsAreSame,
   /** The two objects are comparable but the first object is better. */
   BCP_FirstObjIsBetter,
   /** The two objects are comparable but the second object is better. */
   BCP_SecondObjIsBetter,
   /** The two objects are not comparable or neither is better than the other.
    */
   BCP_DifferentObjs
};

#endif
