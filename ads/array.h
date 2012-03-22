#ifndef VGT_ARRAY_H
#define VGT_ARRAY_H

#include <vgt/types.h>

typedef struct Array *Array;

/*
   Creates an array of objects.

   @arg size is the number of objects in the array.

   @return a pointer to the array.
*/
Array arrCreate(uint size);

/*
   Destroys an array.

   @arg arr is a pointer to the array to be destroyed.
   Be warned that following the operation, this will
   point to an unallocated address in memory.
*/
void arrDestroy(Array arr);

/*
   Retrieves an element from the array.

   @arg pos is the position of the element to be retrieved.

   @return a pointer to the element if found, or 0 if pos goes
   beyond the array limits.
*/
Obj arrGet(Array arr, uint pos);

/*
   Retrieves the beginning of the array.

   @return a pointer to the beginning of the array. At this
   memory location lies the pointer to the first element in
   the array. Subsequent element pointers can be obtained by
   forward iteration (++).
*/
Obj* arrBegin(Array arr);

/*
   Retrieves the end of the array.

   @return a pointer to the end of the array. This
   memory location lies right after the last entry in the array
   (i.e. after the pointer to the last element in  the array.
   The last element pointer (and the ones preceding that) obtained
   by backward iteration (--).
*/
Obj* arrEnd(Array arr);

#endif//VGT_ARRAY_H
