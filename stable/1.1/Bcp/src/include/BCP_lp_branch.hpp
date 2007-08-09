// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_LP_BRANCH_H
#define _BCP_LP_BRANCH_H

// This file is fully docified.

#include "BCP_math.hpp"
#include "BCP_enum_branch.hpp"
#include "BCP_vector.hpp"
#include "BCP_lp_result.hpp"
#include "BCP_USER.hpp"
#include "BCP_var.hpp"
#include "BCP_cut.hpp"
#include "BCP_matrix.hpp"

//#############################################################################

class OsiSolverInterface;

//#############################################################################
// ASSUMPTION
// ----------
// All a branching object does are:
// - add cuts and/or variables
// - change bounds on cuts and/or variables
//#############################################################################

// *THINK* : Maybe the methods should be hidden???

// *THINK* : Maybe we should just use vectors instead of pointers to vectors?
// *THINK* : It'd simplify things and would not be significant extra storage
// *THINK* : (maybe none at all, depending pon the system operator new...)

/**
   This class describes a generic branching object. The object may contain
   variables and cuts to be added to the formulation and it contains the
   bound changes for each child.

   Note that it is unlikely that the user would need any of the member
   functions. She should simply call its constructor to create the object. The
   rest is internal to BCP.
*/

class BCP_lp_branching_object {
private:
	/**@name Disabled methods */
	/*@{*/
	/** The copy constructor is declared but not defined to disable it. */
	BCP_lp_branching_object(const BCP_lp_branching_object&);
	/** The assignment operator is declared but not defined to disable it. */
	BCP_lp_branching_object& operator=(const BCP_lp_branching_object&);
	/*@}*/

public:
	/**@name Data members */
	/*@{*/
	/** The number of children for this branching object. */
	int child_num;
	/** Variables to be added to the formulation. */
	BCP_vec<BCP_var*>* vars_to_add;
	/** Cuts to be added to the formulation. */
	BCP_vec<BCP_cut*>* cuts_to_add;

	/**@name Data members referring to forced bound changes. "Forced" means
	   that the branching rule specifies this change. */
	/*@{*/
	/** Positions of variables whose bounds change ("forcibly", by branching)
		in the children. If a position is non-negative, it refers to a variable
		in the current LP formulation. If a position is negative (-i), it
		refers to an added variable (i-1st). */ 
	BCP_vec<int>* forced_var_pos;
	/** Positions of cutss whose bounds change ("forcibly", by branching)
		in the children. If a position is non-negative, it refers to a cut
		in the current LP formulation. If a position is negative (-i), it
		refers to an added cut (i-1st). */ 
	BCP_vec<int>* forced_cut_pos;
	/** Contains the actual bounds for the variables indexed by <code>
		forced_var_pos</code>. List the lower/upper bound pairs for each of the
		variables, for each child in turn. The length of this vector is thus
		<code>child_num * 2 * forced_var_pos.size()</code>. */
	BCP_vec<double>* forced_var_bd;
	/** Contains the actual bounds for the cuts indexed by <code>
		forced_cut_pos</code>. List the lower/upper bound pairs for each of the
		cuts, for each child in turn. The length of this vector is thus
		<code>child_num * 2 * forced_cut_pos.size()</code>. */
	BCP_vec<double>* forced_cut_bd;
	/*@}*/

