#ifndef VGT_TET_CLS_H
#define VGT_TET_CLS_H

#include <vgt/tet.h>
#include <math/obj.h>

struct Tet {
    Vertex* v[4];
    Tet n[4];
    // a map. Each neighbor of this tet sees it as either opposite to a (00), b (01), c (10), or d (11).
    byte m;
};

inline static
Tet tetCopy(Tet dest, Tet src) {
    oCopyTo(dest, src, sizeof(struct Tet));
    tetConnect(dest, A, dest->n[A], tetReadMap(dest->m, A));
    tetConnect(dest, B, dest->n[B], tetReadMap(dest->m, B));
    tetConnect(dest, C, dest->n[C], tetReadMap(dest->m, C));
    tetConnect(dest, D, dest->n[D], tetReadMap(dest->m, D));
    check(tetIsLegit(dest));
    if (dest->n[A]) check(tetIsLegit(dest->n[A]));
    if (dest->n[B]) check(tetIsLegit(dest->n[B]));
    if (dest->n[C]) check(tetIsLegit(dest->n[C]));
    if (dest->n[D]) check(tetIsLegit(dest->n[D]));
    return dest;
}

inline static
void tetConnect(Tet x, TetFace fx, Tet y, TetFace fy)
{
    if (x) {
        x->n[fx] = y;
        // erase the old note in the map for neighbor in fx
        x->m &= 0xff ^ (3 << (fx << 1));
        // and set the new value
        x->m |= fy << (fx << 1);
    }
    if (y) {
        y->n[fy] = x;
        // erase the old note in the map for neighbor in fx
        y->m &= 0xff ^ (3 << (fy << 1));
        // and set the new value
        y->m |= fx << (fy << 1);
    }
}

inline static
TetFace tetReadMap(byte m, TetNeighbour n)
{
    return (m >> (n<<1)) & 3;
}

inline static
TetVertex tetVertexLabel(Tet t, Vertex * v)
{
    if (v == t->v[0]) return A;
    if (v == t->v[1]) return B;
    if (v == t->v[2]) return C;
    if (v == t->v[3]) return D;
 //   conjecture(0, "Requested label of a vertex which is not part of t.");
    return INVALID_FACET;
}


#endif//VGT_TET_CLS_H
