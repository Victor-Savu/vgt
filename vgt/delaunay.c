#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/predicates.h>

#include <GL/glut.h>

static
Array ins3(Delaunay del, Tet t, Vertex* p)
{
    p = arrPush(del->v, p);

    // new tets
    struct Tet tet_b = {{p, t->v[A], t->v[D], t->v[C]}, {0, 0, 0, 0}, 0};
    struct Tet tet_c = {{p, t->v[A], t->v[B], t->v[D]}, {0, 0, 0, 0}, 0};
    struct Tet tet_d = {{p, t->v[A], t->v[C], t->v[B]}, {0, 0, 0, 0}, 0};
    t->v[A] = p;

    Tet b = arrPush(del->t, &tet_b);
    tetConnect(b, oA, t->n[oB], tetReadMap(t->m, oB));

    Tet c = arrPush(del->t, &tet_c);
    tetConnect(c, oA, t->n[oC], tetReadMap(t->m, oC));
    tetConnect(c, oC, b, oD);

    Tet d = arrPush(del->t, &tet_d);
    tetConnect(d, oA, t->n[oD], tetReadMap(t->m, oD));
    tetConnect(d, oD, b, oC);
    tetConnect(d, oC, c, oD);

    tetConnect(b, oB, t, oB);
    tetConnect(c, oB, t, oC);
    tetConnect(d, oB, t, oD);

    Array stack = arrCreate(sizeof(Tet), 2);
    arrPush(stack, &t);
    arrPush(stack, &b);
    arrPush(stack, &c);
    arrPush(stack, &d);

    // check vertices
    check(t->v[A] == p);
    check(b->v[A] == p);
    check(c->v[A] == p);
    check(d->v[A] == p);

    // check neighbors
    TetNeighbour n = A;
    for (n = A; n<=D; n++) {
        if (t->n[n]) conjecture(t->n[n]->n[tetReadMap(t->m, n)] == t, "");
        if (b->n[n]) conjecture(b->n[n]->n[tetReadMap(b->m, n)] == b, "");
        if (c->n[n]) conjecture(c->n[n]->n[tetReadMap(c->m, n)] == c, "");
        if (d->n[n]) conjecture(d->n[n]->n[tetReadMap(d->m, n)] == d, "");
    }

    // check that the tetrahedra have the right orientation
    check(orient3d(*t->v[A], *t->v[B], *t->v[C], *t->v[D]) > 0);
    check(orient3d(*b->v[A], *b->v[B], *b->v[C], *b->v[D]) > 0);
    check(orient3d(*c->v[A], *c->v[B], *c->v[C], *c->v[D]) > 0);
    check(orient3d(*d->v[A], *d->v[B], *d->v[C], *d->v[D]) > 0);

    //stub;
    return stack;
}

