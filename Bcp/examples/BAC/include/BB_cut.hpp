// Copyright (C) 2003, International Business Machines
// Corporation and others.  All Rights Reserved.

#ifndef _BB_CUT_H
#define _BB_CUT_H

#include "BCP_cut.hpp"
#include "BCP_mempool.hpp"
#include "OsiRowCut.hpp"

class BCP_buffer;

/****************************************************************************/
/** When doing a sprint sort of algorithm on the cuts (i.e., leave out a
    number of cuts at the beginning and add them only as necessary) this
    object represents one of these cuts. Only the index of the cut needs to be
    stored. */

class BB_indexed_cut : public BCP_cut_algo {

private:

    BB_indexed_cut();
    BB_indexed_cut(const BB_indexed_cut&);
    BB_indexed_cut& operator=(const BB_indexed_cut&);
    
private:

    static BCP_MemPool memPool;

    int index_;

public:

    static inline void * operator new(size_t size) {
	return memPool.alloc(size);
    }

    static inline void operator delete(void *p, size_t size) {
	memPool.free(p, size);
    }

    /// Packing cut to a buffer
    void pack(BCP_buffer& buf) const;

    /**@name Constructors and destructors */
    //@{
    /// Constructor from content of buffer 
    BB_indexed_cut(BCP_buffer& buf);

    /// Constructor from the index itself
    BB_indexed_cut(int index, double lb, double ub);

    /// Destructor
    ~BB_indexed_cut() {}

    inline int index() const { return index_; }
};


/****************************************************************************/
/** Simple representation of a cut by storing non zero coefficients only */ 

class BB_cut : public BCP_cut_algo, public OsiRowCut {

private:

    static BCP_MemPool memPool;

public:

    static inline void * operator new(size_t size) {
	return memPool.alloc(size);
    }

    static inline void operator delete(void *p, size_t size) {
	memPool.free(p, size);
    }

    /// Packing cut to a buffer
    void pack(BCP_buffer& buf) const;

    /**@name Constructors and destructors */
    //@{
    /// Constructor from content of buffer 
    BB_cut(BCP_buffer& buf);

    /// Constructor from an OsiRowCut 
    BB_cut(const OsiRowCut& cut);

    /// Destructor
    ~BB_cut() {}
};

#endif

