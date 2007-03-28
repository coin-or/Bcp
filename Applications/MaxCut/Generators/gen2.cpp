// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
/* This gnerator generates $G_{-1/1,dens}$ :
   A graph with density $dens$ and the existing edges have weights uniformly
   from {-1,1}.
*/

#include <cstdio>
#include <cstdlib>
#include <algorithm>

int
main(int argc, char* argv[])
{
   if (argc != 4) {
      printf("Usage: gen2 <nodenum> <density> <seed>\n");
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
      if (drand48() >= .5)
	 printf("%3i %3i -1\n", i-1, j-1);
      else
	 printf("%3i %3i  1\n", i-1, j-1);
   }

   return 0;
}
