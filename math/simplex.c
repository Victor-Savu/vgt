#include <math/simplex.h>

bool sxIsZeroSimplex(Simplex a)
{
    return (a) &&(a->pos) && (true);
}

bool sxIsOneSimplex(Simplex a)
{
    check(sxIsZeroSimplex(a));
    check(sxIsZeroSimplex(a->o[0]));
    return (a->o[0]->o[0] == a);
}

bool sxIsTwoSimplex(Simplex a)
{
    check(sxIsOneSimplex(a));
    check(sxIsOneSimplex(a->o[1]));
    check(sxIsOneSimplex(a->o[1]->o[1]));
    return (a->o[1]->o[1]->o[1] == a) && (a->o[0]->o[1]->o[1]->o[1] == a->o[0]);
}

bool sxIsThreeSimplex(Simplex a)
{
    check(sxIsTwoSimplex(a));
    check(sxIsTwoSimplex(a->o[2]));
    check(sxIsTwoSimplex(a->o[2]->o[2]));
    check(sxIsTwoSimplex(a->o[2]->o[2]->o[2]));
    return (a->o[2]->o[2]->o[2]->o[2] == a) &&
        (a->o[1]->o[2]->o[2]->o[2]->o[2] == a->o[1]) &&
        (a->o[1]->o[1]->o[2]->o[2]->o[2]->o[2] == a->o[1]->o[1]);
}

const Simplex sxZeroSimplex(Vec* pos)
{
    static struct Simplex s;
    s.pos = pos;
    s.o[0] = s.o[1] = s.o[2] = 0;
    return &s;
}

const Simplex sxOneSimplex(Simplex a, Simplex b)
{
    check(isZeroSimplex(a));
    check(isZeroSimplex(b));

    a->o[0] = b; b->o[0] = a;
    return a;
}

const Simplex sxTwoSimplex(Simplex a, Simplex b, Simplex c)
{
    check(sxIsOneSimplex(a));
    check(sxIsOneSimplex(b));
    check(sxIsOneSimplex(c));

    a->o[1] = b; b->o[1] = c; c->o[1] = a;
    a->o[0]->o[1] = b->o[0]; b->o[0]->o[1] = c->o[0]; c->o[0]->o[1] = a->o[0];
    return a;
}

const Simplex sxThreeSimplex(Simplex a, Simplex b, Simplex c, Simplex d)
{
    check(sxIsTwoSimplex(a));
    check(sxIsTwoSimplex(b));
    check(sxIsTwoSimplex(c));
    check(sxIsTwoSimplex(d));

//    a->o[2] = b; b->o[2] = c; c->o[2] = d; d->o[2] = a;
//    a->o[1]->o[2] = b->o[2]; b->o[2]->
    return a;
}
