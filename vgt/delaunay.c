#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/predicates.h>

#include <GL/glut.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

static
Array ins3(Delaunay del, Tet t, Vertex* p)
{
    pthread_mutex_lock(&del->mutex);
    p = arrPush(del->v, p);
    pthread_mutex_unlock(&del->mutex);

    // new tets
    struct Tet tet_b = {{p, t->v[A], t->v[D], t->v[C]}, {0, 0, 0, 0}, 0};
    struct Tet tet_c = {{p, t->v[A], t->v[B], t->v[D]}, {0, 0, 0, 0}, 0};
    struct Tet tet_d = {{p, t->v[A], t->v[C], t->v[B]}, {0, 0, 0, 0}, 0};


    pthread_mutex_lock(&del->mutex);
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


    pthread_mutex_unlock(&del->mutex);

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
    check(tetIsLegit(t));
    check(tetIsLegit(b));
    check(tetIsLegit(c));
    check(tetIsLegit(d));

    TetVertex n = B;
    for (n = B; n<=D; n++) {
        if (t->n[n]) check(t->n[n]->v[A] == p);
        if (b->n[n]) check(b->n[n]->v[A] == p);
        if (c->n[n]) check(c->n[n]->v[A] == p);
        if (d->n[n]) check(d->n[n]->v[A] == p);
    }

    stub;
    return stack;
}

static
Array ins2(Delaunay del, Tet t, Vertex* p, enum TetFacet f)
{

    pthread_mutex_lock(&del->mutex);
    p = arrPush(del->v, p);

  //  printf("f = %d\n", f);
  //  printf("t = "); tetPrint(t, stdout); printf("\n"); fflush(stdout);

    if (f == oA) { // rotate around D
        tetRot(t, D);
        f = oB;
    }

 //   printf("f = %d\n", f);
 //   printf("t = "); tetPrint(t, stdout); printf("\n"); fflush(stdout);

    check(tetIsLegit(t));

    Tet o = t->n[f];
    TetFace g = tetReadMap(t->m, f);

    if (o) {
        if (g == oA) { // if it's degenerate, swap it to normal
            tetRot(o, D);
            g = oB;
        }
        check(tetIsLegit(o));
    }

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

    pthread_mutex_unlock(&del->mutex);

    Array stack = arrCreate(sizeof(Tet), 2);
    arrPush(stack, &t);
    arrPush(stack, &x);
    arrPush(stack, &y);

    if (o) {
        pthread_mutex_lock(&del->mutex);
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


 //       printf("g = %d\n", g);
 //       printf("o = "); tetPrint(o, stdout); printf("\n"); fflush(stdout);
 //       printf("p = "); vPrint(p, stdout); printf("\n"); fflush(stdout);
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

        pthread_mutex_unlock(&del->mutex);


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
            if (t->n[n]) conjecture(t->n[n]->n[tetReadMap(t->m, n)] == t, "");
            if (x->n[n]) conjecture(x->n[n]->n[tetReadMap(x->m, n)] == x, "");
            if (y->n[n]) conjecture(y->n[n]->n[tetReadMap(y->m, n)] == y, "");
        }

        // check vertices
        check(o->v[A] == p);
        check(xx->v[A] == p);
        check(yy->v[A] == p);

        check(tetIsLegit(o));
        check(tetIsLegit(xx));
        check(tetIsLegit(yy));

        // check neighbors
        n = A;
        for (n = B; n<=D; n++) {
            if (o->n[n]) check(o->n[n]->v[A] == p);
            if (xx->n[n]) check(xx->n[n]->v[A] == p);
            if (yy->n[n]) check(yy->n[n]->v[A] == p);
        }

    }


    TetVertex n = B;
    for (n = B; n<=D; n++) {
        if (t->n[n]) check(t->n[n]->v[A] == p);
        if (x->n[n]) check(x->n[n]->v[A] == p);
        if (y->n[n]) check(y->n[n]->v[A] == p);
    }

    check(tetIsLegit(t));
    check(tetIsLegit(x));
    check(tetIsLegit(y));

    stub;
    return stack;
}


