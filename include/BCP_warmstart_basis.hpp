// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_WARMSTART_BASIS_H
#define _BCP_WARMSTART_BASIS_H

#include "BCP_warmstart.hpp"
#include "BCP_vector_change.hpp"

/* NOTE:
   Doxygen gracefully generates description for all inherited methods if they
   are not documented here. So the virtual methods are documented ONLY if they
   have something additional to say.
*/

//#############################################################################

/** This class describes a warmstart information that consists of basis
    information for structural and artificial variables.

    "basic / on_upper / on_lower" info is stored on 2 bits for every variable.
    Only the added methods are documented in details on this page, for the
    inherited methods see the description of the base class. */

class BCP_warmstart_basis : public BCP_warmstart {
private:
  /** The assignment operator is declared but not defined to disable it. */
  BCP_warmstart_basis& operator=(const BCP_warmstart_basis&);

  /** The default constructor creates an empty WrtParent warmstart info (i.e.,
      no change wrt the parent). This is a private method since only the
      empty_wrt_this() method should be able to use this form of creation. */
  BCP_warmstart_basis() :
    _var_stat(BCP_Storage_WrtParent), _cut_stat(BCP_Storage_WrtParent) {}

private:
  /** The stati of the variables stored as a vector change. */
  BCP_vec_change<char> _var_stat;
  /** The stati of the cuts stored as a vector change. */
  BCP_vec_change<char> _cut_stat;

public:
  /**@name Constructors and destructor */
  /*@{*/
    /** Create the object by unpacking it from a buffer */
    BCP_warmstart_basis(BCP_buffer& buf) : _var_stat(buf), _cut_stat(buf) {}
    /** Create an explicitly stored warmstart info by considering the two
	character arrays (<code>[vfirst,vlast)</code> and
	<code>[cfirst,clast)</code>) as the status arrays for the
	variables/cuts */
    BCP_warmstart_basis(const char* vfirst, const char* vlast,
			const char* cfirst, const char* clast) :
       _var_stat(vfirst, vlast), _cut_stat(cfirst, clast) {}
    /** Copy constructor */
    BCP_warmstart_basis(const BCP_warmstart_basis& ws) :
      _var_stat(ws._var_stat), _cut_stat(ws._cut_stat) {}

   ~BCP_warmstart_basis() {}
  /*@}*/

  // Inherited methods --------------------------------------------------------

  /** Return an CoinwarmStart object that can be fed to the LP engine. The
      implementation for this class will return an CoinwarmStartBasis
      object. */
  virtual CoinWarmStart* convert_to_CoinWarmStart() const;

  virtual BCP_storage_t storage() const;

  virtual BCP_warmstart* clone() const {
    return new BCP_warmstart_basis(*this);
  }

  virtual BCP_warmstart* empty_wrt_this() const {
    // The default constructor creates an empty WrtParent warmstart info
    // (i.e., no change wrt the parent)
    return new BCP_warmstart_basis();
  }

  virtual int storage_size() const {
    return _var_stat.storage_size() + _cut_stat.storage_size();
  }

  virtual void update(const BCP_warmstart* const change);

  virtual BCP_warmstart* as_change(const BCP_warmstart* const old_ws,
				   const BCP_vec<int>& del_vars,
				   const BCP_vec<int>& del_cuts,
				   const double petol,
				   const double detol) const; 

  // Not inherited methods ----------------------------------------------------

  /** Pack the warmstart info into a buffer. */
  void pack(BCP_buffer& buf) const {
    _var_stat.pack(buf);
    _cut_stat.pack(buf);
  }
};

#endif