static
Array ins2(Delaunay del, Tet t, Vertex* p, enum TetFacet f)
{
    p = arrPush(del->v, p);

    if (f == oA) { // rotate around D
        Obj swap = 0;
        // swap vertices
        swap = t->v[A]; t->v[A] = t->v[D]; t->v[D] = swap;
        swap = t->v[C]; t->v[C] = t->v[B]; t->v[B] = swap;
        // swap neighbors
        swap = t->n[A]; t->n[A] = t->n[D]; t->n[D] = swap;
        swap = t->n[C]; t->n[C] = t->n[B]; t->n[B] = swap;
        // notify neighbors
        byte m = t->m;
        tetConnect(t, oA, t->n[oA], tetReadMap(m, oD));
        tetConnect(t, oB, t->n[oB], tetReadMap(m, oC));
        tetConnect(t, oC, t->n[oC], tetReadMap(m, oB));
        tetConnect(t, oD, t->n[oD], tetReadMap(m, oA));
        f = oD;
    }

    Tet o = t->n[f];
    TetFace g = tetReadMap(t->m, f);


    Tet x, y;
    {
        struct Tet tmp = {{p, t->v[A], t->v[f], t->v[(f+3-(f==oB))&3]}, {0, 0, 0, 0}, 0};
        x = arrPush(del->t, &tmp);
        tetConnect(x, oA, t->n[(f+1+(f==oD))&3], tetReadMap(t->m, (f+1+(f==oD))&3));
        tetConnect(x, oB, t, (f+1+(f==oD))&3);
    }
    {
        struct Tet tmp = {{p, t->v[(f+1+(f==oD))&3], t->v[f], t->v[A]}, {0, 0, 0, 0}, 0};
        y = arrPush(del->t, &tmp);
        tetConnect(y, oA, t->n[(f+3-(f==oB))&3], tetReadMap(t->m, (f+3-(f==oB))&3));
        tetConnect(y, oD, t, (f+3-(f==oB))&3);
    }
    tetConnect(x, oD, y, oB);

    t->v[A] = p;

    Array stack = arrCreate(sizeof(Tet), 2);
    arrPush(stack, &t);
    arrPush(stack, &x);
    arrPush(stack, &y);

    if (o) {
        if (g == oA) { // if it's degenerate, swap it to normal
            Obj swap = 0;
            // swap vertices
            swap = o->v[A]; o->v[A] = o->v[D]; o->v[D] = swap;
            swap = o->v[C]; o->v[C] = o->v[B]; o->v[B] = swap;
            // swap neighbors
            swap = o->n[A]; o->n[A] = o->n[D]; o->n[D] = swap;
            swap = o->n[C]; o->n[C] = o->n[B]; o->n[B] = swap;
            // notify neighbors
            byte m = o->m;
            tetConnect(o, oA, o->n[oA], tetReadMap(m, oD));
            tetConnect(o, oB, o->n[oB], tetReadMap(m, oC));
            tetConnect(o, oC, o->n[oC], tetReadMap(m, oB));
            g = oD;
        }

        Tet xx, yy;
        {
            struct Tet tmp = {{p, o->v[A], o->v[g], o->v[(g+3-(g==oB))&3]}, {0, 0, 0, 0}, 0};
            xx = arrPush(del->t, &tmp);
            tetConnect(xx, oA, o->n[(g+1+(g==oD))&3], tetReadMap(o->m, (g+1+(g==oD))&3));
            tetConnect(xx, oB, o, (g+1+(g==oD))&3);
        }
        {
            struct Tet tmp = {{p, o->v[(g+1+(g==oD))&3], o->v[g], o->v[A]}, {0, 0, 0, 0}, 0};
            yy = arrPush(del->t, &tmp);
            tetConnect(yy, oA, o->n[(g+3-(g==oB))&3], tetReadMap(o->m, (g+3-(g==oB))&3));
            tetConnect(yy, oD, o, (g+3-(g==oB))&3);
        }
        tetConnect(xx, oD, yy, oB);

        o->v[A] = p;

        if (t->v[(f+3-(f==oB))&3] == o->v[(g+1+(g==oD))&3]) {
            tetConnect(x, C, yy, C);
            tetConnect(t, f,  o, g);
            tetConnect(y, C, xx, C);
        } else  if (t->v[(f+3-(f==oB))&3] == o->v[(g+3-(g==oB))&3]) {
            tetConnect(x, C,  o, g);
            tetConnect(t, f, xx, C);
            tetConnect(y, C, yy, C);
        } else {
            tetConnect(x, C, xx, C);
            tetConnect(t, f, yy, C);
            tetConnect(y, C,  o, g);
        }

        arrPush(stack, &o);
        arrPush(stack, &xx);
        arrPush(stack, &yy);


        // check vertices
        check(o->v[A] == p);
        check(xx->v[A] == p);
        check(yy->v[A] == p);

        // check neighbors
        TetNeighbour n = A;
        for (n = A; n<=D; n++) {
            if (o->n[n]) conjecture(o->n[n]->n[tetReadMap(o->m, n)] == o, "");
            if (x->n[n]) conjecture(x->n[n]->n[tetReadMap(x->m, n)] == x, "");
            if (y->n[n]) conjecture(y->n[n]->n[tetReadMap(y->m, n)] == y, "");
        }

        // check vertices
        check(o->v[A] == p);
        check(xx->v[A] == p);
        check(yy->v[A] == p);

        // check neighbors
        n = A;
        for (n = A; n<=D; n++) {
            if (o->n[n]) conjecture(o->n[n]->n[tetReadMap(o->m, n)] == o, "");
            if (xx->n[n]) conjecture(xx->n[n]->n[tetReadMap(xx->m, n)] == xx, "");
            if (yy->n[n]) conjecture(yy->n[n]->n[tetReadMap(yy->m, n)] == yy, "");
        }

    }

    //arrPrint(del->t, stdout, tetPrint);

    stub;
    return stack;
}


