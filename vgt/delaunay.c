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

    Tet b = arrPush(del->t, &tet_b);
    tetConnect(b, oA, t->n[oB], tetReadMap(t->m, oB));
    tetConnect(b, oB, t, oB);

    Tet c = arrPush(del->t, &tet_c);
    tetConnect(c, oA, t->n[oC], tetReadMap(t->m, oC));
    tetConnect(c, oC, t, oC);
    tetConnect(c, oD, b, oD);

    Tet d = arrPush(del->t, &tet_d);
    tetConnect(d, oA, t->n[oD], tetReadMap(t->m, oD));
    tetConnect(d, oD, t, oD);
    tetConnect(d, oC, b, oC);
    tetConnect(d, oB, c, oB);

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
    Array stack = arrCreate(sizeof(Tet), 2);

    Tet o = t->n[f];
    TetFace g = tetReadMap(t->m, f);

    if (f == oA) {
        Tet t_b, t_c;
        {
            struct Tet tmp = {{p, t->v[A], t->v[D], t->v[C]}, {0, 0, 0, 0}, 0};
            t_b = arrPush(del->t, &tmp);
            tetConnect(t_b, oA, t->n[oB], tetReadMap(t->m, oB));
            tetConnect(t_b, oC, t, oB);
        }
        {
            struct Tet tmp = {{p, t->v[D], t->v[A], t->v[B]}, {0, 0, 0, 0}, 0};
            t_c = arrPush(del->t, &tmp);
            tetConnect(t_c, oA, t->n[oC], tetReadMap(t->m, oC));
            tetConnect(t_c, oB, t, oC);
        }
        tetConnect(t_b, oD, t_c, oD);

        t->v[D] = t->v[A];
        t->v[A] = p;
        tetConnect(t, oA, t->n[oD], tetReadMap(t->m, oD));
        // still need to connect t_b in oB, t_c in oC and t in oA and oD
    } else {
        Tet x, y;
        {
            struct Tet tmp = {{p, t->v[], t->v[], t->v[A]}, {0, 0, 0, 0}, 0};
            t_b = arrPush(del->t, &tmp);
            tetConnect(t_b, oA, t->n[oB], tetReadMap(t->m, oB));
            tetConnect(t_b, oC, t, oB);
        }
    }

    arrPush(stack, o);


    t->v[(f+2)&3] = o->v[g];
    tetConnect(t, f, o->n[(g+1)&3], tetReadMap(o->m, (g+1)&3));
    tetConnect(t, (f+1)&3, o, (g+1)&3);



    struct Tet embrio = { {p, 0, 0, 0},{ 0, 0, 0, 0}, 0};
    Tet Charlie = arrPush(d->t, &embrio);
    arrPush(stack, Charlie);


    void ins2ABC() {
        o = t->n[oD];
        arrPush(stack, o);

        if (t == o->n[oA]) {

        } else if (t == o->n[oB]) {

        } else if (t == o->n[oC]) {

        } else if (t == o->n[oD]) {

        } else check(0);

    };

    void ins2ACD() {
        o = t->n[oB];
        arrPush(stack, o);

    };

    void ins2ADB() {
        o = t->n[oC];
        arrPush(stack, o);

    };

    void ins2BDC() {
        o = t->n[oA];
        arrPush(stack, o);

    }

    switch (f) {
    case ABC:
        ins2ABC();
        break;
    case ACD:
        ins2ACD();
        break;
    case ADB:
        ins2ADB();
        break;
    case BDC:
        ins2BDC();
        break;
    default:
        check(0);
        break;
    }

    stub;

    return stack;
}

Array ins1(Delaunay d, Tet t, Vec* p, enum TetEdge e)
{
    Array stack = 0;

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

    glColor4f(0.0, 1.0, 0.0, 1.0);
    end = arrSize(d->t);
    for (i=0; i<end; i++) {
        Tet t = arrGet(d->t, i);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3fv(*(t->v[A]));   glVertex3fv(*(t->v[B]));
        glVertex3fv(*(t->v[C]));   glVertex3fv(*(t->v[D]));
        glVertex3fv(*(t->v[A]));   glVertex3fv(*(t->v[B]));
        glEnd();
    }



    glPointSize(5.0);
    glBegin(GL_POINTS);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    end = arrSize(d->v);
    for (i=0; i<end; i++) {
        Vec* v = oCast(Vec*, arrGet(d->v, i));
        glVertex3fv(*v);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}
