#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>

#include <stdio.h>

#include <math/predicates.h>
#include <vgt/half_edge.h>
#include <vgt/half_edge_cls.h>
#include <math/obj.h>
#include <ads/array.h>
#include <math/vec.h>
#include <GL/glut.h>

/*
   Creates a tetrahedron.
   @arg d is the delaunay tetrahedrization
   @arg V is the list of 4 vertices <A, B, C, D>. the first vertex should be on the positive size of the plane of the other three.
   @arg N is the list of the 4 neighboring tetrahedra of the one which is created

   @return the tetrahedron ABCD
*/
HalfEdge create_cell(Delaunay restrict d, Vec* (*V)[4], HalfEdge (*N)[4])
{
    Vec* restrict A = V[0][0];
    Vec* restrict B = V[0][1];
    Vec* restrict C = V[0][2];
    Vec* restrict D = V[0][3];
    // A "sees" BCD from the positive side

    // ABC                               pos : vert, next, oped, opfac
    HalfEdge AB = arrPush(d->e, heConst(B, 0, 0, (*N)[0])); // # 9 :    B,   BC,   BA,  none
    HalfEdge BC = arrPush(d->e, heConst(C, 0, 0, (*N)[0])); // #10 :    C,   CA,   CB,  none
    HalfEdge CA = arrPush(d->e, heConst(A, 0, 0, (*N)[0])); // #11 :    A,   AB,   AC,  none

    // ACD                               pos : vert, next, oped, opfac
    HalfEdge AC = arrPush(d->e, heConst(C, 0, 0, (*N)[1])); // # 6 :    C,   CD,   CA,  none
    HalfEdge CD = arrPush(d->e, heConst(D, 0, 0, (*N)[1])); // # 7 :    D,   DA,   DC,  none
    HalfEdge DA = arrPush(d->e, heConst(A, 0, 0, (*N)[1])); // # 8 :    A,   AC,   AD,  none

    // ADB                               pos : vert, next, oped, opfac
    HalfEdge AD = arrPush(d->e, heConst(D, 0, 0, (*N)[2])); // # 3 :    D,   DB,   DA,  none
    HalfEdge DB = arrPush(d->e, heConst(B, 0, 0, (*N)[2])); // # 4 :    B,   BA,   BD,  none
    HalfEdge BA = arrPush(d->e, heConst(A, 0, 0, (*N)[2])); // # 5 :    A,   AD,   AB,  none

    // BDC                               pos : vert, next, oped, opfac
    HalfEdge CB = arrPush(d->e, heConst(B, 0, 0, (*N)[3])); // # 2 :    B,   BD,   BC,  none
    HalfEdge BD = arrPush(d->e, heConst(D, 0, 0, (*N)[3])); // # 0 :    D,   DC,   DB,  none
    HalfEdge DC = arrPush(d->e, heConst(C, 0, 0, (*N)[3])); // # 1 :    C,   CB,   CD,  none

    AB->n = BC; AB->o = BA;
    BC->n = CA; BC->o = CB;
    CA->n = AB; CA->o = AC;

    AC->n = CD; AC->o = CA;
    CD->n = DA; CD->o = DC;
    DA->n = AC; DA->o = AD;

    AD->n = DB; AD->o = DA;
    DB->n = BA; DB->o = BD;
    BA->n = AD; BA->o = AB;

    CB->n = BD; CB->o = BC;
    BD->n = DC; BD->o = DB;
    DC->n = CB; DC->o = CD;

    return CA;
}

void neighborhood(HalfEdge e, HalfEdge (*n)[4])
{
    (*n)[0] = e->f;
    (*n)[1] = e->o->f;
    (*n)[2] = e->n->o->f;
    (*n)[3] = e->n->n->o->f;
}
/*
void delete_edge(Delaunay restrict d, HalfEdge e)
{
    Edge last = arrBack(d->e);
    unused(last);
}
*/
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

Delaunay delCreate(Vec (*hull)[4])
{
    Delaunay d = oCreate(sizeof (struct Delaunay));
    d->v = arrCreate(sizeof (Vec), 2);
    d->e = arrCreate(sizeof (struct HalfEdge), 4);

    // vertices
    Vec* V[4];

    real side = orient3d((*hull)[0], (*hull)[1], (*hull)[2], (*hull)[3]);
    if ( side == 0) {
        fprintf(stderr, "Failed to initialize the delaunay tetrahedrization from coplanar points.\n");
        return 0;
    } else {
        if (side > 0) {
            V[0] = arrPush(d->v, &((*hull)[0]));
            V[1] = arrPush(d->v, &((*hull)[1]));
            V[2] = arrPush(d->v, &((*hull)[2]));
            V[3] = arrPush(d->v, &((*hull)[3]));
        } else {
            V[0] = arrPush(d->v, &((*hull)[0]));
            V[1] = arrPush(d->v, &((*hull)[1]));
            V[2] = arrPush(d->v, &((*hull)[3]));
            V[3] = arrPush(d->v, &((*hull)[2]));
        }
    }
/*
    printf("%s vectors:\n", __func__);
    printf("0: "); vPrint(V[0], stdout); printf("\n");
    printf("1: "); vPrint(V[1], stdout); printf("\n");
    printf("2: "); vPrint(V[2], stdout); printf("\n");
    printf("3: "); vPrint(V[3], stdout); printf("\n");
*/
    HalfEdge N[4] = {0, 0, 0, 0};
    create_cell(d, &V, &N);

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
