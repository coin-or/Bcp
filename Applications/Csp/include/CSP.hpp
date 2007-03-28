// Copyright (C) 2005, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef CSP_H
#define CSP_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>

#include <CoinHelperFunctions.hpp>

#include "BCP_buffer.hpp"

class PATTERN;
class CSPROBLEM;

struct DELETEOBJECT {
template<typename T>
  void operator() (const T*& ptr){ delete ptr; ptr = NULL; }
template<typename T>
  void operator() (T*& ptr){ delete ptr; ptr = NULL; }
};

class CSP_packedVector {

private:
  int size_;
  int * indices_;
  double * elements_;


public: 
 // constructor: we ALWAYS copy
  CSP_packedVector(const int size,
		   const int* indices, 
		   const double* elements){
    size_=size;
    indices_=new int[size_];
    elements_=new double[size_];
    CoinDisjointCopyN(indices, size_, indices_);
    CoinDisjointCopyN(elements, size_, elements_);
  }

  CSP_packedVector():size_(0), indices_(0), elements_(0){}

  // constructor from a buffer
  
  CSP_packedVector(BCP_buffer & buf){
    buf.unpack(indices_, size_);
    buf.unpack(elements_, size_);
  }
    
  void  pack(BCP_buffer& buf) const {
    buf.pack(indices_, size_);
    buf.pack(elements_, size_);
  }

  void  unpack(BCP_buffer& buf){
    delete [] indices_;
    delete [] elements_;
    buf.unpack(indices_, size_);
    buf.unpack(elements_, size_);
  }


  const int* getIndices()const {
    return indices_;
  }

  const double* getElements() const {
    return elements_;
  }

  const int getSize() const {
    return size_;
  }

  // disable copy constructor
  CSP_packedVector(const CSP_packedVector& pv):size_(pv.size_){
    indices_=new int[size_];
    elements_=new double[size_];
    CoinDisjointCopyN(pv.indices_, size_, indices_);
    CoinDisjointCopyN(pv.elements_, size_, elements_);
  }
    
    



private:


  // disable assignmnet operator
  CSP_packedVector& operator=(const CSP_packedVector& pv);
};
  
    
//#############################################################################
// Class CSPROBLEM contains all the data 
//       for an instance of the cutting stock problem

class CSPROBLEM {

public:

  inline int getL() const { return l_; }
  inline const int* getDemand() const { return demand_; }
  inline const int* getW() const { return w_; }
  inline int getM() const { return m_; }
  inline int getS() const { return s_; }

   inline bool doesCombineExclusionConstraints() const {
      return combineExclusionConstraints;
   }
   inline bool doesAddKnapsackMirConstraints() const {
      return addKnapsackMirConstraints;
   }
   inline bool doesAddKnifeMirConstraints() const {
      return addKnifeMirConstraints;
   }

   inline void setCombineExclusionConstraints(char yesno) {
      combineExclusionConstraints = yesno;
   }
   inline void setAddKnapsackMirConstraints(char yesno) {
      addKnapsackMirConstraints = yesno;
   }
   inline void setAddKnifeMirConstraints(char yesno) {
      addKnifeMirConstraints = yesno;
   }
  
  CSPROBLEM();
  CSPROBLEM(std::istream &inputStream);
  ~CSPROBLEM(){}
  
  void pack(BCP_buffer& buf) const {
    buf.pack(l_);
    buf.pack(demand_, m_);
    buf.pack(w_, m_);
    buf.pack(s_);
    buf.pack(combineExclusionConstraints);
    buf.pack(addKnapsackMirConstraints);
    buf.pack(addKnifeMirConstraints);
  }

  void unpack(BCP_buffer& buf){
    buf.unpack(l_);
    buf.unpack(demand_, m_);
    buf.unpack(w_, m_);
    buf.unpack(s_);
    buf.unpack(combineExclusionConstraints);
    buf.unpack(addKnapsackMirConstraints);
    buf.unpack(addKnifeMirConstraints);
  }

private:
  int l_ ;            // stock roll length
  int* demand_;       // quanty of demanded widths
  int* w_;            // demanded widths
  int m_;             // number of demanded widths
  int s_;             // number of knives ("stilletto")
  double start_time_;

   char combineExclusionConstraints;
   char addKnapsackMirConstraints;
   char addKnifeMirConstraints;
 
 // upper and lower bounds on patterns (cols)
 // is handled by BCP variable classes
};

//#############################################################################

class PATTERN {
private:
  // One single, shared, CSPROBLEM for all instances of PATTERN
  const static CSPROBLEM* csproblem_;

   // The cost of the pattern, e.g. 1 for our flavor of the CSP.
  double cost_;

  // We may want to store ubs in the future
  // but right now, we'll always calculate

  // Need to know how many of each width to cut for this pattern
  // the integer vector  A^j*=(a^j*_1,....a^j*_m)
  CSP_packedVector widths_;   

   // at most how many of this pattern is needed
   double ub_;

public:
   
  // CSP_VECTOR's pack method returned a BCP_buffer &. Should this too?
  void pack(BCP_buffer& buf) const {
    buf.pack(cost_).pack(ub_);
    widths_.pack(buf);
    // no need to "return" because this returns a void
  }

  // this constructor is essentially an unpack method for the pattern    
  PATTERN(BCP_buffer & buf)
  {
    // need a routine that unpacks an OsiShallowPackedVector (?)
    // Q: How do I know how long the size is? 
    // A: It's known by the BCP_buffer method that unpacks a C-style array.
    //    See ~/COIN/Bcp/include/BCP_buffer.hpp

    // unpack the buffers into these variables
    buf.unpack(cost_).unpack(ub_);
    widths_.unpack(buf);
  }

  // destructor needs to delete the storage for the ShallowPackedVector
  virtual ~PATTERN(){
  }
  
  // may or maynot need these...
  inline double getCost () const { return cost_; }
  inline double getUb () const { return ub_; }

  const CSP_packedVector& getWidths() const{
    return widths_;
  }
  
  // return d_js based on current pis
  double dj(const double* pi) const;
  
  // default constructor 
  PATTERN() : cost_(1), widths_(){
  }


  // constructor 
  // Note: nonzero element values are permitted
  //       but shouldn't be allowed.
  //       we are going to assume in the vars-to-cols method that 
  //       the widths_ elements are stricly positive.
  //       This is for efficiency. 
  PATTERN(const int numElements, const int* indices, const double * elements,
	  const double ub) : 
     cost_(1), widths_(numElements, indices, elements), ub_(ub) {}

  // copy construct   - private for now. 
  PATTERN(const PATTERN& x) :
    cost_(x.cost_), widths_(x.widths_), ub_(x.ub_) {}




};

#endif

