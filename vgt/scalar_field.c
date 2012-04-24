#include <vgt/scalar_field.h>
#include <vgt/scalar_field_cls.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <math/obj.h>
#include <vgt/vector_field.h>

ScalarField sfCreate(   uint64_t x, uint64_t y, uint64_t z,
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

ScalarField sfCopy(ScalarField s)
{
    ScalarField c = oCopy(s, sizeof (struct ScalarField));
    c->data = oCopy(s->data, s->nx * s->ny * s->nz * sizeof (real));
    return c;
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

    uint64_t nelem = s->nx * s->ny * s->nz;

    uint8_t* buffer = malloc(nelem * sizeof (uint8_t));
    if (!buffer) {
        fprintf(stderr, "[x] Out of memory!\n");
        fclose(fin);
        return false;
    }

    uint64_t bytes_read = 0;
    if ((bytes_read = fread(buffer, sizeof(char), nelem, fin) ) != nelem) {
        fprintf(stderr, "Read only [%lu] out of [%lu] bytes from file [%s].\n", bytes_read, nelem, fname);
        fclose(fin);
        return false;
    }

    uint8_t *c = 0;
    real *d = 0;
    for (c = buffer, d = s->data; c < buffer + nelem; c++, d++) *d = oCast(real, *c) / 255.0;

    free(buffer);

    fclose(fin);

    return true;
}

bool sfWriteRaw(ScalarField s, const char* fname)
{
    if (!s) {
        fprintf(stderr, "[x] Tried to write RAW data to file [%s] from a null scalar field.\n", fname);
        return false;
    }

    FILE* fout = fopen(fname, "wb");
    if (!fout) {
       fprintf(stderr, "[x] File [%s]: %s\n", fname, strerror(errno));
       return false;
    }

    uint64_t nelem = s->nx * s->ny * s->nz;

    uint8_t* buffer = malloc(nelem * sizeof (char));
    if (!buffer) {
        fprintf(stderr, "[x] Out of memory!\n");
        fclose(fout);
        return false;
    }

    uint8_t *c = 0;
    real *d = 0;

    real min = *sfMin(s);
    real ampl = *sfMax(s) - min;
    if (ampl == 0) ampl = 1.0;

    for (c = buffer, d = s->data; c < buffer + nelem; c++, d++) {
        *c = (int)(((*d - min) * 255.0)/ampl);
    }


    uint64_t bytes_written = 0;
    if ((bytes_written = fwrite(buffer, sizeof(char), nelem, fout) ) != nelem) {
        fprintf(stderr, "Wrote only [%lu] out of [%lu] bytes to file [%s].\n", bytes_written, nelem, fname);
        fclose(fout);
        return false;
    }

    free(buffer);

    fclose(fout);

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

VectorField sfGradient(ScalarField field)
{
    check(field);

    VectorField grad = vfCreate(field->nx, field->ny, field->nz, field->dx, field->dy, field->dz);

    real* s = sfAt(field, 0, 0, 0);
    Vertex* v = vfAt(grad, 0, 0, 0);

    real dx = field->dx;
    real dy = field->dy;
    real dz = field->dz;

    real nx, ny, nz;
    uint64_t x, y, z;
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

                (*v)[0] = nx;
                (*v)[1] = ny;
                (*v)[2] = nz;

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

    real* s = sfAt(field, 0, 0, 0);
    real* l = sfAt(lapl, 0, 0, 0);

    uint64_t x, y, z;
    for (z = 0; z < field->nz; z++) {
        for (y = 0; y < field->ny; y++) {
            for (x = 0; x < field->nx; x++)
            {
                if (x != 0) *l += (*sfRelX(field, s, -1)) - (*s);
                if (x != field->nx -1) *l += (*sfRelX(field, s, 1)) - (*s);

                if (y != 0) *l += (*sfRelY(field, s, -1)) - (*s);
                if (y != field->ny -1) *l += (*sfRelY(field, s, 1)) - (*s);

                if (z != 0) *l += (*sfRelZ(field, s, -1)) - (*s);
                if (z != field->nz -1) *l += (*sfRelZ(field, s, 1)) - (*s);



                s = sfRelX(field, s, 1);
                l = sfRelX(lapl, l, 1);
            }
        }
    }

    return lapl;
}

inline
real sfValue(const ScalarField const restrict field, real x, real y, real z)
{
    usage(field);
    
    if (    x<0 || y <0 || z < 0 ||
            x * field->dx >= (field->nx-1) ||
            y * field->dy >= (field->ny-1) ||
            z * field->dz >= (field->nz-1) ) {
        fprintf(stderr, "<x, y, z> : <%f, %f, %f>\n", (float)x, (float)y, (float)z);
        return 0;
    }


    usage(x >= 0 && x * field->dx < (field->nx-1));
    usage(y >= 0 && y * field->dy < (field->ny-1));
    usage(z >= 0 && z * field->dz < (field->nz-1));

    x /= field->dx; y /= field->dy; z /= field->dz;

    real* cell = sfAt(field, oCast(uint64_t, x), oCast(uint64_t, y), oCast(uint64_t, z));
    x -= (uint64_t)x;  y -= (uint64_t)y;  z -= (uint64_t)z;
    
    usage ( oCast(uint64_t, x * field->dx) < (field->nx-1) || x==0 );
    usage ( oCast(uint64_t, y * field->dy) < (field->ny-1) || x==0 );
    usage ( oCast(uint64_t, z * field->dz) < (field->nz-1) || x==0 );

    const real s000 = *cell; cell = sfRel(field, cell, 1, 0, 0);
    const real s100 = *cell; cell = sfRel(field, cell, 0, 1, 0);
    const real s110 = *cell; cell = sfRel(field, cell,-1, 0, 0);
    const real s010 = *cell; cell = sfRel(field, cell, 0, 0, 1);
    const real s011 = *cell; cell = sfRel(field, cell, 1, 0, 0);
    const real s111 = *cell; cell = sfRel(field, cell, 0,-1, 0);
    const real s101 = *cell; cell = sfRel(field, cell,-1, 0, 0);
    const real s001 = *cell;

    const real z00 = s001 * z + s000 * (1-z);
    const real z10 = s101 * z + s100 * (1-z);
    const real z01 = s011 * z + s010 * (1-z);
    const real z11 = s111 * z + s110 * (1-z);

    const real y0 = z01 * y + z00 * (1-y);
    const real y1 = z11 * y + z10 * (1-y);

    return (y1 * x + y0 * (1-x));
}


inline
real* sfAt(const ScalarField const restrict s, uint64_t x, uint64_t y, uint64_t z)
{
    return sfRel(s, s->data, x, y, z);
}

inline
real* sfRelX(const ScalarField const restrict field, real* restrict e, int64_t x)
{
    usage(field);
    e += (int64_t)field->step_x * x;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}

inline
real* sfRelY(const ScalarField const restrict field, real* restrict e, int64_t y)
{
    usage(field);
    e += (int64_t)field->step_y * y;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}

inline
real* sfRelZ(const ScalarField const restrict field, real* restrict e, int64_t z)
{
    usage(field);
    e += (int64_t)field->step_z * z;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}

inline
real* sfRel(const ScalarField const restrict field, real* restrict e, int64_t x, int64_t y, int64_t z)
{
    return sfRelX(field, sfRelY(field, sfRelZ(field, e, z), y), x);
}


inline
real* sfMin(const ScalarField const restrict s_field)
{
    real* m = s_field->data;
    real* i = s_field->data;
    uint64_t nelem = s_field->nx * s_field->ny * s_field->nz;
    while (nelem--) {
        if (*i < *m) m = i;
        i = sfRelX(s_field, i, 1);
    }
    return m;
}

inline
real* sfMax(const ScalarField const restrict s_field)
{
    real* m = s_field->data;
    real* i = s_field->data;
    uint64_t nelem = s_field->nx * s_field->ny * s_field->nz;
    while (nelem--) {
        if (*i > *m) m = i;
        i = sfRelX(s_field, i, 1);
    }
    return m;
}