	/**@name Data members referring to implied bound changes. "Implied" means
	   that these changes could be made by applying the forced changes and then
	   doing logical fixing. Therefore this information is not recorded in the
	   Tree Manager when it stores the branching rule. However, if there are
	   implied changes, it is useful to specify them, because strong branching
	   may be more effecive then.

	   The interpretation of these data members is identical to their forced
	   counterparts. */
	/*@{*/
	///
	BCP_vec<int>* implied_var_pos;
	///
	BCP_vec<int>* implied_cut_pos;
	///
	BCP_vec<double>* implied_var_bd;
	///
	BCP_vec<double>* implied_cut_bd;
	/*@}*/
	/*@}*/

public:
	/**@name Constructor and destructor */
	/*@{*/
	/** The constructor makes a copy of each vector passed to it. If a 0
		pointer is passed for one of the arguments that means that the vector
		is empty. <code>new_{vars,cuts}</code> contains the variables/cuts to
		be added and <code>[fi][vc][pb]</code> contains the forced/implied
		variable/cut positions/bounds. */
	explicit BCP_lp_branching_object(const int children,
									 BCP_vec<BCP_var*>* const new_vars,
									 BCP_vec<BCP_cut*>* const new_cuts,
									 const BCP_vec<int>* const fvp,
									 const BCP_vec<int>* const fcp,
									 const BCP_vec<double>* const fvb,
									 const BCP_vec<double>* const fcb,
									 const BCP_vec<int>* const ivp,
									 const BCP_vec<int>* const icp,
									 const BCP_vec<double>* const ivb,
									 const BCP_vec<double>* const icb ) :
		child_num(children),
		vars_to_add(0), cuts_to_add(0),
		forced_var_pos(0), forced_cut_pos(0),
		forced_var_bd(0), forced_cut_bd(0),
		implied_var_pos(0), implied_cut_pos(0),
		implied_var_bd(0), implied_cut_bd(0)
	{
		if ( ((fvp == 0) ^ (fvb == 0)) || ((fcp == 0) ^ (fcb == 0)) || 
			 ((ivp == 0) ^ (ivb == 0)) || ((icp == 0) ^ (icb == 0)) )
			throw BCP_fatal_error("Bad args to BCP_lp_branching_object()\n");
		if ( (fvp && (2 * children* fvp->size() != fvb->size())) ||
			 (fcp && (2 * children* fcp->size() != fcb->size())) ||
			 (ivp && (2 * children* ivp->size() != ivb->size())) ||
			 (icp && (2 * children* icp->size() != icb->size())) )
			throw BCP_fatal_error("Bad args to BCP_lp_branching_object()\n");

		if (new_vars) {
			vars_to_add = new BCP_vec<BCP_var*>(*new_vars);
			new_vars->clear();
		}
		if (new_cuts) {
			cuts_to_add = new BCP_vec<BCP_cut*>(*new_cuts);
			new_cuts->clear();
		}

		if (fvp) forced_var_pos = new BCP_vec<int>(*fvp);
		if (fcp) forced_cut_pos = new BCP_vec<int>(*fcp);
		if (fvb) forced_var_bd = new BCP_vec<double>(*fvb);
		if (fcb) forced_cut_bd = new BCP_vec<double>(*fcb);

		if (ivp) implied_var_pos = new BCP_vec<int>(*ivp);
		if (icp) implied_cut_pos = new BCP_vec<int>(*icp);
		if (ivb) implied_var_bd = new BCP_vec<double>(*ivb);
		if (icb) implied_cut_bd = new BCP_vec<double>(*icb);
	}

	// NOTE: when the desctructor `delete's vars_to_add and cuts_to_add, it
	// will just delete the pointers in the BCP_lp_..._sets (see the
	// destructors of those classes). But this is intentional, because the
	// vars/cuts will be actually deleted only later, when the unnecessery
	// ones are deleted from the formulation.
	/** The destructor deletes each vector. */
	~BCP_lp_branching_object() {
		delete vars_to_add;     delete cuts_to_add;
		delete forced_var_pos;  delete forced_cut_pos;
		delete forced_var_bd;   delete forced_cut_bd;
		delete implied_var_pos; delete implied_cut_pos;
		delete implied_var_bd;  delete implied_cut_bd;
	}
	/*@}*/

	/**@name Query methods */
	/*@{*/
	/** Return the number of variables whose bounds are affected by the
		branching. (Including both forced and implied changes.) */
	inline int vars_affected() const {
		return
			(forced_var_bd ? forced_var_bd->size() / (2*child_num) : 0) +
			(implied_var_bd ? implied_var_bd->size() / (2*child_num) : 0);
	}
	/** Return the number of cuts whose bounds are affected by the branching.
		(Including both forced and implied changes.) */
	inline int cuts_affected() const {
		return
			(forced_cut_bd ? forced_cut_bd->size() / (2*child_num) : 0) +
			(implied_cut_bd ? implied_cut_bd->size() / (2*child_num) : 0);
	}
	/** Return the number of variables added in the branching. */
	inline int vars_added() const {
		return vars_to_add ? vars_to_add->size() : 0;
	}
	/** Return the number of cuts added in the branching. */
	inline int cuts_added() const {
		return cuts_to_add ? cuts_to_add->size() : 0;
	}
	/** Return a const iterator to the position in the forced variable bound
		changes where the new bounds for the <code>index</code>-th child start.
		This method should be invoked only if the appropriate data member is
		non-0. */
	inline
	BCP_vec<double>::const_iterator forced_var_bd_child(const int index)const {
		return forced_var_bd->entry(2 * forced_var_pos->size() * index);
	}
	/** Return a const iterator to the position in the forced cut bound
		changes where the new bounds for the <code>index</code>-th child start.
		This method should be invoked only if the appropriate data member is
		non-0. */
	inline
	BCP_vec<double>::const_iterator forced_cut_bd_child(const int index)const {
		return forced_cut_bd->entry(2 * forced_cut_pos->size() * index);
	}
	/** Return a const iterator to the position in the implied variable bound
		changes where the new bounds for the <code>index</code>-th child start.
		This method should be invoked only if the appropriate data member is
		non-0. */
	inline
	BCP_vec<double>::const_iterator implied_var_bd_child(const int index)const{
		return implied_var_bd->entry(2 * implied_var_pos->size() * index);
	}
	/** Return a const iterator to the position in the implied cut bound
		changes where the new bounds for the <code>index</code>-th child start.
		This method should be invoked only if the appropriate data member is
		non-0. */
	inline
	BCP_vec<double>::const_iterator implied_cut_bd_child(const int index)const{
		return implied_cut_bd->entry(2 * implied_cut_pos->size() * index);
	}
	/*@}*/

