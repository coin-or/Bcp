// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_VECTOR_GENERAL_H
#define _BCP_VECTOR_GENERAL_H

// This file is fully docified.

#include "BCP_os.hpp"

#include <memory>

/**
   The class BCP_vec serves the same purpose as the vector class in the
   standard template library. The main difference is that while the vector 
   class is <em>likely</em> to be implemented as a memory array, BCP_vec 
   <em>is</em> implemented that way. Also, BCP_vec has a number of extra
   member methods, most of them exist to speed up operations (e.g.,
   there are <em>unchecked</em> versions of the insert member methods,
   i.e., the method does not check whether there is enough space
   allocated to fit the new elements). 
*/

template <class T> class BCP_vec {
public:
   /**@name Type definitions (needed for using the STL) */
   /*@{*/
   ///
   typedef size_t size_type;
   ///
   typedef T value_type;
   ///
   typedef T* iterator;
   ///
   typedef const T* const_iterator;
   ///
   typedef T& reference;
   ///
   typedef const T& const_reference;
   /*@}*/

protected:
   /**@name Internal methods */
   /*@{*/
   /** insert <code>x</code> into the given <code>position</code> in the
       vector. Reallocate the vector if necessary. */
   void insert_aux(iterator position, const_reference x);
   /** allocate raw, uninitialized memory for <code>len</code> entries. */
   inline iterator allocate(size_t len) {
      return static_cast<iterator>(::operator new(len * sizeof(T)));
   }
   /** Destroy the entries in the vector and free the memory allocated for the
       vector. */
   void deallocate();
   /*@}*/

protected:
   /**@name Data members */
   /*@{*/
   /** Iterator pointing to the beginning of the memory array
       where the vector is stored. */ 
   iterator start;
   /** Iterator pointing to right after the last entry in the vector. */ 
   iterator finish;
   /** Iterator pointing to right after the last memory location usable by the
       vector without reallocation. */
   iterator end_of_storage;
   /*@}*/

public:
   /**@name Constructors / Destructor */
   /*@{*/
   /** The default constructor initializes the data members as 0 pointers. */
   BCP_vec() : start(0), finish(0), end_of_storage(0) {}
   /** The copy constructor copies over the content of <code>x</code>. */
   BCP_vec(const BCP_vec<T>& x) : start(0), finish(0), end_of_storage(0) {
      *this = x;
   }
   /** Construct a <code>BCP_vec</code> with <code>n</code> elements, all
       initialized with the second argument (or initialized with the default
       constructor of <code>T</code> if the second argument is missing). */
   BCP_vec(const size_t n, const_reference value = T());
   /** Construct a <code>BCP_vec</code> by copying the elements from
       <code>first</code> to <code>last-1</code>. */
   BCP_vec(const_iterator first, const_iterator last);
   /** Construct a <code>BCP_vec</code> by copying <code>num</code> objects
       of type <code>T</code> from the memory starting at <code>x</code>. */
   BCP_vec(const T* x, const size_t num);
   /** The destructor deallocates the memory allocated for the
       <code>BCP_vec</code>. */
   virtual ~BCP_vec() { deallocate(); }
   /*@}*/

   /**@name Query methods */
   /*@{*/
   /** Return an iterator to the beginning of the object. */
   iterator begin()                           { return start; }
   /** Return a const iterator to the beginning of the object. */
   const_iterator begin() const               { return start; }

   /** Return an iterator to the end of the object. */
   iterator end()                             { return finish; }
   /** Return a const iterator to the end of the object. */
   const_iterator end() const                 { return finish; }

   /** Return an iterator to the <code>i</code>-th entry. */
   iterator entry(const int i)                { return start + i; }
   /** Return a const iterator to the <code>i</code>-th entry. */
   const_iterator entry(const int i) const    { return start + i; }

   /** Return the index of the entry pointed to by <code>pos</code>. */
   size_t index(const_iterator pos) const     { return size_t(pos - start); }
   /** Return the current number of entries. */
   size_t size() const                        { return finish - start; }
   /** Return the capacity of the object (space allocated for this many
       entries). */
   size_t capacity() const                    { return end_of_storage - start;}
   /** Test if there are any entries in the object. */
   bool empty() const                         { return start == finish; }

   /** Return a reference to the <code>i</code>-th entry. */
   reference operator[](const size_t i)       { return *(start + i); }
   /** Return a const reference to the <code>i</code>-th entry. */
   const_reference operator[](const size_t i) const { return *(start + i); }

   /** Return a reference to the first entry. */
   reference front()                          { return *start; }
   /** Return a const reference to the first entry. */
   const_reference front() const              { return *start; }
   /** Return a reference to the last entry. */
   reference back()                           { return *(finish - 1); }
   /** Return a const reference to the last entry. */
   const_reference back() const               { return *(finish - 1); }
   /*@}*/

   /**@name General modifying methods */
   /*@{*/
   /** Reallocate the object to make space for <code>n</code> entries. */
   void reserve(const size_t n);
   /** Exchange the contents of the object with that of <code>x</code>. */
   void swap(BCP_vec<T>& x);

   /** Copy the contents of <code>x</code> into the object and return a
       reference the the object itself. */
   BCP_vec<T>& operator=(const BCP_vec<T>& x);

   /** Copy <code>num</code> entries of type <code>T</code> starting at the
       memory location <code>x</code> into the object. (<code>x</code> is a
       void pointer since it might be located somewhere in a buffer and
       therefore might not be aligned for type <code>T</code> entries.) */ 
   void assign(const void* x, const size_t num);
   /** Insert <code>num</code> entries starting from memory location
       <code>first</code> into the vector from position <code>pos</code>. */
   void insert(iterator position, const void* first, const size_t num);
   
   /** Insert the entries <code>[first,last)</code> into the vector from
       position <code>pos</code>. */
   void insert(iterator position, const_iterator first, const_iterator last);
   /** Insert <code>n</code> copies of <code>x</code> into the vector from
       position <code>pos</code>. */
   void insert(iterator position, const size_t n, const_reference x);
   /** Insert <code>x</code> (a single entry) into the vector at position
       <code>pos</code>. Return an iterator to the newly inserted entry. */
   iterator insert(iterator position, const_reference x);

   /** Append the entries in <code>x</code> to the end of the vector. */
   void append(const BCP_vec<T>& x) {
      insert(end(), x.begin(), x.end()); }
   /** Append the entries <code>[first,last)</code> to the end of the vector.
    */
   void append(const_iterator first, const_iterator last) {
      insert(end(), first, last); }

   /** Append <code>x</code> to the end of the vector. Check if enough space
       is allocated (reallocate if necessary). */
   inline void push_back(const_reference x) {
      if (finish != end_of_storage) {
	 BCP_CONSTRUCT(finish++, x);
      } else
	 insert_aux(finish, x);
   }
   /** Append <code>x</code> to the end of the vector. Does not check if
       enough space is allcoated. */
   inline void unchecked_push_back(const_reference x) {
      BCP_CONSTRUCT(finish++, x);
   }
   /** Delete the last entry. */
   inline void pop_back() {
      BCP_DESTROY(--finish);
   }

   /** Delete every entry. */
   void clear() {
      if (start) erase(start, finish);
   }

   /** Update those entries listed in <code>positions</code> to the given
       <code>values</code>. The two argument vector must be of equal length.
       Sanity checks are done on the given positions. */
   void update(const BCP_vec<int>& positions,
	       const BCP_vec<T>& values);
   /** Same as the previous method but without sanity checks. */
   void unchecked_update(const BCP_vec<int>& positions,
			 const BCP_vec<T>& values);

   /*@}*/

   //--------------------------------------------------------------------------

   /**@name Methods for selectively keeping entries */
   /*@{*/
   /** Keep only the entry pointed to by <code>pos</code>. */
   void keep(iterator pos);
   /** Keep the entries <code>[first,last)</code>. */
   void keep(iterator first, iterator last);
   /** Keep the entries indexed by <code>indices</code>. Abort if the indices
       are not in increasing order, if there are duplicate indices or if any
       of the indices is outside of the range <code>[0,size())</code>. */
   void keep_by_index(const BCP_vec<int>& positions);
   /** Same as the previous method but without the sanity checks. */
   void unchecked_keep_by_index(const BCP_vec<int>& positions);
   /** Like the other <code>keep_by_index</code> method (including sanity
       checks), just the indices of the entries to be kept are given in
       <code>[firstpos,lastpos)</code>. */
   void keep_by_index(const int * firstpos, const int * lastpos);
   /** Same as the previous method but without the sanity checks. */
   void unchecked_keep_by_index(const int * firstpos, const int * lastpos);
   /*@}*/

   //-------------------------------------------------------------------------

   /**@name Methods for selectively erasing entries */
   /*@{*/
   /** Erase the entry pointed to by <code>pos</code>. */
   void erase(iterator pos);
   /** Erase the entries <code>[first,last)</code>. */
   void erase(iterator first, iterator last);
   /** Erase the entries indexed by <code>indices</code>. Abort if the indices
       are not in increasing order, if there are duplicate indices or if any
       of the indices is outside of the range <code>[0,size())</code>. */
   void erase_by_index(const BCP_vec<int>& positions);
   /** Same as the previous method but without the sanity check. */
   void unchecked_erase_by_index(const BCP_vec<int>& positions);
   /** Like the other <code>erase_by_index</code> method (including sanity
       checks), just the indices of the entries to be erased are given in
       <code>[firstpos,lastpos)</code>. */
   void erase_by_index(const int * firstpos, const int * lastpos);
   /** Same as the previous method but without the sanity checks. */
   void unchecked_erase_by_index(const int * firstpos, const int * lastpos);
   /*@}*/
};