/*static
void orient(Delaunay del, Tet t,  Vertex* p, enum TetEdge e)
{
    // see Fig #1
    // find x, y, z and t, where xy = e
    //Vertex *x, *y, *z, *t;
   // Tet oX, oY, oZ, oT;

    switch(e) {
    case BD:
    case DC:
    case CB:
        tetRot(t, (e << (e&1))|(e&4)); // lambda BD => C | DC => B | CB => D
        e = (e^1)&3; // lambda BD => AD | DC => AC | CB => AB
    default:
        break;
    }

    // lambda AD => <C, B> | AC => <B, D> | AB => <D, C>
    TetVertex base[2] = { ((e^1)<<1)|(e!=0), ((e>>1)^1)|(e<<(e&1)) };

    struct Tet tmp = {
        {p, A, t->v[base[0]], t->v[base[1]]},
        {0, 0, 0, 0},
        0};

    Tet o = arrPush(del->t, &tmp);
//    ignore arrPush(stack, o);

    // lambda AD => oD | AC => oC | AB => oB
    tetConnect(o, oA, t->n[e^3], tetReadMap(t->m, e^3));
    tetConnect(o, e^3, t, e^3);

    //split(t->


    //ignore(x && y && z && t);
    // split t, create o = { }



    p = arrPush(del->v, p);
    Array stack = arrCreate(sizeof(Tet), 2);


    stub;

    return stack;
}
*/

static
Array ins1(Delaunay del, Tet t, Vertex* p, enum TetEdge e)
{
    stub;
    return ins3(del, t, p);
}

static
Array flip23(Delaunay del, Tet t, Array stack)
{
    stub;
    return stack;
}

static
Array flip32(Delaunay del, Array stack)
{
    stub;
    return stack;
}

