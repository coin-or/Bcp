// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_WARMSTART_DUAL_H
#define _BCP_WARMSTART_DUAL_H

#include "BCP_warmstart.hpp"
#include "BCP_vector_change.hpp"

/* NOTE:
   Doxygen gracefully generates description for all inherited methods if they
   are not documented here. So the virtual methods are documented ONLY if they
   have something additional to say.
*/

//#############################################################################

/** This class describes a warmstart information that consists solely of the
    dual vector. */

class BCP_warmstart_dual : public BCP_warmstart {
private:
  /** The assignment operator is declared but not defined to disable it. */
  BCP_warmstart_dual& operator=(const BCP_warmstart_dual&);

  /** The default constructor creates an empty WrtParent warmstart info (i.e.,
      no change wrt the parent). This is a private method since only the
      empty_wrt_this() method should be able to use this form of creation. */
  BCP_warmstart_dual() : _dual(BCP_Storage_WrtParent) {}

private:
  /** The dual vector stored as a vector change. */
  BCP_vec_change<double> _dual;
   /*@}*/

public:
  /**@name Constructors and destructor */
  /*@{*/
    /** Create the object by unpacking it from a buffer */
    BCP_warmstart_dual(BCP_buffer& buf) : _dual(buf) {}
    /** Create an explicitly stored warmstart info by considering the double
	array <code>[first,last)</code> as the dual vector. */
    BCP_warmstart_dual(const double* first, const double* last) :
       _dual(first, last) {}
    /** Copy constructor */
    BCP_warmstart_dual(const BCP_warmstart_dual& ws) : _dual(ws._dual) {}

    virtual ~BCP_warmstart_dual() {}
  /*@}*/

  // Inherited methods --------------------------------------------------------

  /** Return an CoinWarmStart object that can be fed to the LP engine. The
      implementation for this class will return an CoinWarmStartDual object. */
  virtual CoinWarmStart* convert_to_CoinWarmStart() const;

  virtual BCP_storage_t storage() const { return _dual.storage(); }

  virtual BCP_warmstart* clone() const {
    return new BCP_warmstart_dual(*this);
  }

  virtual BCP_warmstart* empty_wrt_this() const {
    // The default constructor creates an empty WrtParent warmstart info
    // (i.e., no change wrt the parent)
    return new BCP_warmstart_dual();
  }

  virtual int storage_size() const {
    return _dual.storage_size();
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
    _dual.pack(buf);
  }
};

#endif
