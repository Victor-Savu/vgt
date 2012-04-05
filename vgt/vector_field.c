#include <vgt/vector_field.h>
#include <vgt/vector_field_cls.h>

#include <math/vertex.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math/obj.h>
#include <vgt/scalar_field.h>

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


Vertex* vfAt(VectorField s, uint64_t x, uint64_t y, uint64_t z)
{
    if (x >= s->nx || y >= s->ny || z >= s->nz) {
        fprintf(stderr, "[!] Vector field access out of bounds.\n");
        exit(EXIT_FAILURE);
    }
    return s->data + z * s->step_z + y * s->step_y + x * s->step_x;
}

Vertex* vfRel(VectorField v_field, Vertex* e, int x, int y, int z)
{
    // TODO: Check preconditions & access range

    //

    return e + (int)v_field->step_x * x + (int)v_field->step_y * y + (int)v_field->step_z * z;
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