static
Array flip44(Delaunay del, Tet t, TetVertex tV, Array stack)
{
    Tet restrict tb = t->n[tV];
    if (!tb) {
        fprintf(stderr, "[!] No tb.\n"); fflush(stderr);
        arrPush(stack, &t);
        return stack;
    }

    Tet restrict tc = tb->n[oA];
    if (!tc) {
        fprintf(stderr, "[!] No tc.\n"); fflush(stderr);
        arrPush(stack, &t);
        return stack;
    }

    Tet restrict ta = t->n[oA];

    if (tc != ta->n[tetVertexLabel(ta, t->v[tV])]) {
     //   fprintf(stderr, "[!] tc and ta are not neighbors.\n"); fflush(stderr);
        arrPush(stack, &t);
        return stack;
    }

    // we know that tU = A
    const TetVertex tW = (tV + 1 + (tV == D))&3;
    const TetVertex tY = (tW + 1 + (tW == D))&3;

    // t
    // aX is opposite to the common face with t
    const TetVertex taX = tetReadMap(t->m, A);
    const TetVertex taY = tetVertexLabel(ta, t->v[tY]);
    const TetVertex taW = tetVertexLabel(ta, t->v[tW]);

    // tbU = A
    // tbZ is opposite to the common face with t
    const TetVertex tbZ = tetReadMap(t->m, tV);
    const TetVertex tbY = (tbZ + 1 + (tbZ == D))&3;
    const TetVertex tbW = (tbY + 1 + (tbY == D))&3;

    const TetVertex tcY = tetVertexLabel(tc, t->v[tY]);
    const TetVertex tcW = tetVertexLabel(tc, t->v[tW]);

    Vertex* const U = t->v[A];
    Vertex* const V = t->v[tV];
    Vertex* const W = t->v[tW];
    Vertex* const X = ta->v[taX];
    Vertex* const Y = ta->v[taY];
    Vertex* const Z = tb->v[tbZ];

    if (orient3d(*U, *V, *Y, *Z) > 0) {
        fprintf(stderr, "[!] Concave.\n"); fflush(stderr);
        arrPush(stack, &t);
        return stack;
    }

    if (orient3d(*U, *W, *V, *Z) > 0) {
        fprintf(stderr, "[!] Concave.\n"); fflush(stderr);
        arrPush(stack, &t);
        return stack;
    }

    Tet const taoW = ta->n[taW]; const TetFace taoWm = tetReadMap(ta->m, taW);
    Tet const tcoW = tc->n[tcW]; const TetFace tcoWm = tetReadMap(tc->m, tcW);

    // moving vertices
    ta->v[A] = t->v[A];   tc->v[A] = t->v[A];
    ta->v[B] = Y;   tc->v[B] = Z;
    ta->v[C] = V;   tc->v[C] = Y;
    ta->v[D] = X;   tc->v[D] = X;

    t->v[tY] = X;   tb->v[tbY] = X;

    // 9 connects
    tetConnect(t, oA, ta->n[taY], tetReadMap(ta->m, taY));
    tetConnect(tb, oA, tc->n[tcY], tetReadMap(tc->m, tcY));

    tetConnect(ta, oD, t->n[tW], tetReadMap(t->m, tW));
    tetConnect(tc, oD, tc->n[tW], tetReadMap(tc->m, tcW));

    tetConnect(t, tW, ta, B);
    tetConnect(tb, tbW, tc, C);
    tetConnect(ta, C, tc, B);

    tetConnect(ta, A, taoW, taoWm);
    tetConnect(tc, A, tcoW, tcoWm);


    arrPush(stack, &t);
    arrPush(stack, &ta);
    arrPush(stack, &tb);
    arrPush(stack, &tc);


    // checkping flip:

    // all A vertices are the same
    conjecture(t->v[A] == ta->v[A], "");
    conjecture(ta->v[A] == tb->v[A], "");
    conjecture(tb->v[A] == tc->v[A], "");

    // connectivity among themselves
    conjecture(t->n[tV] == tb, "");
    conjecture(tb->n[tbW] == tc, "");
    conjecture(tc->n[B] == ta, "");
    conjecture(ta->n[B] == t, "");

    conjecture(t->n[tW] == ta, "");
    conjecture(ta->n[C] == tc, "");
    conjecture(tc->n[C] == tb, "");
    conjecture(tb->n[tbZ] == t, "");

    // connectivity with neighbors
    TetNeighbour n = A;
    for (n = A; n<=D; n++) {
        if (t->n[n]) conjecture(t->n[n]->n[tetReadMap(t->m, n)] == t, "");
        if (ta->n[n]) conjecture(ta->n[n]->n[tetReadMap(ta->m, n)] == ta, "");
        if (tb->n[n]) conjecture(tb->n[n]->n[tetReadMap(tb->m, n)] == tb, "");
        if (tc->n[n]) conjecture(tc->n[n]->n[tetReadMap(tc->m, n)] == tc, "");
    }


    // check that the tetrahedra have the right orientation
    check(orient3d(*t->v[A], *t->v[B], *t->v[C], *t->v[D]) > 0);
    check(orient3d(*ta->v[A], *ta->v[B], *ta->v[C], *ta->v[D]) > 0);
    check(orient3d(*tb->v[A], *tb->v[B], *tb->v[C], *tb->v[D]) > 0);
    check(orient3d(*tc->v[A], *tc->v[B], *tc->v[C], *tc->v[D]) > 0);


    check(orient3d(*t->v[D], *t->v[B], *t->v[C], *t->v[A]) < 0);
    check(orient3d(*ta->v[D], *ta->v[B], *ta->v[C], *ta->v[A]) < 0);
    check(orient3d(*tb->v[D], *tb->v[B], *tb->v[C], *tb->v[A]) < 0);
    check(orient3d(*tc->v[D], *tc->v[B], *tc->v[C], *tc->v[A]) < 0);


    stub;
    return stack;
}
/*
void flip12()
{
    stub;
}

void flip21()
{
    stub;
}
*/

