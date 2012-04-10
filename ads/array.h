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
   @arg fact is one less than the logarithm of the number of elements per data segment used for performance tunning. The segment size will be elem_size << (1+fact). Use 0 for the least memory overhead.
   @return a pointer to the array.
*/
Array arrCreate(uint8_t elem_size, uint8_t fact);

/*
   Destroys an array.

   @arg arr is a pointer to the array to be destroyed.
   Be warned that following the operation, this will
   point to an unallocated address in memory.
*/
void arrDestroy(Array restrict arr);

/*
  Create a deep copy of an array.
*/
Array arrCopy(Array restrict arr);

/*
   Retrieves an element from the array.

   @arg pos is the position of the element to be retrieved.

   @return a pointer to the element if found, or 0 if pos goes
   beyond the array limits.
*/
Obj arrGet(Array restrict arr, uint64_t pos);

Obj arrFront(Array restrict arr);
Obj arrBack(Array restrict arr);

void arrForEach(Array restrict arr, ArrOperation op, Obj data);

bool arrIsEmpty(Array restrict arr);


Obj arrSet(Array restrict arr, Obj restrict o, uint64_t pos);

#define arrPush(array, object)   arrPushSafe((array), (object), sizeof (*(object)), __FILE__, __func__, __LINE__)
Obj arrPushSafe(Array restrict arr, Obj restrict o, size_t objsize, const char* filename, const char* funcname, int lineno);
void arrPop(Array restrict arr);

size_t arrElementSize(Array restrict arr);

uint64_t arrSize(Array restrict arr);

void printStatus(Array restrict arr);

void arrPrint(Array restrict a, FILE* f, ObjPrint print);

void arrRandomSwap(Array a, ObjRelocator relocate);

Obj arrToC(Array restrict arr);

#endif//ADS_ARRAY_H
