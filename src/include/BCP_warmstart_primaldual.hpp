// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_WARMSTART_PRIMALDUAL_H
#define _BCP_WARMSTART_PRIMALDUAL_H

#include "BCP_enum.hpp"
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

class BCP_warmstart_primaldual : public BCP_warmstart {
private:
  /** The assignment operator is declared but not defined to disable it. */
  BCP_warmstart_primaldual& operator=(const BCP_warmstart_primaldual&);

  /** The default constructor creates an empty WrtParent warmstart info (i.e.,
      no change wrt the parent). This is a private method since only the
      empty_wrt_this() method should be able to use this form of creation. */
  BCP_warmstart_primaldual() :
      _primal(BCP_Storage_WrtParent), _dual(BCP_Storage_WrtParent) {}

private:
  /** The primal vector stored as a vector change. */
  BCP_vec_change<double> _primal;
  /** The dual vector stored as a vector change. */
  BCP_vec_change<double> _dual;
   /*@}*/

public:
  /**@name Constructors and destructor */
  /*@{*/
    /** Create the object by unpacking it from a buffer */
    BCP_warmstart_primaldual(BCP_buffer& buf) : _primal(buf), _dual(buf) {}
    /** Create an explicitly stored warmstart info by considering the double
	arrays <code>[fpirst,plast)</code> and <code>[dpirst,dlast)</code> as
	the primal and dual vectors. */
    BCP_warmstart_primaldual(const double* pfirst, const double* plast,
			     const double* dfirst, const double* dlast) :
	_primal(pfirst, plast), _dual(dfirst, dlast) {}
    /** Copy constructor */
    BCP_warmstart_primaldual(const BCP_warmstart_primaldual& ws) :
	_primal(ws._primal), _dual(ws._dual) {}

    virtual ~BCP_warmstart_primaldual() {}
  /*@}*/

  // Inherited methods --------------------------------------------------------

  /** Return an CoinWarmStart object that can be fed to the LP engine. The
      implementation for this class will return an CoinWarmStartDual object. */
  virtual CoinWarmStart* convert_to_CoinWarmStart() const;

  virtual BCP_storage_t storage() const {
      BCP_storage_t return_matrix[16] = {
	  BCP_Storage_NoData, BCP_Storage_NoData,
	  BCP_Storage_NoData, BCP_Storage_NoData,
	  
	  BCP_Storage_NoData, BCP_Storage_Explicit,
	  BCP_Storage_WrtParent, BCP_Storage_NoData,
	  
	  BCP_Storage_NoData, BCP_Storage_WrtParent,
	  BCP_Storage_WrtParent, BCP_Storage_NoData,
	  
	  BCP_Storage_NoData, BCP_Storage_NoData,
	  BCP_Storage_NoData, BCP_Storage_NoData
      };
      int pst = _primal.storage();
      int dst = _dual.storage();
      return return_matrix[4*pst + dst];
  }

  virtual BCP_warmstart* clone() const {
    return new BCP_warmstart_primaldual(*this);
  }

  virtual BCP_warmstart* empty_wrt_this() const {
    // The default constructor creates an empty WrtParent warmstart info
    // (i.e., no change wrt the parent)
    return new BCP_warmstart_primaldual();
  }

  virtual int storage_size() const {
    return _primal.storage_size() + _dual.storage_size();
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
      _primal.pack(buf);
      _dual.pack(buf);
  }
};

#endif
