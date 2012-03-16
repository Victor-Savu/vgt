#ifndef VGT_OBJ_H
#define VGT_OBJ_H

#include <vgt/types.h>
#include <stddef.h>

/*
   Allocates an object. In case an allocation error occurs, the method
   issues a message and terminates the program.

   @arg size is the size of the object in bytes.
*/
Obj oCreate(size_t size);

/*
   Frees the memory allocated for an object.

   @arg o is the object handle. After the execution, the handle points to an unallocated address.
*/
void oDestroy(Obj o);

#define oCast(type, o)  ((type)(o))

#endif//VGT_OBJ_H
