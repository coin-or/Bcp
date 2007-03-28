// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
/* This gnerator generates $G_{grid}$ :
   An n-by-n grid with edge weigths chosen uniformly from {-maxw,+maxw}.
   (For now we'll solve this problem by treating it as a complete graph. Sort
   of inefficient...)
*/

#include <cstdio>
#include <cstdlib>
#include <algorithm>

int
main(int argc, char* argv[])
{
   if (argc != 4) {
      printf("Usage: gen4 <nodenum> <maxweigth> <seed>\n");
      return -1;
   }

   const int num  = atoi(argv[1]);
   const int maxw = atoi(argv[2]);
   const int seed = atoi(argv[3]);

   srand48(seed);

   printf("%i %i\n", num*num, 2*num*num-2*num);

   for (int i = 0; i < num; ++i) {
      const int colstart = i * num + 1;
      for (int j = 0; j < num; ++j) {
	 const int w0 = static_cast<int>((2*maxw+1) * drand48()) - maxw;
	 printf("%3i %3i %i\n", colstart+j-1, colstart+j, w0);
	 const int w1 = static_cast<int>((2*maxw+1) * drand48()) - maxw;
	 printf("%3i %3i %i\n", colstart+j-1, colstart+j+num-1, w1);
      }
   }
   return 0;
}
