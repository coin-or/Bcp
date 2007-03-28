// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
/* This gnerator generates $Q_{maxw,dens}$ :
*/

#include <cstdio>
#include <cstdlib>
#include <algorithm>

int
main(int argc, char* argv[])
{
   if (argc != 5) {
      printf("Usage: gen3 <nodenum> <density> <maxweight> <seed>\n");
      return -1;
   }

   int i, j, k;

   const int    num  = atoi(argv[1]);
   const double dens = atof(argv[2]);
   const int    maxw = atoi(argv[3]);
   const int    seed = atoi(argv[4]);

   srand48(seed);

   const int maxedgenum = (num*(num-1)) / 2;
   int * pick = new int[maxedgenum];
   k = 0;
   for (i = 0; i < num; ++i) {
      for (j = i + 1; j < num; ++j) {
	 pick[k++] = (i << 16) + j;
      }
   }

   random_shuffle(pick, pick + maxedgenum);
   const int edgenum = static_cast<int>(maxedgenum * dens);

   printf("%i %i\n", num + 1, num + edgenum);

   double *matrix = new double[num * num];
   for (i = 0; i < num; ++i)
      matrix[i*num+i] = (2*maxw+1)*drand48() - maxw;

   for (k = 0; k < edgenum; ++k) {
      const int i = pick[k] >> 16;
      const int j = pick[k] & 0xffff;
      matrix[i*num+j] = matrix[j*num+i] = (2*maxw+1)*drand48() - maxw;
   }
   for (; k < edgenum; ++k) {
      const int i = pick[k] >> 16;
      const int j = pick[k] & 0xffff;
      matrix[i*num+j] = matrix[j*num+i] = 0.0;
   }

   for (i = 0; i < num; ++i) {
      double sum = 2 * matrix[i*num+i];
      for (j = 0; j < i; ++j)
	 sum += matrix[i*num+j];
      for (j = i+1; j < num; ++j)
	 sum += matrix[i*num+j];
      matrix[i*num+i] = sum;
   }

   for (i = 0; i < num; ++i) {
      printf("%3i %3i %i\n", 0, i+1, static_cast<int>(matrix[i*num+i]));
   }
   for (i = 0; i < num; ++i) {
      for (j = i+1; j < num; ++j) {
	 if (matrix[i*num+j] != 0.0) {
	    printf("%3i %3i %i\n", i+1,j+1, static_cast<int>(matrix[i*num+j]));
	 }
      }
   }

   return 0;
}