static
Array ins1(Delaunay del, Tet t, Vertex* p, enum TetEdge e)
{
    call;
    const Tet flag = t;

    pthread_mutex_lock(&del->mutex);
    do {
        switch(e) {
        case BD:
        case DC:
        case CB:
            tetRot(t, ((e&1)<<1)|((e&4)>>2)); // lambda BD => C | DC => B | CB => D
            e = (((e&1)^1)<<1) | (e==5); // lambda BD => AB | DC => AD | CB => AC
        default:
            if (e == AB) break;
            if (e == AD) { tetRot(t, A); e = AC; }
            check (e == AC);

            tetRot(t, A);
            e = AB;

            break;
        }

        if (t->v[A] > t->v[B]) {
            tetRot(t, D);
            tetRot(t, B);
            tetRot(t, B);
        }

        Vertex* a = t->v[A];
        Vertex* b = t->v[B];

        t = t->n[D];
        TetVertex lblA = tetVertexLabel(t, a);
        TetVertex lblB = tetVertexLabel(t, b);

        switch (lblA+lblB)
        {
        case 1:
            e = AB;
            break;
        case 2:
            e = AC;
            break;
        case 3:
            e = AD + 2 * (lblA && lblB);
            break;
        case 4:
            e = BD;
            break;
        case 5:
              e = DC;
              break;
        default:
            check(0);
        }

    } while (flag != t);

    Vertex AP;  vNormalizeI(vSub(p, t->v[A], &AP));
    Vertex BP;  vNormalizeI(vSub(p, t->v[B], &BP));
    check(fabs(vDot(&AP, &BP)) == 1);

    Array stack = arrCreate(sizeof (Tet), 1);

    do {
        struct Tet tmp = {{p, t->v[A], t->v[D], t->v[C]}, {0, 0, 0, 0}, 0};
        Tet o = arrPush(del->t, &tmp);
        tetConnect(o, A, t->n[B], tetReadMap(t->m, B));
        tetConnect(o, B, t, B);
        t->v[A] = p;

        arrPush(stack, &o);
        arrPush(stack, &t);

        t = t->n[D];
    } while (flag != t);

    do {
        tetConnect(t->n[B], C, t->n[D]->n[B], D);
        t = t->n[D];
    } while (flag != t);
    pthread_mutex_unlock(&del->mutex);

    stub;
    return stack;
}

static
bool flip23(Delaunay del, Tet t, Array stack)
{
    //
    check(t->n[oA]);
    Tet o = t->n[oA];

    pthread_mutex_lock(&del->mutex);
    if (tetReadMap(t->m, oA) == oA) tetRot(o, D);

    while (tetVertexLabel(o, t->v[B]) != A) tetRot(t, A); // this should do at most 2 rotations
    while (tetVertexLabel(o, t->v[C]) != B) tetRot(o, A); // this should do at most 2 rotations

    Tet s = 0;

    struct Tet tmp = {
        {t->v[A], t->v[D], t->v[B], o->v[D]},
        {0, 0, 0, 0}, 0};
    s = arrPush(del->t, &tmp);

    // fix external neighbors
    tetConnect(s, A, o->n[B], tetReadMap(o->m, B));
    tetConnect(s, D, t->n[C], tetReadMap(t->m, C));
    tetConnect(t, A, o->n[C], tetReadMap(o->m, C));
    tetConnect(o, D, t->n[B], tetReadMap(t->m, B));

    // set vertices
    t->v[D] = s->v[D];
    o->v[A] = t->v[A];

    // fix internal neighbors
    tetConnect(t, B, o, C);
    tetConnect(t, C, s, B);
    tetConnect(s, C, o, B);

    pthread_mutex_unlock(&del->mutex);

    arrPush(stack, &t);
    arrPush(stack, &o);
    arrPush(stack, &s);

    check(tetIsLegit(t));
    check(tetIsLegit(o));
    check(tetIsLegit(s));

    stub;
    return true;
}


struct flip_pair { Tet what; Tet with; };

inline static
void update_stack(uint64_t i, Obj o, Obj d) {
    struct flip_pair* p = d;
    if (*oCast(Tet*, o) == p->what) {
        *oCast(Tet*, o) = p->with;
        check(tetIsLegit(*oCast(Tet*, o)));
    }
}

