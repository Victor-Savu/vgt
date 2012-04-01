#include <vgt/tet.h>
#include <vgt/tet_cls.h>


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
