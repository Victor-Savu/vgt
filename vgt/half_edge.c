#include <vgt/half_edge.h>
#include <vgt/half_edge_cls.h>

HalfEdge heConst(Vec* v, HalfEdge n, HalfEdge o, HalfEdge f)
{
    static struct HalfEdge e;

    e.v = v;
    e.n = n;
    e.o = o;
    e.f = f;

    return &e;
}
