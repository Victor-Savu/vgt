#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <math/types.h>
#include <stddef.h>



void oShortPrint(Obj i, FILE* f);
void oIntPrint(Obj i, FILE* f);
void oLongPrint(Obj i, FILE* f);
void oUshortPrint(Obj i, FILE* f);
void oUintPrint(Obj i, FILE* f);
void oUlongPrint(Obj i, FILE* f);
void oFloatPrint(Obj i, FILE* f);
void oDoublePrint(Obj i, FILE* f);
void oRealPrint(Obj i, FILE* f);
void oLongDoublePrint(Obj i, FILE* f);


/*
   Allocates an object. In case an allocation error occurs, the method
   issues a message and terminates the program.

   @arg size is the size of the object in bytes.
*/
Obj oCreate(size_t size) __attribute__((alloc_size(1)));

/*
   Frees the memory allocated for an object.

   @arg o is the object handle. After the execution, the handle points to an unallocated address.
*/
void oDestroy(Obj restrict o);

/*
   Allocates memory and creates a shallow copy of an object.
   @arg o is the object to be copied.
   @arg size is the size of the object in bytes.
*/
Obj oCopy(Obj restrict o, size_t size) __attribute__((alloc_size(2)));

Obj oCopyTo(Obj restrict dst, Obj restrict src, size_t size);

#define oCast(type, o)  ((type)(o))

#endif//VGT_OBJ_H
