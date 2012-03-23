#ifndef ADS_ARRAY_H
#define ADS_ARRAY_H

#include <ads/types.h>

/*
   This implementation of resizable arrays is based on the paper by Brodnik, et al.[1]

Version:
1.0     Implementing the Basic version presented in [1]


References:
[1] "Resizable Arrays in Optimal Time and Space" by Andrej Brodnik,
Svante Carlsson, Erik D. Demaine, J. Ian Munro, and Robert Sedgewick

 */

/*
   Creates an array of objects.

   @arg size is the size of an element in the array

   @return a pointer to the array.
*/
Array arrCreate(uint8_t block_size);

/*
   Destroys an array.

   @arg arr is a pointer to the array to be destroyed.
   Be warned that following the operation, this will
   point to an unallocated address in memory.
*/
void arrDestroy(Array restrict arr);

/*
   Retrieves an element from the array.

   @arg pos is the position of the element to be retrieved.

   @return a pointer to the element if found, or 0 if pos goes
   beyond the array limits.
*/
Obj arrGet(Array restrict arr, uint64_t pos);


void arrSet(Array restrict arr, Obj restrict o, uint64_t pos);
void arrPush(Array restrict arr, Obj restrict o);
void arrPop(Array restrict arr);
uint64_t arrSize(Array restrict arr);
void printStatus(Array restrict arr);

#endif//ADS_ARRAY_H
