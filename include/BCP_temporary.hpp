// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_TEMP_H
#define _BCP_TEMP_H

// This file is fully docified.

#include "BCP_vector.hpp"

/** This class should be used when the user needs a vector for temporary
    storage. 

    Frequently one needs to use a vector (a BCP_vec) just within a function.
    To allocate space for the memory array in that vector all the time the
    function is executed and free it each time the function exits is a waste
    of time (especially if the vector grows, thus reallocation is needed).
    <code>BCP_temp_vec</code>, a <em>temporary vector</em> solves this
    problem. These temporary vectors are never actually deleted. When a
    destructor is called for a temporary vector, the memory array in it is not
    freed, rather it is placed on a stack. Similarly, when a temporary vector
    is constructed, a vector from the top of the stack is returned (instead of
    allocating place for a new). Also, since these temporary vectors are never
    freed, they can only grow in size. This has the additional advantage that
    after a while no reallocation of these vectors will be necessary. 

    Usage of a temporary vector:
<pre>
{
   ...
   BCP_temp_vec<int> tmp_int_vec; // or use a different constructor
   BCP_vec<int>& int_vec = tmp_int_vec.vec(); // get the reference to the vec
   ... use int_vec
} // the BCP_temp_vec goes out of scope, its vector is put back on the stack
</pre>
*/

template <class T> class BCP_temp_vec {
private:
   /**@name Disabled methods */
   /*@{*/
   /** The copy constructor is declared but not defined to disable it. */
   BCP_temp_vec(const BCP_temp_vec&);
   /** The assignment operator is declared but not defined to disable it. */
   BCP_temp_vec<T>& operator=(const BCP_temp_vec&);
   /*@}*/

private:
   /**@name Private data members */
   /*@{*/
   /** The stack that holds the currently unused temporary vectors (pointers
       to them). Note that this member is static, thus <em>all</em> existing
       temporary vectors have access to it. */
   static BCP_vec<BCP_vec<T>*> _temp_vecs;
   /** The vector this temporary vector uses. */
   BCP_vec<T>* _vec;
   /*@}*/
   //--------------------------------------------------------------------------

public:
   /**@name Query method */
   /*@{*/
   /** Return a reference to the actual vector in this temporary vector. */
   inline BCP_vec<T>& vec() { return *_vec; }
   /** Return a const reference to the actual vector in this temporary vector.
    */
   inline const BCP_vec<T>& vec() const { return *_vec; }
   /*@}*/
   //--------------------------------------------------------------------------

   /**@name Constructors and destructor. 
      All constructors first create a vector by either taking the vector from
      the top of the stack or by creating a new vector if the stack is empty.
      After the vector is created it is filled according to the constructor. */
   /*@{*/
   /** Create an empty temporary vector. */
   BCP_temp_vec() : _vec(0) {
      if (!_temp_vecs.empty()){
	 _vec = _temp_vecs.back();
	 _temp_vecs.pop_back();
      } else {
	 _vec = new BCP_vec<T>;
      }
   }
   /** Space will be reserved for <code>size</code> entries but the space
       remains uninitialized. */
   BCP_temp_vec(const size_t size) : _vec(0) {
      if (!_temp_vecs.empty()){
	 _vec = _temp_vecs.back();
	 _temp_vecs.pop_back();
      } else {
	 _vec = new BCP_vec<T>;
      }
      _vec->reserve(size);
   }
   /** Fill the vector with <code>size</code> copies of <code>x</code>. */
   BCP_temp_vec(const size_t size, const T& x) : _vec(0) {
      if (!_temp_vecs.empty()){
	 _vec = _temp_vecs.back();
	 _temp_vecs.pop_back();
      } else {
	 _vec = new BCP_vec<T>;
      }
      _vec->reserve(size);
      _vec->insert(_vec->end(), size, x);
   }
   /** The vector will be a copy of <code>x</code>. */
   BCP_temp_vec(const BCP_vec<T>& x) : _vec(0) {
      if (!_temp_vecs.empty()){
	 _vec = _temp_vecs.back();
	 _temp_vecs.pop_back();
      } else {
	 _vec = new BCP_vec<T>;
      }
      *_vec = x;
   }
   /** The destructor clears the vector (but the space that was allocated for
       the entries is not freed) and the vector is placed on top of the stack.
   */
   ~BCP_temp_vec() {
      _vec->clear();
      _temp_vecs.push_back(_vec);
   }
   /*@}*/
};

#endif
