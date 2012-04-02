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
   Accesses an element of the scalar field.
*/
real* sfAt(ScalarField s, uint64_t x, uint64_t y, uint64_t z);

/*
   Access an element of the scalar field relative to another element.
*/
real* sfRel(ScalarField s_field, real* e, int x, int y, int z);


/*
   Compute the gradient of a scalar field
*/
VectorField sfGradient(ScalarField field);

/*
   Compute the laplacian of a scalar field
*/
ScalarField sfLaplacian(ScalarField field);


#endif//VGT_SCALAR_FIELD_H