	/**@name Modifying methods */
	/*@{*/
	// this routine reorders the positions/bounds as well so that the positions
	// are in increasing order (the vars/cuts are already added to the problem,
	// no need to reorder those, too)
	/** This method "cleans up" the positions and bounds. First it re-indexes
		the negative positions starting from <code>added_vars_start</code> for
		the variables (and from <code>added_cuts_start</code> for the cuts).
		Then it reorders the positions (and follows that ordering with the
		bounds) so that the positions will be in increasing order. */
	void init_pos_for_added(const int added_vars_start,
							const int added_cuts_start);
	/** This method invokes the appropriate methods of <code>lp</code> to apply
		the forced and implied bound changes of the <code>child_ind</code>-th
		child. */
	void apply_child_bd(OsiSolverInterface* lp, const int child_ind) const;
	/** This method prints out some information about the branching object.
		(The positions, new bounds, the primal value of the variables.) */
	void print_branching_info(const int orig_varnum,
							  const double* x, const double * obj) const;
	/*@}*/
};

//#############################################################################

/** A presolved branching object candidate.

During strong branching all children of a branching object (of class
\URL[<code>BCP_lp_branching_object</code>]{BCP_lp_branching_object.html})
are <em>presolved</em> to gain some insight how good it would be to branch
on that particular branching object. The results of the presolved children
are stored in an object of this type as well as the instructions what to
do with each child in case this object is selected for branching.
*/

class BCP_presolved_lp_brobj {
private:
	/**@name Disabled methods */
	/*@{*/
	/** The copy constructor is declared but not defined to disable it. */
	BCP_presolved_lp_brobj(const BCP_presolved_lp_brobj&);
	/** The assignment operator is declared but not defined to disable it. */
	BCP_presolved_lp_brobj& operator=(const BCP_presolved_lp_brobj&);
	/*@}*/

private:
	/**@name Data members */
	/*@{*/
	/** A pointer to the branching object (created internally or by the user)
		whose presolved data is stored in this object */
	BCP_lp_branching_object* _candidate;
	/** A vector of lp results holding the actual presolved data. */
	BCP_vec<BCP_lp_result*> _lpres;
	// what to do with each child
	/** The action to be taken for each child (send back to the TM, keep for
		diving, prune it). Note that this member is created if and only if
		the object is selected for branching. */
	BCP_vec<BCP_child_action> _child_action;
	/** The user data to be passed around with the child nodes. Note that this
		member is created if and only if the object is selected for
		branching. */
	BCP_vec<BCP_user_data*> _user_data;
	/** */
	BCP_vec< BCP_vec<BCP_cut*> > _new_cuts;
	BCP_vec< BCP_vec<BCP_row*> > _new_rows;
	/*@}*/

public:
	/**@name Constructor and destructor */
	/*@{*/
	/** The only one way to construct a presolved branching object is to create
		it from a regular branching object.*/
	explicit BCP_presolved_lp_brobj(BCP_lp_branching_object* candidate) :
		_candidate(candidate),
		_lpres(),
		_child_action(candidate->child_num, BCP_ReturnChild),
		_user_data(candidate->child_num, NULL)
	{
		_lpres.reserve(candidate->child_num);
		for (int i = candidate->child_num; i; --i) {
			_lpres.unchecked_push_back(new BCP_lp_result);
			_new_cuts.push_back(BCP_vec<BCP_cut*>());
			_new_rows.push_back(BCP_vec<BCP_row*>());
		}
	}
	/** The destructor
		simply deletes every member (deletes every lpres in the vector). */
	~BCP_presolved_lp_brobj() {
		purge_ptr_vector(_lpres);
		purge_ptr_vector(_user_data);
		for (int i = _new_cuts.size() - 1; i >= 0; --i) {
			purge_ptr_vector(_new_cuts[i]);
			purge_ptr_vector(_new_rows[i]);
		}
	}
	/*@}*/

