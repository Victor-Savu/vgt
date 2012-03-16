#include <vgt/scalar_field.h>
#include <vgt/scalar_field_cls.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <vgt/vector_field.h>
#include <vgt/vec.h>

ScalarField sfCreate(   unsigned int x, unsigned int y, unsigned int z,
                        real dx, real dy, real dz)
{
    ScalarField f = malloc(sizeof (struct ScalarField));

    if (!f) {
        fprintf(stderr, "[x] Out of memory.\n");
        return 0;
    }

    if (x * y * z) {
        f->data = malloc(x * y * z * sizeof (real));
        if (!f->data) {
            fprintf(stderr, "[x] Out of memory.\n");
            free(f);
            return 0;
        }

    } else { // create an empty field
        f->data = 0;
    }

    f->nx = x;
    f->ny = y;
    f->nz = z;

    f->step_x = 1;
    f->step_y = x;
    f->step_z = x*y;

    f->data_owner = f;
    f->dx = dx;
    f->dy = dy;
    f->dz = dz;

    return f;
}

bool sfReadRaw(ScalarField s, const char* fname)
{
    if (!s) {
        fprintf(stderr, "[x] Tried to read RAW data from file [%s] into a null scalar field.\n", fname);
        return false;
    }

    FILE* fin = fopen(fname, "rb");
    if (!fin) {
       fprintf(stderr, "[x] File [%s]: %s\n", fname, strerror(errno));
       return false;
    }

    unsigned int nelem = s->nx * s->ny * s->nz;

    char* buffer = malloc(nelem * sizeof (char));
    if (!buffer) {
        fprintf(stderr, "[x] Out of memory!\n");
        fclose(fin);
        return false;
    }

    unsigned int bytes_read = 0;
    if ((bytes_read = fread(buffer, sizeof(char), nelem, fin) ) != nelem) {
        fprintf(stderr, "Read only [%u] out of [%u] bytes from file [%s].\n", bytes_read, nelem, fname);
        fclose(fin);
        return false;
    }

    char *c = 0;
    real *d = 0;
    for (c = buffer, d = s->data; c < buffer + nelem; c++, d++) *d = (real) *c / 255.0;

    free(buffer);

    fclose(fin);

    return true;
}

void sfClear(ScalarField s)
{
    if (!s) {
        fprintf(stderr, "[!] Tried to clear a null scalar field.\n");
        return;
    }

    // TODO: DESTROY all the registered subfields

    if (s->data) {
        free(s->data);
        s->data = 0;
    }
    memset(s, 0, sizeof (struct ScalarField));
}

void sfDestroy(ScalarField s)
{
    if (!s) {
        fprintf(stderr, "[!] Tried to destroy a null scalar field.\n");
        return;
    }
    sfClear(s);
    free(s);
}


real* sfAt(ScalarField s, unsigned int x, unsigned int y, unsigned int z)
{
    if (x > s->nx || y > s->ny || z > s->nz) {
        fprintf(stderr, "[x] Scalar field access out of bounds.\n");
        exit(EXIT_FAILURE);
    }
    return s->data + z * s->step_z + y * s->step_y + x * s->step_x;
}

real* sfRel(ScalarField s_field, real* e, int x, int y, int z)
{
    // TODO: Check preconditions & access range

    //

    return e + (int)s_field->step_x * x + (int)s_field->step_y * y + (int)s_field->step_z * z;
}

VectorField sfGradient(ScalarField field)
{
    // TODO: Check preconditions

    //

    VectorField grad = vfCreate(field->nx, field->ny, field->nz, field->dx, field->dy, field->dz);

    real* s = sfAt(field, 0, 0, 0);
    Vec v = vfAt(grad, 0, 0, 0);

    real dx = field->dx;
    real dy = field->dy;
    real dz = field->dz;

    real nx, ny, nz;
    uint x, y, z;
    for (z = 0; z < field->nz; z++) {
        for (y = 0; y < field->ny; y++) {
            for (x = 0; x < field->nx; x++)
            {
                if (x == 0)
                    nx = (*sfRel(field, s, 1, 0, 0) - *s) / dx * 1.0f;
                else if (x == field->nx - 1)
                    nx = (*s - *sfRel(field, s, -1, 0, 0)) / dx * 1.0f;
                else
                    nx = (*sfRel(field, s, 1, 0, 0) - *sfRel(field, s, -1, 0, 0)) / 2.0f / dx;

                if (y == 0)
                    ny = (*sfRel(field, s, 0, 1, 0) - *s) / dy * 1.0f;
                else if (y == field->ny - 1)
                    ny = (*s - *sfRel(field, s, 0, -1, 0)) / dy * 1.0f;
                else
                    ny = (*sfRel(field, s, 0, 1, 0) - *sfRel(field, s, 0, -1, 0)) / 2.0f / dy;

                if (z == 0)
                    nz = (*sfRel(field, s, 0, 0, 1) - *s) / dz * 1.0f;
                else if (z == field->nz - 1)
                    nz = (*s - *sfRel(field, s, 0, 0, -1)) / dz * 1.0f;
                else
                    nz = (*sfRel(field, s, 0, 0, 1) - *sfRel(field, s, 0, 0, -1)) / 2.0f / dz;

                vSet(nx, ny, nz, v);

                v = vfRel(grad, v, 1, 0, 0);
                s = sfRel(field, s, 1, 0, 0);
            }
        }
    }

    return grad;
}



/*
   Laplacian of a scalar field is computed by convolution with the kernel:

   [[[ 0, 0, 0],
     [ 0, 1, 0],
     [ 0, 0, 0]],

    [[ 0, 1, 0],
     [ 1,-6, 1],
     [ 0, 1, 0]],

    [[ 0, 0, 0],
     [ 0, 1, 0],
     [ 0, 0, 0]]]
*/
ScalarField sfLaplacian(ScalarField field)
{
    // TODO: Check preconditions

    //

    ScalarField lapl = sfCreate(field->nx, field->ny, field->nz, field->dx, field->dy, field->dz);

    real* s = sfAt(lapl, 0, 0, 0);
    real* l = sfAt(field, 0, 0, 0);

    uint x, y, z;
    for (z = 0; z < field->nz; z++) {
        for (y = 0; y < field->ny; y++) {
            for (x = 0; x < field->nx; x++)
            {
                *l = -6.0 * (*s);

                if (x != 0) *l += (*sfRel(field, s, -1, 0, 0));
                if (x != field->nx -1) *l += (*sfRel(field, s, 1, 0, 0));

                if (y != 0) *l += (*sfRel(field, s,  0,-1, 0));
                if (y != field->ny -1) *l += (*sfRel(field, s, 0, 1, 0));

                if (z != 0) *l += (*sfRel(field, s,  0, 0,-1));
                if (z != field->nz -1) *l += (*sfRel(field, s, 0, 0, 1));

                l = sfRel(field, l, 1, 0, 0);
                s = sfRel(lapl, s, 1, 0, 0);
            }
        }
    }

    return lapl;
}
