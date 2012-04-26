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
   Writes the data to a raw file
*/
bool sfWriteRaw(ScalarField s, const char* fname);

/*
   Clears the contents of the scalar field structure.

   If the data is owned by the structure, it is deallocated, and all
   of the registered subfields are destroyed.
*/
void sfClear(ScalarField s);

ScalarField sfCopy(ScalarField s);

bool sfInside(ScalarField field, real x, real y, real z);

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
real sfValue(const ScalarField const restrict field, real x, real y, real z);

/*
   Accesses an element of the scalar field.
*/
real* sfAt(ScalarField s, uint64_t x, uint64_t y, uint64_t z);

/*
   Access an element of the scalar field relative to another element in the X direction.
*/
real* sfRelX(const ScalarField const restrict s_field, real* restrict e, int64_t x);

/*
   Access an element of the scalar field relative to another element in the Y direction.
*/
real* sfRelY(const ScalarField const restrict s_field, real* restrict e, int64_t y);

/*
   Access an element of the scalar field relative to another element in the Z direction.
*/
real* sfRelZ(const ScalarField const restrict s_field, real* restrict e, int64_t z);

/*
   Access an element of the scalar field relative to another element.
*/
real* sfRel(const ScalarField const restrict s_field, real* restrict e, int64_t x, int64_t y, int64_t z);

/*
   Find the global minimum value and its position.
*/
real* sfMin(const ScalarField restrict s_field);

/*
   Find the global maximum value and its position.
*/
real* sfMax(const ScalarField restrict s_field);

#endif//VGT_SCALAR_FIELD_H
