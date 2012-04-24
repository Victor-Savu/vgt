#include <vgt/vector_field.h>
#include <vgt/vector_field_cls.h>

#include <math/vertex.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math/obj.h>
#include <vgt/scalar_field.h>
#include <vgt/scalar_field_cls.h>

VectorField vfCreate(   uint64_t x, uint64_t y, uint64_t z,
                        real dx, real dy, real dz)
{
    VectorField f = malloc(sizeof (struct VectorField));

    if (!f) {
        fprintf(stderr, "[x] Out of memory.\n");
        return 0;
    }

    if (x * y * z) {
        f->data = malloc(x * y * z * sizeof (Vertex));
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

    f->dx = dx;
    f->dy = dy;
    f->dz = dz;

    f->data_owner = f;

    return f;
}

void vfClear(VectorField s)
{
    if (!s) {
        fprintf(stderr, "[!] Tried to clear a null vector field.\n");
        return;
    }
    // TODO: DESTROY all the registered subfields
    if (s->data) {
        free(s->data);
        s->data = 0;
    }
    memset(s, 0, sizeof (struct VectorField));
}

VectorField vfCopy(VectorField s)
{
    VectorField c = oCopy(s, sizeof (struct VectorField));
    c->data = oCopy(s->data, s->nx * s->ny * s->nz * sizeof (Vertex));
    return c;
}

void vfDestroy(VectorField s)
{
    if (!s) {
        fprintf(stderr, "[!] Tried to destroy a null vector field.\n");
        return;
    }
    vfClear(s);
    free(s);
}

inline
Vertex* vfAt(const VectorField const restrict s, uint64_t x, uint64_t y, uint64_t z)
{
    return vfRel(s, s->data, x, y, z);
}

inline
Vertex* vfRelX(const VectorField const restrict field, Vertex* restrict e, int64_t x)
{
    usage(field);
    e += (int64_t)field->step_x * x;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}

inline
Vertex* vfRelY(const VectorField const restrict field, Vertex* restrict e, int64_t y)
{
    usage(field);
    e += (int64_t)field->step_y * y;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}

inline
Vertex* vfRelZ(const VectorField const restrict field, Vertex* restrict e, int64_t z)
{
    usage(field);
    e += (int64_t)field->step_z * z;
    usage(e >= field->data && e <= field->data + field->nz * field->step_z);
    return e;
}
inline
Vertex* vfRel(const VectorField const restrict field, Vertex* restrict e, int64_t x, int64_t y, int64_t z)
{
    return vfRelX(field, vfRelY(field, vfRelZ(field, e, z), y), x);
}

inline
Vertex* vfValue(const VectorField const restrict field, Vertex* v, real x, real y, real z)
{
    usage(field);

    if (    x<0 || y <0 || z < 0 ||
            x * field->dx >= (field->nx-1) ||
            y * field->dy >= (field->ny-1) ||
            z * field->dz >= (field->nz-1) ) {
        fprintf(stderr, "<x, y, z> : <%f, %f, %f>\n", (float)x, (float)y, (float)z);
        vSet(v, 0, 0, 0);
        return v;
    }

    usage(x >= 0 && x * field->dx < (field->nx-1));
    usage(y >= 0 && y * field->dy < (field->ny-1));
    usage(z >= 0 && z * field->dz < (field->nz-1));

    x /= field->dx; y /= field->dy; z /= field->dz;

    Vertex* cell = vfAt(field, oCast(uint64_t, x), oCast(uint64_t, y), oCast(uint64_t, z));
    x -= (uint64_t)x;  y -= (uint64_t)y;  z -= (uint64_t)z;

    usage ( oCast(uint64_t, x * field->dx) < (field->nx-1) || x==0 );
    usage ( oCast(uint64_t, y * field->dy) < (field->ny-1) || x==0 );
    usage ( oCast(uint64_t, z * field->dz) < (field->nz-1) || x==0 );

    Vertex v000; vCopy(cell, &v000); cell = vfRel(field, cell, 1, 0, 0);
    Vertex v100; vCopy(cell, &v100); cell = vfRel(field, cell, 0, 1, 0);
    Vertex v110; vCopy(cell, &v110); cell = vfRel(field, cell,-1, 0, 0);
    Vertex v010; vCopy(cell, &v010); cell = vfRel(field, cell, 0, 0, 1);
    Vertex v011; vCopy(cell, &v011); cell = vfRel(field, cell, 1, 0, 0);
    Vertex v111; vCopy(cell, &v111); cell = vfRel(field, cell, 0,-1, 0);
    Vertex v101; vCopy(cell, &v101); cell = vfRel(field, cell,-1, 0, 0);
    Vertex v001; vCopy(cell, &v001);

    vAddI(vScaleI(&v000, 1-z), vScaleI(&v001, z));
    vAddI(vScaleI(&v100, 1-z), vScaleI(&v101, z));
    vAddI(vScaleI(&v010, 1-z), vScaleI(&v011, z));
    vAddI(vScaleI(&v110, 1-z), vScaleI(&v111, z));

    vAddI(vScaleI(&v000, 1-y), vScaleI(&v010, y));
    vAddI(vScaleI(&v100, 1-y), vScaleI(&v110, y));

    vAddI(vScaleI(&v000, 1-x), vScaleI(&v100, x));

    return vCopy(&v000, v);
}


ScalarField vfDivergence(VectorField field)
{
    // TODO: Check preconditions

    //

    ScalarField lapl = sfCreate(field->nx, field->ny, field->nz, field->dx, field->dy, field->dz);

    real* s = sfAt(lapl, 0, 0, 0);
    Vertex* v = vfAt(field, 0, 0, 0);

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
                    nx = (vGetX(vfRel(field, v, 1, 0, 0)) - vGetX(v)) / dx * 1.0f;
                else if (x == field->nx - 1)
                    nx = (vGetX(v) - vGetX(vfRel(field, v, -1, 0, 0))) / dx * 1.0f;
                else
                    nx = (vGetX(vfRel(field, v, 1, 0, 0)) - vGetX(vfRel(field, v, -1, 0, 0))) / 2.0f / dx;

                if (y == 0)
                    ny = (vGetY(vfRel(field, v, 0, 1, 0)) - vGetY(v)) / dy * 1.0f;
                else if (y == field->ny - 1)
                    ny = (vGetY(v) - vGetY(vfRel(field, v, 0, -1, 0))) / dy * 1.0f;
                else
                    ny = (vGetY(vfRel(field, v, 0, 1, 0)) - vGetY(vfRel(field, v, 0, -1, 0))) / 2.0f / dy;

                if (z == 0)
                    nz = (vGetZ(vfRel(field, v, 0, 0, 1)) - vGetZ(v)) / dz * 1.0f;
                else if (z == field->nz - 1)
                    nz = (vGetZ(v) - vGetZ(vfRel(field, v, 0, 0, -1))) / dz * 1.0f;
                else
                    nz = (vGetZ(vfRel(field, v, 0, 0, 1)) - vGetZ(vfRel(field, v, 0, 0, -1))) / 2.0f / dz;

                *s = nx + ny + nz;

                v = vfRel(field, v, 1, 0, 0);
                s = sfRel(lapl, s, 1, 0, 0);
            }
        }
    }

    return lapl;
}
