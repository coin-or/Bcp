// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include <numeric>

#include "BCP_matrix.hpp"
#include "BCP_buffer.hpp"

void
BCP_lp_relax::pack(BCP_buffer& buf) const {
   const int major = getMajorDim();
   const int minor = getMinorDim();
   buf.pack(colOrdered_)
      .pack(extraGap_)
      .pack(extraMajor_)
      .pack(major)
      .pack(minor)
      .pack(size_)
      .pack(maxMajorDim_)
      .pack(maxSize_);

   const int * length = getVectorLengths();
   const int * start = getVectorStarts();
   const int * ind = getIndices();
   const double * elem = getElements();
   if (major > 0) {
     buf.pack(length, major);
     buf.pack(start, major+1);
     for (int i = 0; i < major; ++i)
       buf.pack(ind + start[i], length[i]);
     for (int i = 0; i < major; ++i)
       buf.pack(elem + start[i], length[i]);
   }

   buf.pack(_Objective).pack(_ColLowerBound).pack(_ColUpperBound)
      .pack(_RowLowerBound).pack(_RowUpperBound);
}

void
BCP_lp_relax::unpack(BCP_buffer& buf) {
   CoinPackedMatrix::gutsOfDestructor();

   buf.unpack(colOrdered_)
      .unpack(extraGap_)
      .unpack(extraMajor_)
      .unpack(majorDim_)
      .unpack(minorDim_)
      .unpack(size_)
      .unpack(maxMajorDim_)
      .unpack(maxSize_);

   length_  = new int[maxMajorDim_];
   start_   = new int[maxMajorDim_+1];
   index_   = new int[maxSize_];
   element_ = new double[maxSize_];
   if (majorDim_) {
     buf.unpack(length_, majorDim_, false);
     int md1 = majorDim_+1;
     buf.unpack(start_, md1, false);

     for (int i = 0; i < majorDim_; ++i) {
       int * itmp = index_ + start_[i];
       buf.unpack(itmp, length_[i], false);
     }
     for (int i = 0; i < majorDim_; ++i) {
       double * dtmp = element_ + start_[i];
       buf.unpack(dtmp, length_[i], false);
     }
   }

   buf.unpack(_Objective).unpack(_ColLowerBound).unpack(_ColUpperBound)
      .unpack(_RowLowerBound).unpack(_RowUpperBound);
}