static
bool flip32(Delaunay del, Tet t, TetFace f, Array stack)
{
    Tet o = t->n[oA];
    Tet s = t->n[f];

    // preliminary checks
    check(t->n[f]->v[A] == t->v[A]);
    check(s);

    if (!o) {
        arrPush(stack, &t);
        return false;
    }

    if (s->n[oA] != o) {
        arrPush(stack, &t);
        return false;
    }

    // get labels
    const TetVertex F = f;
    const TetVertex G = (f + 1 + (f==D))&3;
    const TetVertex H = ((f | 4) - 1 - (f==B))&3;

    const TetVertex X = tetReadMap(t->m, f);
    const TetVertex Y = (X + 1 + (X==D))&3;
    const TetVertex Z = ((X | 4) - 1 - (X==B))&3;

    const TetVertex N = tetVertexLabel(o, t->v[G]);
    const TetVertex O = tetVertexLabel(o, t->v[H]);

    if (orient3d(*t->v[A], *t->v[F], *t->v[G], *s->v[X]) < 0) {
        arrPush(stack, &t);
        return false;
    }
    if (orient3d(*t->v[A], *t->v[H], *t->v[F], *s->v[X]) < 0) {
        arrPush(stack, &t);
        return false;
    }

    pthread_mutex_lock(&del->mutex);
    // Moving vertices
    t->v[H] = s->v[X];
    s->v[Z] = t->v[F];

    tetConnect(t, A, o->n[O], tetReadMap(o->m, O));
    tetConnect(s, A, o->n[N], tetReadMap(o->m, N));

    tetConnect(s, X, t->n[G], tetReadMap(t->m, G));
    tetConnect(t, F, s->n[Y], tetReadMap(s->m, Y));

    tetConnect(t, G, s, Y);

    check(tetIsLegit(s));
    check(tetIsLegit(t));


    Tet moved = arrBack(del->t);
    if (o!=moved) {
        tetCopy(o, moved);
        check(tetIsLegit(o));
        struct flip_pair swap_them = {moved, o};
        arrForEach(stack, update_stack, &swap_them);
    }

    arrPop(del->t);

    if (t==moved) t = o;
    if (s==moved) s = o;
    check(tetIsLegit(t));
    check(tetIsLegit(s));


    pthread_mutex_unlock(&del->mutex);

    arrPush(stack, &t);
    arrPush(stack, &s);

    stub;
    return true;
}