static
void flip(Delaunay del, Array stack)
{
    while (!arrIsEmpty(stack)) {
        arrRandomSwap(stack, 0);
        Tet t = *oCast(Tet*, arrBack(stack));
        arrPop(stack);

        Tet ta = t->n[oA];

        if (!ta) continue;

        conjecture((t->n[oB])?(t->n[oB]->v[0] == t->v[0]):(1), "there is a tet incident in p with vertex A not in p.");
        conjecture((t->n[oC])?(t->n[oC]->v[0] == t->v[0]):(1), "there is a tet incident in p with vertex A not in p.");
        conjecture((t->n[oD])?(t->n[oD]->v[0] == t->v[0]):(1), "there is a tet incident in p with vertex A not in p.");
        // t = <A, B, C, D> = <p, a, b, c>
        real* const p = *t->v[A];
        real* const a = *t->v[B];
        real* const b = *t->v[C];
        real* const c = *t->v[D];
        // assume no degenerate triangles
        //   conjecture(orient3d(p, a, b, c) > 0, "Found a degenerate tetrahedron.");


        // ta = <A, B, C, D> = <X, Y, Z, d> where <X, Y, Z> is a circular permutation of <a, b, c>

        real* const d = *ta->v[tetReadMap(t->m, oA)];

        // if the opposing vertex of ta is in the circumsphere of t, apply one of the 3 flip operations
        if (insphere(p, a, b, c, d) > 0) {
            // determine which of the 4 cases we are in
            real o = orient3d(p, a, b, d);
            if (o < 0) {
                // d should lie strictly below other two faces
                conjecture(orient3d(p, b, c, d) > 0, "d does not lie below pbc.");
                conjecture(orient3d(p, c, a, d) > 0, "d does not lie below pca.");

                if (t->n[oD] && t->n[oD] == ta->n[tetVertexLabel(ta, t->v[D])]) {
                    conjecture(t->n[oD]->v[0] == t->v[0], "there is a tet incident on p with vertex A not in p.");
                    // perform a flip32 on t, ta and their common neighbor t->n[oD]
                    stack = flip32(del, stack);
                } else {
                    conjecture(0, "Cannot perform flip32 in case #2 because pdab does not exist.");
                }

            } else if (o > 0) {
                o = orient3d(p, b, c, d);
                if (o < 0) {
                    // d should lie strictly below pca
                    conjecture(orient3d(p, c, a, d) > 0, "d does not lie below pca");

                    if (t->n[oB] && t->n[oB] == ta->n[tetVertexLabel(ta, t->v[B])]) {
                        // perform a flip32 on t, ta and their common neighbor t->n[oB]
                        stack = flip32(del, stack);
                    } else {
                        conjecture(0, "Cannot perform flip32 in case #2 because pdbc does not exist.");
                    }

                } else if (o > 0) {
                    o = orient3d(p, c, a, d);

                    if (o < 0) {
                        if (t->n[oC] && t->n[oC] == ta->n[tetVertexLabel(ta, t->v[C])]) {
                            stack = flip32(del, stack);
                        } else {
                            conjecture(0, "Cannot perform flip32 in case #2 because pdca does not exist.");
                        }
                    } else if (o > 0) {
                        stack = flip23(del, t, stack);
                    } else {// o = 0  =>  d is on plane pca
                        stack = flip44(del, t, C, stack);
                    }

                } else {// o = 0  =>  d is on plane pbc
                    stack = flip44(del, t, B, stack);

                }
            } else {// o = 0  => d  is on plane pab
                stack = flip44(del, t, D, stack);
            }
        }
    }

    arrDestroy(stack);
    stub;
}


