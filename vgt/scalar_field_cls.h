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

/*
   Accesses an element of the scalar field.
*/
static
real* sfAt(ScalarField s, uint64_t x, uint64_t y, uint64_t z);

/*
   Access an element of the scalar field relative to another element in the X direction.
*/
static
real* sfRelX(ScalarField s_field, real* e, int x);

/*
   Access an element of the scalar field relative to another element in the Y direction.
*/
static
real* sfRelY(ScalarField s_field, real* e, int y);

/*
   Access an element of the scalar field relative to another element in the Z direction.
*/
static
real* sfRelZ(ScalarField s_field, real* e, int z);

/*
   Access an element of the scalar field relative to another element.
*/
static
real* sfRel(ScalarField s_field, real* e, int x, int y, int z);


inline static
real* sfAt(ScalarField s, uint64_t x, uint64_t y, uint64_t z)
{
    return sfRel(s, s->data, x, y, z);
}

inline static
real* sfRelX(ScalarField field, real* e, int x)
{
    check(field);
    e += (int)field->step_x * x;
    check(e >= field->data && e < field->data + field->nz * field->step_z);
    return e;
}

inline static
real* sfRelY(ScalarField field, real* e, int y)
{
    check(field);
    e += (int)field->step_y * y;
    check(e >= field->data && e < field->data + field->nz * field->step_z);
    return e;
}

inline static
real* sfRelZ(ScalarField field, real* e, int z)
{
    check(field);
    e += (int)field->step_z * z;
    check(e >= field->data && e < field->data + field->nz * field->step_z);
    return e;
}

inline static
real* sfRel(ScalarField field, real* e, int x, int y, int z)
{
    return sfRelX(field, sfRelY(field, sfRelZ(field, e, z), y), x);
}




#endif//VGT_SCALAR_FIELD_CLS_H
