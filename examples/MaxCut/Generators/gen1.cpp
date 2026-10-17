// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
/* This gnerator generates $G_{dens}$ :
   A complete unweighted graph with edge probability $dens$.
   Note that we'll print -1's in the problem file since we treat minimization
   problems in BCP!
*/

#include <cstdio>
#include <cstdlib>
#include <algorithm>

int
main(int argc, char* argv[])
{
   if (argc != 4) {
      printf("Usage: gen1 <nodenum> <density> <seed>\n");
      return -1;
   }

   const int num  = atoi(argv[1]);
   const double dens = atof(argv[2]);
   const int seed = atoi(argv[3]);

   srand48(seed);

   const int maxedgenum = (num*(num-1)) / 2;
   int * pick = new int[maxedgenum];
   int k = 0;
   for (int i = 1; i < num; ++i) {
      for (int j = i + 1; j <= num; ++j) {
	 pick[k++] = (i << 16) + j;
      }
   }

   random_shuffle(pick, pick + maxedgenum);
   const int edgenum = static_cast<int>(maxedgenum * dens);

   printf("%i %i\n", num, edgenum);

   for (k = 0; k < edgenum; ++k) {
      const int i = pick[k] >> 16;
      const int j = pick[k] & 0xffff;
      printf("%3i %3i -1\n", i-1, j-1);
   }

   return 0;
}
