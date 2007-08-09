// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_MEMPOOL_H
#define _BCP_MEMPOOL_H

// #define BCP_MEMPOOL_SAVE_BLOCKHEADS

class BCP_MemPool {
private:
   const size_t BLOCK_SIZE;
   size_t entry_size;
   void ** first_free;
#ifdef BCP_MEMPOOL_SAVE_BLOCKHEADS
   void *** block_heads;
   size_t block_num;
   size_t max_block_num;
#endif
public:
   // Create an allocator for objects of size n
   BCP_MemPool(const size_t n, const size_t bl_size = 1023) :
      BLOCK_SIZE(bl_size), entry_size(n), first_free(0)
#ifdef BCP_MEMPOOL_SAVE_BLOCKHEADS
      , block_heads(0), block_num(0), max_block_num(0)
#endif
   {}

   // Allocate enough memory for one object;
   inline void * alloc(size_t n) {
      if (n != entry_size)
	 return ::operator new(n);
      void ** p = first_free;
      if (p) {
	 first_free = static_cast<void **>(*p);
      } else {
	 // create the new block and save its head
	 const size_t ptr_in_entry = entry_size/sizeof(void**) +
	    ((entry_size % sizeof(void **)) == 0 ? 0 : 1);
	 const size_t dist = ptr_in_entry * sizeof(void **);
	 void ** block = static_cast<void**>(::operator new(BLOCK_SIZE*dist));
#ifdef BCP_MEMPOOL_SAVE_BLOCKHEADS
	 // see if we can record another block head. If not, then resize
	 // block_heads
	 if (max_block_num == block_num) {
	    max_block_num = 1.2 * block_num + 10;
	    const void *** old_block_heads = block_heads;
	    block_heads = static_cast<void ***>(::operator new(max_block_num));
	    for (size_t i = 0; i < block_num; ++i)
	       block_heads[i] = old_block_heads[i];
	    ::operator delete(old_block_heads);
	 }
	 // save the new block
	 block_heads[block_num++] = block;
#endif
	 // link the entries in the new block together. skip the zeroth
	 // element, that'll be returned to the caller.
	 for (size_t i = 1; i < BLOCK_SIZE-1; ++i)
	    block[i*ptr_in_entry] =
	       static_cast<void*>(block + ((i+1)*ptr_in_entry));
	 // terminate the linked list with a null pointer
	 block[(BLOCK_SIZE-1)*ptr_in_entry] = 0;
	 p = block;
	 first_free = block + ptr_in_entry;
      }
      return static_cast<void*>(p);
   }

   // Return to the pool the memory pointed to by p; 
   inline void free(void *p, size_t n) {
      if (p == 0) return;
      if (n != entry_size) {
	 ::operator delete(p);
	 return;
      }
      void** pp = static_cast<void**>(p);
      *pp = static_cast<void*>(first_free);
      first_free = pp;
   }
   // Deallocate all memory in the pool
   ~BCP_MemPool() {
#ifdef BCP_MEMPOOL_SAVE_BLOCKHEADS
      for (size_t i = 0; i < block_num; ++i) {
	 ::operator delete(block_heads[i]);
      }
      ::operator delete(block_heads);
#endif
   }
};

#endif