	/**@name Query methods */
	/*@{*/
	/** Return a pointer to the candidate. <br>
	 *THINK* : is it necessary to have a non-const version???
	 */
	inline BCP_lp_branching_object* candidate() {
		return _candidate;
	}
	/** Return a const pointer to the candidate. */
	inline const BCP_lp_branching_object* candidate() const {
		return _candidate;
	}
	/** Return a const reference to the presolved results of the
		<code>child_ind</code>-th child. */
	inline const BCP_lp_result& lpres(const int child_ind) const {
		return *(_lpres[child_ind]);
	}
	/** Return a reference to the actions to be taken. A non-const method is
		needed, since this is the easiest way to set the entries. Maybe it'd be
		cleaner to have a separate set method... */
	inline BCP_vec<BCP_child_action>& action() {
		return _child_action;
	}
	/** Return a const reference to the actions to be taken. */
	inline const BCP_vec<BCP_child_action>& action() const {
		return _child_action;
	}

	/** Return a reference to the user data vector. A non-const method is
		needed, since this is the easiest way to set the entries. Maybe it'd be
		cleaner to have a separate set method... */
	inline BCP_vec<BCP_user_data*>& user_data() {
		return _user_data;
	}
	/** Return a const reference to the user data vector. */
	inline const BCP_vec<BCP_user_data*>& user_data() const {
		return _user_data;
	}

	/** Return true if every children can be fathomed. (The lower bound for
		each is above <code>objval_limit</code>.) */
	const bool fathomable(const double objval_limit) const;
	/** Return true if at least one child had numerical difficulties while
		presolving. */
	const bool had_numerical_problems() const; 
	/*@}*/

	/**@name Modifying methods */
	/*@{*/
	inline void initialize_lower_bound(const double val) {
		for (int i = _candidate->child_num - 1; i >= 0; --i) {
			_lpres[i]->fake_objective_value(val);
		}
	}
	inline void keep_no_child() {
		for (int i = _child_action.size() - 1; i >= 0; --i) {
			if (_child_action[i] == BCP_KeepChild) {
				_child_action[i] = BCP_ReturnChild;
				return;
			}
		}
	}
	inline bool is_pruned() const {
		for (int i = _child_action.size() - 1; i >= 0; --i) {
			if (_child_action[i] != BCP_FathomChild)
				return false;
		}
		return true;
	}
	/** Fill up <code>obj</code> with the lower bound on each child. */
	inline void get_lower_bounds(BCP_vec<double>& obj) {
		obj.clear();
		obj.reserve(_candidate->child_num);
		const int num_children = _lpres.size();
		for (int i = 0; i < num_children; ++i)
			obj.unchecked_push_back(_lpres[i]->objval());
	}
	/** Fill up the lower bounds on the children with the content of
		<code>obj</code>.*/
	inline void set_lower_bounds(const BCP_vec<double>& obj) {
		const int num_children = _lpres.size();
		for (int i = 0; i < num_children; ++i)
			_lpres[i]->fake_objective_value(obj[i]);
	}
	/** Extract the lp results from the LP solver for the
		<code>child_ind</code>-th child. This is done immediately after
		presolving the child. */
	inline void get_results(OsiSolverInterface& lp, const int child_ind) {
		_lpres[child_ind]->get_results(lp);
	}
	/** Examine the termination codes for the children and for those that do
		not have a valid lower bound fake the objective value depending on the
		termination code:
		<ul>
		<li> primal infeasibility / dual objective limit: <code>BCP_DBL_MAX</code>
		<li> iteration limit : maximum of the lower bound at termination and
		<code>itlim_objval</code>
		<li> abandoned : <code>itlim_objval</code>
		</ul>
	*/
	void fake_objective_values(const double itlim_objval);

	/** swap the two presolved branching object */
	void swap(BCP_presolved_lp_brobj& rhs) {
		std::swap(_candidate, rhs._candidate);
		_lpres.swap(rhs._lpres);
		_child_action.swap(rhs._child_action);
		_user_data.swap(rhs._user_data);
		_new_cuts.swap(rhs._new_cuts);
		_new_rows.swap(rhs._new_rows);
	}
	inline BCP_vec< BCP_vec<BCP_cut*> >& get_new_cuts() { return _new_cuts; }
	inline BCP_vec< BCP_vec<BCP_row*> >& get_new_rows() { return _new_rows; }
	/*@}*/
};

#endif