static
bool flip44(Delaunay del, Tet t, TetVertex tV, Array stack)
{
    Tet tb = t->n[tV];
    if (!tb) {
        fprintf(stderr, "[!] No tb.\n"); fflush(stderr);
        arrPush(stack, &t);
        return false;
    }

    Tet tc = tb->n[oA];
    if (!tc) {
        fprintf(stderr, "[!] No tc.\n"); fflush(stderr);
        arrPush(stack, &t);
        return false;
    }

    Tet ta = t->n[oA];

    if (tc != ta->n[tetVertexLabel(ta, t->v[tV])]) {
     //   fprintf(stderr, "[!] tc and ta are not neighbors.\n"); fflush(stderr);
        arrPush(stack, &t);
        return false;
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
    Vertex* const W = t->v[tW]; ignore W;
    Vertex* const X = ta->v[taX];
    Vertex* const Y = ta->v[taY];
    Vertex* const Z = tb->v[tbZ];

    if (orient3d(*U, *Y, *V, *X) < 0) {
      //  fprintf(stderr, "[!] Concave.\n"); fflush(stderr);
        arrPush(stack, &t);
        return false;
    } else check(orient3d(*U, *V, *Y, *Z) != 0);

    if (orient3d(*U, *Z, *Y, *Z) < 0) {
       // fprintf(stderr, "[!] Concave.\n"); fflush(stderr);
        arrPush(stack, &t);
        return false;
    } else check(orient3d(*U, *W, *V, *Z) != 0);

    Tet const taoW = ta->n[taW]; const TetFace taoWm = tetReadMap(ta->m, taW);
    Tet const tcoW = tc->n[tcW]; const TetFace tcoWm = tetReadMap(tc->m, tcW);


    pthread_mutex_lock(&del->mutex);

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
    tetConnect(tc, oD, tb->n[tbW], tetReadMap(tb->m, tbW));

    tetConnect(t, tW, ta, B);
    tetConnect(tb, tbW, tc, C);
    tetConnect(ta, C, tc, B);

    tetConnect(ta, A, taoW, taoWm);
    tetConnect(tc, A, tcoW, tcoWm);

    pthread_mutex_unlock(&del->mutex);

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

    check(tetIsLegit(t));
    check(tetIsLegit(ta));
    check(tetIsLegit(tb));
    check(tetIsLegit(tc));

    stub;
    return true;
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
Array flip(Delaunay del, Array stack)
{
    Array ret = arrCreate(sizeof(Tet), 1);

    while (!arrIsEmpty(stack)) {
        arrRandomSwap(stack, 0);
        Tet t = *oCast(Tet*, arrBack(stack));
        arrPush(ret, &t);
        arrPop(stack);

        Tet ta = t->n[oA];
        if (!ta) continue;

        conjecture((t->n[oB])?(t->n[oB]->v[A] == t->v[A]):(1), "there is a tet incident in p with vertex A not in p.");
        conjecture((t->n[oC])?(t->n[oC]->v[A] == t->v[A]):(1), "there is a tet incident in p with vertex A not in p.");
        conjecture((t->n[oD])?(t->n[oD]->v[A] == t->v[A]):(1), "there is a tet incident in p with vertex A not in p.");
        // t = <A, B, C, D> = <p, a, b, c>
        real* const p = *t->v[A];
        real* const a = *t->v[B];
        real* const b = *t->v[C];
        real* const c = *t->v[D];
        // assume no degenerate triangles

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

                Tet tbremoved = t->n[oD]; 
                if (flip32(del, t, oD, stack)) {
                    Obj pos = arrFind(ret, &tbremoved, 0);
                    if (pos) oCopyTo(pos, arrBack(ret), sizeof(Tet));
                    arrPop(ret);
                }

            } else if (o > 0) {

                o = orient3d(p, b, c, d);

                if (o < 0) {

                    // d should lie strictly below pca
                    conjecture(orient3d(p, c, a, d) > 0, "d does not lie below pca");
                    Tet tbremoved = t->n[oD]; 
                    if (flip32(del, t, oB, stack)) {
                        Obj pos = arrFind(ret, &tbremoved, 0);
                        if (pos) oCopyTo(pos, arrBack(ret), sizeof(Tet));
                        arrPop(ret);
                    }

                } else if (o > 0) {
                    o = orient3d(p, c, a, d);

                    if (o < 0) {

                        Tet tbremoved = t->n[oD]; 
                        if (flip32(del, t, oC, stack)) {
                            Obj pos = arrFind(ret, &tbremoved, 0);
                            if (pos) oCopyTo(pos, arrBack(ret), sizeof(Tet));
                            arrPop(ret);
                        }

                    } else if (o > 0) {
                        ignore flip23(del, t, stack);
                    } else {// o = 0  =>  d is on plane pca
                        ignore flip44(del, t, C, stack);
                    }

                } else {// o = 0  =>  d is on plane pbc
                    ignore flip44(del, t, B, stack);
                }
            } else {// o = 0  => d  is on plane pab
                ignore flip44(del, t, D, stack);
            }
        }
    }

    arrDestroy(stack);
    stub;

    return ret;
}

Array delInsert(Delaunay d, Vertex* p)
{
    Tet t = arrFront(d->t);
    Tet ot = t;

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
                    return 0;
                } else {
                    // p is on half-line (AC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with C
                        return 0;
                    } else {
                        // P is on segment (AC)
                        return flip(d, ins1(d, t, p, AC));
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
                        return 0;
                    } else {
                        // P is on segment (AB)
                        return flip(d, ins1(d, t, p, AB));
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (CB)
                        return flip(d, ins1(d, t, p, CB));
                    } else {
                        // P is inside the face (ABC)
                        return flip(d, ins2(d, t, p, ABC));
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
                        return 0;
                    } else {
                        // P is on segment (AD)
                        return flip(d, ins1(d, t, p, AD));
                    }
                } else {
                    // p is inside the angle CAD

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (DC)
                        return flip(d, ins1(d, t, p, DC));
                    } else {
                        // P is on face (ACD)
                        return flip(d, ins2(d, t, p, ACD));
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
                        return flip(d, ins1(d, t, p, BD));
                    } else {
                        // P is inside of face (ADB)
                        return flip(d, ins2(d, t, p, ADB));
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o < 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is inside face (BDC)
                        return flip(d, ins2(d, t, p, BDC));
                    } else {
                        // P is inside the tetrahedron
                        return flip(d, ins3(d, t, p));
                    }
                }
            }
        }
    }

    fprintf(stderr, "p is outside of the tetrahedrization.\n");
    return 0;

}