//============================================================================

// template <class T>
// bool operator==(const BCP_vec<T>& x, const BCP_vec<T>& y);

// template <class T>
// bool operator< (BCP_vec<T>& x, BCP_vec<T>& y);

//#############################################################################

/** This function purges the entries <code>[first,last)</code> from the vector
    of pointers <code>pvec</code>.
    Since these entries are pointers, first <code>operator delete</code> is
    invoked for each of them, then the pointers are erased from the vector. */

template <class T> void purge_ptr_vector(BCP_vec<T*>& pvec,
					 typename BCP_vec<T*>::iterator first,
					 typename BCP_vec<T*>::iterator last)
{
   typename BCP_vec<T*>::iterator origfirst = first;
   while (first != last) {
      delete *first;
      *first = 0;
      ++first;
   }
   pvec.erase(origfirst, last);
}


/** This function purges all the entries from the vector of pointers
    <code>pvec</code>. 
    Since these entries are pointers, first <code>operator delete</code> is
    invoked for each of them, then the pointers are erased from the vector. */

template <class T> void purge_ptr_vector(BCP_vec<T*>& pvec)
{
   purge_ptr_vector(pvec, pvec.begin(), pvec.end());
}


/** This function purges the entries indexed by <code>[first,last)</code> from
    the vector of pointers <code>pvec</code>. 
    Since these entries are pointers, first <code>operator delete</code> is
    invoked for each of them, then the pointers are erased from the vector. */

