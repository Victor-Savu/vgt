#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/obj.h>
#include <math/vec.h>
#include <math/predicates.h>

#include <ads/array.h>

#include <vgt/tet_cls.h>
#include <vgt/tet.h>


#include <GL/glut.h>

/*
void flip14(Delaunay d, Vec* p, HalfEdge cell, HalfEdge (*out)[4])
{

}

void flip23(Delaunay d, HalfEdge (*in)[2], HalfEdge (*out)[3])
{
}

void flip32(Delaunay d, HalfEdge (*in)[3], HalfEdge (*out)[2])
{
}

void flip44(Delaunay d, HalfEdge (*in)[4], HalfEdge (*out)[4])
{
}
*/
void flip12()
{
    stub;
}

void flip21()
{
    stub;
}


Array ins3(Delaunay del, Tet t, Vec* p)
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
    arrPush(stack, t);
    arrPush(stack, b);
    arrPush(stack, c);
    arrPush(stack, d);


    stub;
    return stack;
}


Array ins2(Delaunay del, Tet t, Vec* p, enum TetFacet f)
{
    p = arrPush(del->v, p);

    Tet o = t->n[f];
    TetFace g = tetReadMap(t->m, f);

    if (f == oA) { // if it's degenerate, swap it to normal
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
    arrPush(stack, t);
    arrPush(stack, x);
    arrPush(stack, y);

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

        if (t->v[f+3-(f==oB)] == o->v[g+1+(g==oD)]) {
            tetConnect(x, f, yy, g);
            tetConnect(t, f,  o, g);
            tetConnect(y, f, xx, g);
        } else  if (t->v[f+3-(f==oB)] == o->v[g+3-(g==oB)]) {
            tetConnect(x, f,  o, g);
            tetConnect(t, f, xx, g);
            tetConnect(y, f, yy, g);
        } else {
            tetConnect(x, f, xx, g);
            tetConnect(t, f, yy, g);
            tetConnect(y, f,  o, g);
        }

        arrPush(stack, o);
        arrPush(stack, xx);
        arrPush(stack, yy);
    }

    arrPrint(del->t, stdout, tetPrint);

    stub;
    return stack;
}

Array ins1(Delaunay del, Tet t, Vec* p, enum TetEdge e)
{
    p = arrPush(del->v, p);
    Array stack = arrCreate(sizeof(Tet), 2);

    stub;

    return stack;
}



Delaunay delCreate(Vec (*hull)[4])
{

    Delaunay d = oCreate(sizeof (struct Delaunay));
    d->v = arrCreate(sizeof (Vec), 2);
    d->t = arrCreate(sizeof (struct Tet), 2);

    struct Tet t = {
        {   arrPush(d->v, &(*hull)[0]),
            arrPush(d->v, &(*hull)[1]),
            arrPush(d->v, &(*hull)[2]),
            arrPush(d->v, &(*hull)[3]) },
        {0, 0, 0, 0}, 0 };
    arrPush(d->t, &t);

    return d;
}

void delDestroy(Delaunay restrict d)
{
    arrDestroy(d->v);
    arrDestroy(d->t);
    oDestroy(d);
}

Delaunay delCopy(Delaunay restrict d)
{
    check(d);
    Delaunay c = oCreate(sizeof (struct Delaunay));
    c->v = arrCopy(d->v);
    c->t = arrCopy(d->t);
    return c;
}

/*
enum FlipCase { CASE_1, CASE_2, CASE_3, CASE_4, CASE_SKIP, CASE_UNHANDLED };

enum FlipCase flip_case(HalfEdge* t) {

    if (!t->n->n->o->f) return CASE_SKIP;

    Vec* restrict p = t->v;
    Vec* restrict a = t->n->v;
    Vec* restrict b = t->n->n->v;
    Vec* restrict c = t->o->n->v;
    Vec* restrict d = t->n->n->o->f->n->n->v;

    // if it is not in the sphere
    if (insphere(*p, *a, *b, *c, *d) <= 0) return CASE_SKIP;

    // orientations
    const real abc = orient3d(*a, *b, *c, *p);
    const real acd = orient3d(*a, *c, *d, *p);
    const real adb = orient3d(*a, *b, *b, *p);
    const real cbd = orient3d(*c, *d, *d, *p);

    const uint8_t lt = (acd < 0) + (adb < 0) + (cbd < 0);
    const uint8_t eq = (acd ==0) + (adb ==0) + (cbd ==0);

    if (abc > 0) {
        if (lt == 3) return CASE_1;
        else if (lt == 2) {
            if (eq == 0) {
                if (acd > 0) *td = t->n->o->f;
                else if (adb > 0) *td = t->f;
                else if (cbd > 0) *td = t->o->f;
                else *td = 0;
                if (*td) return CASE_2; else return CASE_SKIP;
            } else if (eq == 1) {
                if (acd == 0) {*tb = t->n->o->f; *tc = (*tb)?():(0);};
                else if (adb == 0) {*tb = t->f; *tc = (*tb)?():(0);};
                else if (cbd == 0) {*tb = t->o->f; *tc = (*tb)?((*tb)->n->n->o->f):(0);};
                else *tb = 0, *tc = 0;
                if (*tb && *tc) return CASE_3; else  return CASE_SKIP;
            }
        }
    } else if (abc == 0) {
        if (lt == 3) return CASE_1; // degenerate case where p is on the face abc
        if (eq == 1) return CASE_4;
    }

    return CASE_UNHANDLED;
}
*/

void flip(Array stack)
{
    arrDestroy(stack);
    stub;
}

