#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/predicates.h>
#include <vgt/tet_cls.h>
#include <math/obj.h>
#include <ads/array.h>
#include <math/vec.h>
#include <GL/glut.h>
/*
void neighborhood(HalfEdge e, HalfEdge (*n)[4])
{
    (*n)[0] = e->f;
    (*n)[1] = e->o->f;
    (*n)[2] = e->n->o->f;
    (*n)[3] = e->n->n->o->f;
}

bool outside_cell(HalfEdge restrict e, Vec* restrict p, real (*orient)[4], uint8_t ignored_faces)
{
    HalfEdge N[4];

    // get the neighboring faces
    neighborhood(e, &N);
    unsigned int i=0;
    bool ret = false;

    for (i=0; i<4; i++) {
        (*orient)[i] = (ignored_faces & (1<<i))?(-1):(orient3d(*N[i]->v, *N[i]->n->v, *N[i]->n->n->v, *p));
        ret |= (*orient)[i] > 0;
    }

    return ret;
}

HalfEdge walk(Delaunay restrict d, Vec* restrict p)
{
    real where[4];
    HalfEdge restrict e = d->t;
    uint8_t ignore_faces = NONE;

    // this can be manually optimized not to check the neighbor we just came from
    while (e && outside_cell(e, p, &where, ignore_faces)) {
        if (!ignore_faces && where[0]<0) e = e->f; else
        if (where[1]<0) e = e->o->f; else
        if (where[2]<0) e = e->n->o->f; else
        if (where[3]<0) e = e->n->n->o->f;
        ignore_faces = FIRST;
    }

    return e;
}



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

enum TetEdge {AB, AC, AD, BC, CD, DB};

void ins1(Delaunay d, Tet t, Vec* p, enum TetEdge e)
{
    stub;
}

void ins2()
{
    stub;
}

void ins3()
{
    stub;
}

Delaunay delCreate(Vec (*hull)[4])
{

    Delaunay d = oCreate(sizeof (struct Delaunay));
    d->v = arrCreate(sizeof (Vec), 2);
    d->t = arrCreate(sizeof (struct Tet), 2);

    struct Tet t = { arrPush(d->v, &(*hull)[0]), arrPush(d->v, &(*hull)[1]), arrPush(d->v, &(*hull)[2]), arrPush(d->v, &(*hull)[3]), 0, 0, 0, 0 };
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
void delInsert(Delaunay restrict d, Vec* restrict p)
{
    Tet t = arrFront(d->t);

    // find the tetrahedron containing p

    real o;
    while (t) {
        // check ABC
        o = orient3d(*t->a, *t->b, *t->c, *p);
        if (o < 0) { t = t->od; continue; }
        if (o == 0) {
            // p is on plane ABC

            // check ACD
            o = orient3d(*t->a, *t->c, *t->d, *p);
            if (o < 0) { t = t->ob; continue; }
            if (o == 0) {
                // p is on line AC

                // check ADB
                o = orient3d(*t->a, *t->d, *t->b, *p);
                if (o < 0) { t = t->oc; continue; }
                if (o == 0) {
                    // p coincides with A
                    return;
                } else {
                    // p is on half-line (AC

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p coincides with C
                        return;
                    } else {
                        // P is on segment (AC)
                        ins1(d, t, p, AC);
                        return;
                    }
                }
            } else {
                // P is on half-plane (CAB

                // check ADB
                o = orient3d(*t->a, *t->d, *t->b, *p);
                if (o < 0) { t = t->oc; continue; }
                if (o == 0) {
                    // p is on half-line (AB

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p coincides with B
                        return;
                    } else {
                        // P is on segment (AB)
                        ins1(d, t, p, AB);
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p is on segment (BC)
                        ins1(d, t, p, BC);
                        return;
                    } else {
                        // P is inside the face (ABC)
                        ins2(d, t, p);
                        return;
                    }
                }

            }
        } else {

            // check ACD
            o = orient3d(*t->a, *t->c, *t->d, *p);
            if (o < 0) { t = t->ob; continue; }
            if (o == 0) {
                // p is on half plane (ACD

                // check ADB
                o = orient3d(*t->a, *t->d, *t->b, *p);
                if (o < 0) { t = t->oc; continue; }
                if (o == 0) {
                    // p is on half-line (AD

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p coincides with D
                        return;
                    } else {
                        // P is on segment (AD)
                        ins1(d, t, p, AD);
                        return;
                    }
                } else {
                    // p is inside the angle CAD

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p is on segment (CD)
                        ins1(d, t, p, CD);
                        return;
                    } else {
                        // P is on face (ACD)
                        ins2(d, t, p);
                        return;
                    }
                }
            } else {

                // check ADB
                o = orient3d(*t->a, *t->d, *t->b, *p);
                if (o < 0) { t = t->oc; continue; }
                if (o == 0) {
                    // p is inside the angle <)BAD

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p is on segment (DB)
                        ins1(d, t, p, DB);
                        return;
                    } else {
                        // P is inside of face (ADB)
                        ins2(d, t, p);
                        return;
                    }
                } else {
                    // p is inside the angle BAC

                    // check BDC
                    o = orient3d(*t->b, *t->d, *t->c, *p);
                    if (o < 0) { t = t->oa; continue; }
                    if (o == 0) {
                        // p is inside face (BCD)
                        ins2(d, t, p);
                        return;
                    } else {
                        // P is inside the tetrahedron
                        ins3(d, t, p);
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
        glVertex3fv(*(t->a));   glVertex3fv(*(t->b));
        glVertex3fv(*(t->c));   glVertex3fv(*(t->d));
        glVertex3fv(*(t->a));   glVertex3fv(*(t->b));
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
