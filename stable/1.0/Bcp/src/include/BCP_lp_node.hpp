// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_NODE_H
#define _BCP_LP_NODE_H

#include "BCP_math.hpp"
#include "BCP_enum_branch.hpp"
#include "BCP_problem_core.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_indexed_pricing.hpp"

//#############################################################################

class BCP_warmstart;
class BCP_user_data;

//#############################################################################

/** NO OLD DOC

    This structure describes the ways various pieces of the current node's
    description are stored in the Tree Manager. */

struct BCP_node_storage_in_tm{
   /** */
   BCP_storage_t core_change;
   /** */
   BCP_storage_t var_change;
   /** */
   BCP_storage_t cut_change;
   /** */
   BCP_storage_t indexed_pricing;
   /** */
   BCP_storage_t warmstart;
};

//#############################################################################

/** NO OLD DOC
    
    This class holds the description of the parent of the current node. */

class BCP_lp_parent{
private:
   /**@name Disabled methods */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_lp_parent(const BCP_lp_parent&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_lp_parent& operator=(const BCP_lp_parent&);
   /*@}*/

public:
   /**@name Data members */
   /*@{*/
   /** */
   BCP_problem_core_change  core_as_change;
   /** */
   BCP_vec<int>   added_vars_index;
   /** */
   BCP_vec<BCP_obj_change>  added_vars_desc;
   /** */
   BCP_vec<int>   added_cuts_index;
   /** */
   BCP_vec<BCP_obj_change>  added_cuts_desc;
   /** */
   BCP_indexed_pricing_list indexed_pricing;
   /** */
   BCP_warmstart* warmstart;
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** */
   BCP_lp_parent() :
      core_as_change(),
      added_vars_index(),
      added_vars_desc(),
      added_cuts_index(),
      added_cuts_desc(),
      indexed_pricing(),
      warmstart(0)
   {}
   /** */
   ~BCP_lp_parent();
   /*@}*/

public:
   /**@name Modifying methods */
   /*@{*/
   /** */
   void clean();
   /*@}*/
};

//#############################################################################

/** NO OLD DOC

    This class holds the description of the current node itself. */

class BCP_lp_node {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_lp_node(const BCP_lp_node&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_lp_node& operator=(const BCP_lp_node&);
   /*@}*/

public:
   /**@name Data members */
   // *FIXME* divide data members into subgroups 
   /*@{*/
   /** */
   BCP_node_storage_in_tm tm_storage;
   /**@name Process id's ?? */
   /*@{*/
   /** */
   BCP_proc_id* cg;
   /** */
   BCP_proc_id* cp;
   /** */
   BCP_proc_id* vg;
   /** */
   BCP_proc_id* vp;
   /*@}*/
   /** */
   int level;
   /** */
   int index;
   /** */
   int iteration_count;
   /** */
   BCP_diving_status dive;
   /** */
   BCP_column_generation colgen;
   /** */
   double quality;
   /** */
   double true_lower_bound;
   /** */
   BCP_var_set vars;
   /** */
   BCP_cut_set cuts;
   /** */
   BCP_indexed_pricing_list indexed_pricing;
   /** */
   BCP_warmstart* warmstart;
   // this is tricky. this vector stores for each cut the lower bound on the
   // lp formulation at the time when the cut was added *in this node* OR if
   // there were columns added afterwards (stiil in the same node) then the
   // lower bound on the lp when columns were added the last time.
   /** */
   BCP_vec<double> lb_at_cutgen;
   /** Data the user wants to pass along with the search tree node. For now it
       cannot be stored wrt. the parent. */
   BCP_user_data* user_data;
   /*@}*/

public:
   /**@name Constructor and destructor */
   /*@{*/
   /** */
   BCP_lp_node() :
      tm_storage(),
      cg(0), cp(0), vg(0), vp(0),
      level(0), index(0), iteration_count(0),
      dive(BCP_DoNotDive), colgen(BCP_DoNotGenerateColumns_Fathom),
      quality(-BCP_DBL_MAX), true_lower_bound(-BCP_DBL_MAX), vars(), cuts(),
      indexed_pricing(),
      warmstart(0),
      user_data(0) {}
   /** */
   ~BCP_lp_node();
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** */
   inline size_t varnum() const { return vars.size(); }
   /** */
   inline size_t cutnum() const { return cuts.size(); }
   /*@}*/

   /**@name Modifying methods */
   /*@{*/
   /** */
   void clean();
   /*@}*/
};

//#############################################################################

#endif