void delInsert(Delaunay restrict d, Vec* restrict p)
{
    Tet t = arrFront(d->t);
    Tet ot = t;

    // find the tetrahedron containing p

    real o;
    while (t) {
        // check ABC
        o = (t->n[oD] == ot)?(-1.0):orient3d(*t->v[A], *t->v[B], *t->v[C], *p);
        if (o > 0) { ot = t; t = t->n[oD]; continue; }
        if (o == 0) {
            // p is on plane ABC

            // check ACD
            o = (t->n[oB] == ot)?(-1.0):orient3d(*t->v[A], *t->v[C], *t->v[D], *p);
            if (o > 0) { ot = t; t = t->n[oB]; continue; }
            if (o == 0) {
                // p is on line AC

                // check ADB
                o = (t->n[oC] == ot)?(-1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o > 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p coincides with A
                    return;
                } else {
                    // p is on half-line (AC

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with C
                        return;
                    } else {
                        // P is on segment (AC)
                        flip(ins1(d, t, p, AC));
                        return;
                    }
                }
            } else {
                // P is on half-plane (CAB

                // check ADB
                o = (t->n[oC] == ot)?(-1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o > 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is on half-line (AB

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with B
                        return;
                    } else {
                        // P is on segment (AB)
                        flip(ins1(d, t, p, AB));
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (CB)
                        flip(ins1(d, t, p, CB));
                        return;
                    } else {
                        // P is inside the face (ABC)
                        flip(ins2(d, t, p, ABC));
                        return;
                    }
                }

            }
        } else {

            // check ACD
            o = (t->n[oB] == ot)?(-1.0):orient3d(*t->v[A], *t->v[C], *t->v[D], *p);
            if (o > 0) { ot = t; t = t->n[oB]; continue; }
            if (o == 0) {
                // p is on half plane (ACD

                // check ADB
                o = (t->n[oC] == ot)?(-1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o > 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is on half-line (AD

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p coincides with D
                        return;
                    } else {
                        // P is on segment (AD)
                        flip(ins1(d, t, p, AD));
                        return;
                    }
                } else {
                    // p is inside the angle CAD

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (DC)
                        flip(ins1(d, t, p, DC));
                        return;
                    } else {
                        // P is on face (ACD)
                        flip(ins2(d, t, p, ACD));
                        return;
                    }
                }
            } else {

                // check ADB
                o = (t->n[oC] == ot)?(-1.0):orient3d(*t->v[A], *t->v[D], *t->v[B], *p);
                if (o > 0) { ot = t; t = t->n[oC]; continue; }
                if (o == 0) {
                    // p is inside the angle <)BAD

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is on segment (BD)
                        flip(ins1(d, t, p, BD));
                        return;
                    } else {
                        // P is inside of face (ADB)
                        flip(ins2(d, t, p, ADB));
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = (t->n[oA] == ot)?(-1.0):orient3d(*t->v[B], *t->v[D], *t->v[C], *p);
                    if (o > 0) { ot = t; t = t->n[oA]; continue; }
                    if (o == 0) {
                        // p is inside face (BDC)
                        flip(ins2(d, t, p, BDC));
                        return;
                    } else {
                        // P is inside the tetrahedron
                        flip(ins3(d, t, p));
                        return;
                    }
                }
            }
        }
    }

    fprintf(stderr, "p is outside of the tetrahedrization.\n");

/*
    Array stack = arrCreate(sizeof (HalfEdge), 2);

    HalfEdge t = walk(arrFront(d->e), p);

    if (!t) return;

    flip14(d, &t, p);
    if (t) {
        arrPush(stack, t); t = t->n->o;
        arrPush(stack, t); t = t->n->o;
        arrPush(stack, t); t = t->n->o;
        arrPush(stack, t);
    }

    while (!arrIsEmpty(stack)) {
        t = *oCast(HalfEdge*, arrBack(stack));
        arrPop(stack);
        switch (flip_case(&t)) {
            case CASE_4:
            case CASE_1:
                flip23(d, &t);
                if (t) {
                    arrPush(stack, t); t = t->f->o;
                    arrPush(stack, t); t = t->f->o;
                    arrPush(stack, t);
                }
                break;
            case CASE_2:
                flip32(d, &t);
                if (t) {
                    arrPush(stack, t);
                    arrPush(stack, t->n->f);
                }
                break;
            case CASE_3:
                flip44(d, &t);
                if (t) {
                    arrPush(stack, t); t = t->f->o;
                    arrPush(stack, t); t = t->f->o;
                    arrPush(stack, t); t = t->f->o;
                    arrPush(stack, t);
                }
                break;
            default:
                break;

        }
    }
*/
}

void delDisplay(Delaunay restrict d)
{
    //static bool once = 0;
    unsigned int i = 0;
    unsigned int end = 0;
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glColor4f(0.0, 0.0, 1.0, 1.0);
    end = arrSize(d->t);
    for (i=0; i<end; i++) {
        Tet t = arrGet(d->t, i);
        glBegin(GL_LINE_STRIP);
        glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
        glVertex3v(*(t->v[D]));   glVertex3v(*(t->v[C]));
        glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[D]));
        glEnd();

        glBegin(GL_LINES);
        glVertex3v(*(t->v[B]));   glVertex3v(*(t->v[C]));
        glEnd();
    }

    glPointSize(5.0);
    glBegin(GL_POINTS);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    end = arrSize(d->v);
    for (i=0; i<end; i++) {
        Vec* v = oCast(Vec*, arrGet(d->v, i));
        glVertex3v(*v);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}
