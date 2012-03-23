#ifndef VGT_VECTOR_FIELD_H
#define VGT_VECTOR_FIELD_H

#include <vgt/types.h>


/*
   Allocates space and initialises a vector field of size @x @y @z, with voxel size
   dx by dy by dz.
*/
VectorField vfCreate(uint64_t x, uint64_t y, uint64_t z, real dx, real dy, real dz);

/*
   Reads in data for the vector field from a RAW file.
*/
bool vfReadRaw(VectorField s, const char* fname);

/*
   Clears the contents of the vector field structure.

   If the data is owned by the structure, it is deallocated, and all
   of the registered subfields are destroyed.
*/
void vfClear(VectorField s);

/*
   Clears and then deallocates the field structure.
   After the execution, @s will point to an unallocated clunk of memory.
*/
void vfDestroy(VectorField s);

/*
   Accesses an element of the vector field.
*/
Vec* vfAt(VectorField s, uint64_t x, uint64_t y, uint64_t z);

/*
   Access an element of the scalar field relative to another element.
*/
Vec* vfRel(VectorField v_field, Vec* e, int x, int y, int z);

/*
   Computes the divergence of the vector field
*/
ScalarField vfDivergence(VectorField field);

#endif//VGT_VECTOR_FIELD_H