template <class T>
void purge_ptr_vector_by_index(BCP_vec<T*>& pvec,
			       typename BCP_vec<int>::const_iterator first,
			       typename BCP_vec<int>::const_iterator last)
{
   BCP_vec<int>::const_iterator origfirst = first;
   while (first != last) {
      delete pvec[*first];
      pvec[*first] = 0;
      ++first;
   }
   pvec.erase_by_index(origfirst, last);
}


/** This function keeps only the entries indexed by <code>[first,last)</code>
    from the vector of pointers <code>pvec</code>. 
    No sanity check is performed. */

template <class T>
void keep_ptr_vector_by_index(BCP_vec<T*>& pvec,
			      typename BCP_vec<int>::const_iterator first,
			      typename BCP_vec<int>::const_iterator last)
{
   BCP_vec<int>::const_iterator origfirst = first;
   const int pvec_size = pvec.size();
   int i;

   for (i = 0;  i < pvec_size && first != last; ++i) {
      if (i != *first) {
	 delete pvec[i];
	 pvec[i] = 0;
      } else {
	 ++first;
      }
   }

   for ( ; i < pvec_size; ++i) {
      delete pvec[i];
      pvec[i] = 0;
   }
   pvec.keep_by_index(origfirst, last);
}

/* Now include the implementation of the methods so the compiler could
   instantiate any requested vector class */

#include "BCP_vector_bool.cpp"
#include "BCP_vector_char.cpp"
#include "BCP_vector_short.cpp"
#include "BCP_vector_int.cpp"
#include "BCP_vector_double.cpp"

#endif