bool delCheck(Delaunay d)
{
    pthread_mutex_lock(&d->mutex);
    uint64_t ntet = arrSize(d->t);
    uint64_t i;
    bool correct = true;
    for (i=0; i<ntet; i++) {
        Tet t = oCast(Tet, arrGet(d->t, i));
        conjecture(tetIsLegit(t), "Incorect tetrahedron int delaunay.");
        TetNeighbour n = oA;
        for (n=A; n<=D; n++) {
            if (t->n[n]) {
                Vertex* v = t->n[n]->v[tetReadMap(t->m, n)];
                if (delIsBounding(d, t) && delIsOnBoundary(d, v)) continue;
                if (insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *v) > 0) {
                    fprintf(stderr, "[x] Inside.\n");
                    fprintf(stderr, "t = "); tetPrint(t, stderr); fprintf(stderr, "\n");
                    fprintf(stderr, "v = "); vPrint(v, stderr); fprintf(stderr, "\n");
                    fflush(stderr);
                    correct = false;
                }
            }
        }
    }
    pthread_mutex_unlock(&d->mutex);
    return correct;
}

void delDisplay(Delaunay d, int tet)
{
    tet = tet % arrSize(d->t);
    Tet t_sel = oCast(Tet, arrGet(d->t, tet));

    //static bool once = 0;
    unsigned int i = 0;
    unsigned int end = 0;
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);

    glColor4f(0.0, 1.0, 0.0, 1.0);

    pthread_mutex_lock(&d->mutex);

    end = arrSize(d->t);
    for (i=0; i<end; i++) {
        if (i == tet) continue;
        Tet t = oCast(Tet, arrGet(d->t, i));
        if (delIsBounding(d, t)) {

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

    glColor4f(0.0, 0.0, 1.0, 1.0);
    for (i=0; i<end; i++) {
        if (i == tet) continue;
        Tet t = oCast(Tet, arrGet(d->t, i));
        if (!delIsBounding(d, t)) {
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


    glPointSize(10.0);
    glBegin(GL_POINTS);


    glColor4f(1.0, 0.0, 0.0, 1.0);
    if (t_sel->n[oA] && insphere(*t_sel->v[0], *t_sel->v[1], *t_sel->v[2], *t_sel->v[3], *t_sel->n[oA]->v[tetReadMap(t_sel->m, A)]) > 0) {
        glVertex3v(*t_sel->n[oA]->v[tetReadMap(t_sel->m, A)]);
    }

    glEnd();

    if (d->render_circ) {
        glColor4f(0.0, 1.0, 1.0, 1.0);
        tetRenderCircumsphere(t_sel);
        glColor4f(1.0, 0.0, 0.0, 1.0);
    }
    glEnable(GL_LIGHTING);
    tetRenderSolid(t_sel);

    pthread_mutex_unlock(&d->mutex);

}

inline
Delaunay delCreate(Vertex (*hull)[4])
{
    Delaunay d = oCreate(sizeof (struct Delaunay));
    d->v = arrCreate(sizeof (Vertex), 2);
    d->t = arrCreate(sizeof (struct Tet), 2);

    struct Tet t = {
        {   arrPush(d->v, &(*hull)[0]),
            arrPush(d->v, &(*hull)[1]),
            arrPush(d->v, &(*hull)[2]),
            arrPush(d->v, &(*hull)[3]) },
        {0, 0, 0, 0}, 0 };

    d->A = t.v[A];
    d->B = t.v[B];
    d->C = t.v[C];
    d->D = t.v[D];
    d->render_circ = false;

    arrPush(d->t, &t);

    pthread_mutex_init(&d->mutex, 0);

    return d;
}

inline
void delDestroy(Delaunay restrict d)
{
    arrDestroy(d->v);
    arrDestroy(d->t);
    pthread_mutex_destroy(&d->mutex);
    oDestroy(d);
}


inline
bool delIsBounding(Delaunay restrict del, Tet restrict t) {
    return delIsOnBoundary(del, t->v[0])
        || delIsOnBoundary(del, t->v[1])
        || delIsOnBoundary(del, t->v[2])
        || delIsOnBoundary(del, t->v[3]);
}

inline
bool delIsOnBoundary(Delaunay restrict del, Vertex* v) {
    return (v == del->A) || (v == del->B) || (v == del->C) || (v == del->D);
}