struct sph_check {
    Vertex* v;
    bool b;
};

inline static
void check_on_sphere(uint64_t i, Obj j, Obj k) {
    Tet t = oCast(Tet, j);
    struct sph_check* c = oCast(struct sph_check*, k);
    Vertex* v = c->v;
    real o = insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *v);
    if (o == 0) c->b = 1;
}

void delInsert(Delaunay d, Vertex* p)
{
    Tet t = arrFront(d->t);
    Tet ot = t;
    struct sph_check sc = {.v = p, .b = false };
    arrForEach(d->t, check_on_sphere, &sc);
   /* if (sc.b) {
        fprintf(stderr, "[!] Did not insert point.\n");fflush(stderr);
        return;
        } */

    // find the tetrahedron containing p

    real o;
    while (t) {
        // check ABC
        o = (t->n[oD] == ot)?(1.0):orient3d(*t->v[A], *t->v[B], *t->v[C], *p);
        if (o < 0) { ot = t; t = t->n[oD]; continue; }
        if (o == 0) {
            // p is on plane ABC

            // check ACD
            o = (t->n[oB] == ot)?(1.0):orient3d(*t->v[A], *t->v[C], *t->v[D], *p);
            if (o < 0) { ot = t; t = t->n[oB]; continue; }
            if (o == 0) {
                // p is on line AC

                // check ADB
                o = (t->n[oC] == ot)?(1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o < 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p coincides with A
                    return;
                } else {
                    // p is on half-line (AC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with C
                        return;
                    } else {
                        // P is on segment (AC)
                        flip(d, ins1(d, t, p, AC));
                        return;
                    }
                }
            } else {
                // P is on half-plane (CAB

                // check ADB
                o = (t->n[oC] == ot)?(1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o < 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is on half-line (AB

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with B
                        return;
                    } else {
                        // P is on segment (AB)
                        flip(d, ins1(d, t, p, AB));
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (CB)
                        flip(d, ins1(d, t, p, CB));
                        return;
                    } else {
                        // P is inside the face (ABC)
                        flip(d, ins2(d, t, p, ABC));
                        return;
                    }
                }

            }
        } else {

            // check ACD
            o = (t->n[oB] == ot)?(1.0):orient3d(*t->v[A], *t->v[C], *t->v[D], *p);
            if (o < 0) { ot = t; t = t->n[oB]; continue; }
            if (o == 0) {
                // p is on half plane (ACD

                // check ADB
                o = (t->n[oC] == ot)?(1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o < 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is on half-line (AD

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with D
                        return;
                    } else {
                        // P is on segment (AD)
                        flip(d, ins1(d, t, p, AD));
                        return;
                    }
                } else {
                    // p is inside the angle CAD

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (DC)
                        flip(d, ins1(d, t, p, DC));
                        return;
                    } else {
                        // P is on face (ACD)
                        flip(d, ins2(d, t, p, ACD));
                        return;
                    }
                }
            } else {

                // check ADB
                o = (t->n[oC] == ot)?(1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o < 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is inside the angle <)BAD

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (BD)
                        flip(d, ins1(d, t, p, BD));
                        return;
                    } else {
                        // P is inside of face (ADB)
                        flip(d, ins2(d, t, p, ADB));
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is inside face (BDC)
                        flip(d, ins2(d, t, p, BDC));
                        return;
                    } else {
                        // P is inside the tetrahedron
                        Array stack = ins3(d, t, p);
                        flip(d, stack);
                        return;
                    }
                }
            }
        }
    }

    fprintf(stderr, "p is outside of the tetrahedrization.\n");

}

