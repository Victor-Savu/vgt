#include <math/mesh.h>

bool sxIsZeroSimplex(Mesh a)
{
    check(a);
    return (a->pos) && (true);
}

bool sxIsOneSimplex(Mesh a)
{
    check(sxIsZeroSimplex(a));
    check(sxIsZeroSimplex(a->n[0]));
    return (a->n[0]->n[0] == a);
}

bool sxIsTwoSimplex(Mesh a)
{
    check(sxIsOneSimplex(a));
    check(sxIsOneSimplex(a->n[1]));
    check(sxIsOneSimplex(a->n[1]->n[1]));
    return (a->n[1]->n[1]->n[1] == a) && (a->n[0]->n[1]->n[1]->n[1] == a->n[0]);
}

bool sxIsThreeSimplex(Mesh a)
{
    check(sxIsTwoSimplex(a));
    check(sxIsTwoSimplex(a->n[2]));
    check(sxIsTwoSimplex(a->n[2]->n[2]));
    check(sxIsTwoSimplex(a->n[2]->n[2]->n[2]));
    return (a->n[2]->n[2]->n[2]->n[2] == a) &&
        (a->n[1]->n[2]->n[2]->n[2]->n[2] == a->n[1]) &&
        (a->n[1]->n[1]->n[2]->n[2]->n[2]->n[2] == a->n[1]->n[1]);
}

const Mesh sxZeroSimplex(Vec* pos)
{
    static struct Mesh s;
    s.pos = pos;
    s.n[0] = s.n[1] = s.n[2] = 0;
    return &s;
}

const Mesh sxOneSimplex(Mesh a, Mesh b)
{
    check(isZeroSimplex(a));
    check(isZeroSimplex(b));

    a->n[0] = b; b->n[0] = a;
    return a;
}

const Mesh sxTwoSimplex(Mesh a, Mesh b, Mesh c)
{
    check(sxIsOneSimplex(a));
    check(sxIsOneSimplex(b));
    check(sxIsOneSimplex(c));

    a->n[1] = b; b->n[1] = c; c->n[1] = a;
    a->n[0]->n[1] = a; b->n[0]->n[1] = b; c->n[0]->n[1] = c;
    return a;
}

const Mesh sxThreeSimplex(Mesh a, Mesh b, Mesh c, Mesh d)
{
    check(sxIsTwoSimplex(a));
    check(sxIsTwoSimplex(b));
    check(sxIsTwoSimplex(c));
    check(sxIsTwoSimplex(d));

    a->n[2] = b; b->n[2] = c; c->n[2] = d; d->n[2] = a;
    a->n[1]->n[2] = b->n[2]; b->n[2]->
    return a;
}

const Mesh mSimplex(Mesh* a, uint16_t dim)
{
    Mesh* m = a; uint16_t cnt = dim+1;
    while (cnt--) mIsSimplex(*m, cnt);
}
