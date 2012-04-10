#ifndef VGT_SCALAR_FIELD_H
#define VGT_SCALAR_FIELD_H

#include <vgt/types.h>


/*
   Allocates space and initialises a scalar field of size @x @y @z, with voxel size
   dx by dy by dz.
*/
ScalarField sfCreate(uint64_t x, uint64_t y, uint64_t z, real dx, real dy, real dz);

/*
   Reads in data for the scalar field from a RAW file.
*/
bool sfReadRaw(ScalarField s, const char* fname);

/*
   Clears the contents of the scalar field structure.

   If the data is owned by the structure, it is deallocated, and all
   of the registered subfields are destroyed.
*/
void sfClear(ScalarField s);

ScalarField sfCopy(ScalarField s);

/*
   Clears and then deallocates the field structure.
   After the execution, @s will point to an unallocated clunk of memory.
*/
void sfDestroy(ScalarField s);

/*
   Compute the gradient of a scalar field
*/
VectorField sfGradient(ScalarField field);

/*
   Compute the laplacian of a scalar field
*/
ScalarField sfLaplacian(ScalarField field);

/*
   Computes the interpolated value of a point in the scalar field
*/
real sfValue(ScalarField field, real x, real y, real z);

#endif//VGT_SCALAR_FIELD_H
