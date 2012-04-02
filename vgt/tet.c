#include <vgt/tet.h>
#include <vgt/tet_cls.h>

#include <stdio.h>
#include <math/vec.h>

inline
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

inline
TetFace tetReadMap(byte m, TetNeighbour n)
{
    return (m >> (n<<1)) & 3;
}

void tetPrint(Obj tet, FILE* f)
{
    Tet const t = tet;
    fprintf(f, "[");
    vPrint(t->v[0], f); fprintf(f, ", ");
    vPrint(t->v[1], f); fprintf(f, ", ");
    vPrint(t->v[2], f); fprintf(f, ", ");
    vPrint(t->v[3], f); fprintf(f, "]\n");
}
