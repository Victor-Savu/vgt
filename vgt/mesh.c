#include <vgt/mesh.h>
#include <vgt/mesh_cls.h>

#include <vgt/edge_cls.h>
#include <math/obj.h>
#define EMPTY_MESH  { 0, 0, 0, 0}

Mesh mCopy(Mesh restrict m)
{
    if (!m) return 0;

    Mesh restrict c = oCopy(m, sizeof(struct Mesh));
    c->vertices = oCopy(m->vertices, c->n_vertices * sizeof(Vec));
    c->edges = oCopy(m->edges, c->n_edges * sizeof(struct Edge));

    return c;
}

void mDestroy(Mesh restrict m)
{
    if (!m) return;
    oDestroy(m->vertices);
    oDestroy(m->edges);
    oDestroy(m);
}

Mesh mReadOff(Mesh restrict m, const char* restrict filename)
{

}
