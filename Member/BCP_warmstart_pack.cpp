// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_warmstart_dual.hpp"
#include "BCP_warmstart_basis.hpp"
#include "BCP_buffer.hpp"
#include "BCP_error.hpp"

void
BCP_pack_warmstart(const BCP_warmstart* ws, BCP_buffer& buf)
{
   const BCP_warmstart_basis* wsb =
      dynamic_cast<const BCP_warmstart_basis*>(ws);
   if (wsb) {
      const int type = 1;
      buf.pack(type);
      wsb->pack(buf);
      return;
   }

   const BCP_warmstart_dual* wsd =
      dynamic_cast<const BCP_warmstart_dual*>(ws);
   if (wsd) {
      const int type = 2;
      buf.pack(type);
      wsb->pack(buf);
      return;
   }

   const int type = 0;
   buf.pack(type);
}

BCP_warmstart*
BCP_unpack_warmstart(BCP_buffer& buf)
{
   int type;
   buf.unpack(type);
   switch (type) {
   case 0: return 0;
   case 1: return new BCP_warmstart_basis(buf);
   case 2: return new BCP_warmstart_dual(buf);
   default:
      throw BCP_fatal_error("Unknown warmstart in BCP_unpack_warmstart.\n");
   }
   return 0;
}
