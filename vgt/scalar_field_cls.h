#ifndef VGT_SCALAR_FIELD_CLS_H
#define VGT_SCALAR_FIELD_CLS_H

#include <vgt/types.h>

struct ScalarField {
    // the scalar data
    real* data;

    // the size of the field (number of elements) along each dimension
    uint64_t nx, ny, nz;

    // the distance to hop between dimensions
    uint64_t step_x, step_y, step_z;

    // step unit in each dimension
    real dx, dy, dz;

    ScalarField data_owner;

    // TODO: Add a binary search tree containing the subfields
};

#endif//VGT_SCALAR_FIELD_CLS_H
