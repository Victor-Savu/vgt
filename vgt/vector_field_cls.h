#ifndef VGT_VECTOR_FIELD_CLS_H
#define VGT_VECTOR_FIELD_CLS_H

#include <vgt/types.h>

struct VectorField {
    // the vector data
    struct Vec* data;

    // the size of the field (number of elements) along each dimension
    uint nx, ny, nz;

    // the distance to hop between dimensions
    uint step_x, step_y, step_z;

    // step unit in each dimension
    real dx, dy, dz;

    VectorField data_owner;

    // TODO: Add a binary search tree containing the subfields
};

#endif//VGT_VECTOR_FIELD_CLS_H
