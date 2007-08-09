// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_math.hpp"
#include "BCP_buffer.hpp"
#include "BB_cut.hpp"

/****************************************************************************/
void
BB_cut::pack(BCP_buffer& buf) const
{
  buf.pack(OsiRowCut::lb())
    .pack(OsiRowCut::ub());
  const CoinPackedVector& v = OsiRowCut::row();
  const int numElem = v.getNumElements();
  buf.pack(v.getIndices(), numElem)
    .pack(v.getElements(), numElem);
}

/****************************************************************************/
BB_cut::BB_cut(BCP_buffer& buf) :
   BCP_cut_algo(-BCP_DBL_MAX, BCP_DBL_MAX), OsiRowCut()
{
   double lb, ub;
   buf.unpack(lb)
      .unpack(ub);
   OsiRowCut::setLb(lb);
   OsiRowCut::setUb(ub);

   int numElem;
   int* indices;
   double* elements;
   buf.unpack(indices, numElem, true)
      .unpack(elements, numElem, true);
   OsiRowCut::setRow(numElem, indices, elements);

   if(numElem > 0) {
     delete[] indices;
     delete[] elements;
   }
}

/****************************************************************************/
BB_cut::BB_cut(const OsiRowCut& cut) :
   BCP_cut_algo(cut.lb(), cut.ub()), OsiRowCut(cut)
{}

/****************************************************************************/
BCP_MemPool BB_cut::memPool(sizeof(BB_cut));