bool delCheck(Delaunay d)
{
    pthread_mutex_lock(&d->mutex);
    uint64_t ntet = arrSize(d->t);
    uint64_t nvert = arrSize(d->v);
    uint64_t i, j;
    bool result = true;
    for (i=0; i<ntet; i++) {
        for (j=4; j<nvert; j++) {
            Tet t = oCast(Tet, arrGet(d->t, i));
            Vertex* v = oCast(Vertex*, arrGet(d->v, j));
            if (insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *v) > 0) result = false;
        }
    }
    pthread_mutex_unlock(&d->mutex);
    return result;
}

void delDisplay(Delaunay restrict d, int tet)
{
    tet = tet % arrSize(d->t);
    Tet t_sel = oCast(Tet, arrGet(d->t, tet));

    //static bool once = 0;
    unsigned int i = 0;
    unsigned int end = 0;
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);

    glColor4f(0.0, 0.0, 1.0, 1.0);

    pthread_mutex_lock(&d->mutex);
    end = arrSize(d->t);
    for (i=0; i<end; i++) {
        if (i == tet) continue;
        Tet t = oCast(Tet, arrGet(d->t, i));
        if (delIsBounding(d, t)) {
            glColor4f(0.0, 1.0, 0.0, 1.0);

            glBegin(GL_LINE_STRIP);
            glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
            glVertex3v(*(t->v[D]));   glVertex3v(*(t->v[C]));
            glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[D]));
            glEnd();

            glBegin(GL_LINES);
            glVertex3v(*(t->v[B]));   glVertex3v(*(t->v[C]));
            glEnd();

            glColor4f(1.0, 0.0, 0.0, 1.0);
        } else {
            glBegin(GL_LINE_STRIP);
            glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
            glVertex3v(*(t->v[D]));   glVertex3v(*(t->v[C]));
            glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[D]));
            glEnd();

            glBegin(GL_LINES);
            glVertex3v(*(t->v[B]));   glVertex3v(*(t->v[C]));
            glEnd();
        }
    }
/*
    glPointSize(5.0);
    glBegin(GL_POINTS);

    glColor4f(1.0, 0.0, 0.0, 1.0);
    end = arrSize(d->v);
    for (i=0; i<end; i++) {
        Vertex* v = oCast(Vertex*, arrGet(d->v, i));
        if (delIsOnBoundary(d, v)) {
            glColor4f(0.0, 1.0, 0.0, 1.0);
            glVertex3v(*v);
            glColor4f(1.0, 0.0, 0.0, 1.0);
        } else {
            glVertex3v(*v);
        }
    }
    glEnd();
*/

    glPointSize(10.0);
    glBegin(GL_POINTS);

    glColor4f(1.0, 0.0, 0.0, 1.0);
    end = arrSize(d->v);
    for (i=0; i<end; i++) {
        Vertex* v = oCast(Vertex*, arrGet(d->v, i));
        real o = insphere(*t_sel->v[0], *t_sel->v[1], *t_sel->v[2], *t_sel->v[3], *v);
        if (o > 0) {
            glVertex3v(*v);
        } else if (o == 0) {
            glColor4f(0.0, 0.0, 1.0, 1.0);
            glVertex3v(*v);
            glColor4f(1.0, 0.0, 0.0, 1.0);
        }
    }
    glEnd();

    //if (;
    glColor4f(0.0, 1.0, 1.0, 1.0);
    tetRenderCircumsphere(t_sel);
    glColor4f(1.0, 0.0, 0.0, 1.0);

    glEnable(GL_LIGHTING);
    tetRenderSolid(t_sel);


    pthread_mutex_unlock(&d->mutex);

}
