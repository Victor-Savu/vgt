#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/predicates.h>
#include <vgt/tet_cls.h>
#include <math/obj.h>
#include <ads/array.h>
#include <math/vec.h>
#include <GL/glut.h>

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

void flip12()
{
    stub;
}

void flip21()
{
    stub;
}

void ins1()
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
    d->t = arrCreate(sizeof (struct HalfEdge), 2);

    struct Tet t = { arrPush(d->v, &(*hull)[0]), arrPush(d->v, &(*hull)[1]), arrPush(d->v, &(*hull)[2]), arrPush(d->v, &(*hull)[3]), 0, 0, 0, 0 };
    arrPush(d->t, &t);

    return d;
}

void delDestroy(Delaunay restrict d)
{
    arrDestroy(d->v);
    arrDestroy(d->e);
    oDestroy(d);
}

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

void delInsert(Delaunay restrict d, Vec* restrict p)
{
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

}

void delDisplay(Delaunay restrict d)
{
    //static bool once = 0;
    unsigned int i = 0;
    unsigned int end = 0;
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);
    glBegin(GL_LINES);
    glColor4f(0.0, 0.0, 1.0, 1.0);
    end = arrSize(d->e);
    for (i=0; i<end; i++) {
        HalfEdge e = arrGet(d->e, i);
        if (e->v < e->n->v) {
      /*      if (!once) {
                printf("Segment: ");
                vPrint(e->v, stdout);
                printf(" ");
                vPrint(e->n->v, stdout);
                printf("\n");
            } */
            glVertex3fv(*(e->v));
            glVertex3fv(*(e->n->v));
        }
    }
    glEnd();

    glPointSize(3.0);
    glBegin(GL_POINTS);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    end = arrSize(d->v);
    for (i=0; i<end; i++) {
        Vec* v = oCast(Vec*, arrGet(d->v, i));
      //  glPushMatrix();
      //  glTranslatef((*v)[0], (*v)[1], (*v)[2]);
      //  glutSolidSphere (0.1, 5, 5);
      //  glPopMatrix();
        glVertex3fv(*v);
    }
    glEnd();

    glEnable(GL_LIGHTING);
  //  once = 1;
}
