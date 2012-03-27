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

void create_cell(Delaunay restrict d, Vec* (*V)[4])
{
    Vec* restrict A = V[0][0];
    Vec* restrict B = V[0][1];
    Vec* restrict C = V[0][2];
    Vec* restrict D = V[0][3];
    // A "sees" BCD from the positive side

    // BDC                               pos : vert, next, oped, opfac
    HalfEdge BD = arrPush(d->e, heConst(D, 0, 0, 0)); // # 0 :    D,   DC,   DB,  none
    HalfEdge DC = arrPush(d->e, heConst(C, 0, 0, 0)); // # 1 :    C,   CB,   CD,  none
    HalfEdge CB = arrPush(d->e, heConst(B, 0, 0, 0)); // # 2 :    B,   BD,   BC,  none

    // ADB                               pos : vert, next, oped, opfac
    HalfEdge AD = arrPush(d->e, heConst(D, 0, 0, 0)); // # 3 :    D,   DB,   DA,  none
    HalfEdge DB = arrPush(d->e, heConst(B, 0, 0, 0)); // # 4 :    B,   BA,   BD,  none
    HalfEdge BA = arrPush(d->e, heConst(A, 0, 0, 0)); // # 5 :    A,   AD,   AB,  none

    // ACD                               pos : vert, next, oped, opfac
    HalfEdge AC = arrPush(d->e, heConst(C, 0, 0, 0)); // # 6 :    C,   CD,   CA,  none
    HalfEdge CD = arrPush(d->e, heConst(D, 0, 0, 0)); // # 7 :    D,   DA,   DC,  none
    HalfEdge DA = arrPush(d->e, heConst(A, 0, 0, 0)); // # 8 :    A,   AC,   AD,  none

    // ABC                               pos : vert, next, oped, opfac
    HalfEdge AB = arrPush(d->e, heConst(B, 0, 0, 0)); // # 9 :    B,   BC,   BA,  none
    HalfEdge BC = arrPush(d->e, heConst(C, 0, 0, 0)); // #10 :    C,   CA,   CB,  none
    HalfEdge CA = arrPush(d->e, heConst(A, 0, 0, 0)); // #11 :    A,   AB,   AC,  none

    BD->n = DC; BD->o = DB;
    DC->n = CB; DC->o = CD;
    CB->n = BD; CB->o = BC;

    AD->n = DB; AD->o = DA;
    DB->n = BA; DB->o = BD;
    BA->n = AD; BA->o = AB;

    AC->n = CD; AC->o = CA;
    CD->n = DA; CD->o = DC;
    DA->n = AC; DA->o = AD;

    AB->n = BC; AB->o = BA;
    BC->n = CA; BC->o = CB;
    CA->n = AB; CA->o = AC;
}
/*
void delete_edge(Delaunay restrict d, HalfEdge e)
{
    Edge last = arrBack(d->e);
    unused(last);
}
*/

bool in_cell(HalfEdge restrict e, Vec* restrict p, real (*orient)[4])
{
    HalfEdge const restrict A = e;
    HalfEdge const restrict B = A->o;
    HalfEdge const restrict C = A->n->o;
    HalfEdge const restrict D = A->n->n->o;



    (*orient)[0] = orient3d(*p, *A->v, *A->n->v, *A->n->n->v);
    (*orient)[1] = orient3d(*p, *B->v, *B->n->v, *B->n->n->v);
    (*orient)[2] = orient3d(*p, *C->v, *C->n->v, *C->n->n->v);
    (*orient)[3] = orient3d(*p, *D->v, *D->n->v, *D->n->n->v);

    return ((*orient)[0] < 0)&&((*orient)[1] < 0)&&((*orient)[2] < 0)&&((*orient)[3] < 0);
}

HalfEdge walk(HalfEdge restrict e, Vec* restrict p)
{
    real where[4];

    while (!in_cell(e, p, &where)) {
        if (where[0]<0) e = e->f; else
        if (where[1]<0) e = e->o->f; else
        if (where[2]<0) e = e->n->o->f; else
        if (where[3]<0) e = e->n->n->o->f;
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
    create_cell(d, &V);

    return d;
}

void delDestroy(Delaunay restrict d)
{
    arrDestroy(d->v);
    arrDestroy(d->e);
    oDestroy(d);
}



void delInsert(Delaunay restrict d, Vec* restrict p)
{
    Array stack = arrCreate(sizeof (HalfEdge), 2);

    // 1:  τ ← Walk {to obtain tetra containing p}
    HalfEdge t = walk(arrFront(d->e), p);

    {
        // 2: insert p in τ with a flip14
        HalfEdge out[4];
        flip14(d, p, t, &out);
        // 3: push 4 new tetrahedra on stack
        arrPush(stack, out[0]);
        arrPush(stack, out[1]);
        arrPush(stack, out[2]);
        arrPush(stack, out[3]);
    }

    bool case1=0, case2=0, case3=0, case4=0;

    // 4: while stack is non-empty do
    while (!arrIsEmpty(stack)) {
    // 5:  τ = {p, a, b, c} ← pop from stack
        t = *oCast(HalfEdge*, arrBack(stack));
    // 6:  τa = {a, b, c, d} ← get adjacent tetrahedron of τ having abc as a facet
        HalfEdge ta = (t->n->n->o->f);
        if (!ta) continue;
    // 7:  if d is inside circumsphere of τ then
        if (insphere(*(ta->n->v), *(t->v), *(ta->v), *(ta->n->v), *(ta->n->n->v))) {
            // 8:     Flip(τ , τa )
            // 1: if case #1 then
            if (case1) {
                // 2:    flip23(τ , τa )
                HalfEdge in[2] = {t, ta};
                HalfEdge out[3];
                flip23(d, &in, &out);
                // 3:    push tetrahedra pabd, pbcd and pacd on stack
                arrPush(stack, out[0]);
                arrPush(stack, out[1]);
                arrPush(stack, out[2]);
                // 4: else if case #2 AND T p has tetrahedron pdab then
            } else if ( (case2) && () ) {
                HalfEdge in[3] = {t, ta, pdab};
                HalfEdge out[2];
                // 5:    flip32(τ , τa , pdab)
                flip32(d, &in, &out);
                // 6:    push pacd and pbcd on stack
                arrPush(stack, out[0]);
                arrPush(stack, out[1]);
                // 7: else if case #3 AND τ and τa are in config44 with τb and τc then
            } else if ( case3 && ()) {
                HalfEdge in[4] = {t, ta, tb, tc};
                HalfEdge out[4];
                // 8:    flip44(τ , τa , τb , τc )
                flip44(d, &in, &out);
                // 9:    push on stack the 4 tetrahedra created
                arrPush(stack, out[0]);
                arrPush(stack, out[1]);
                arrPush(stack, out[2]);
                arrPush(stack, out[3]);
                //10: else if case #4 then
            } else if (case4) {
                HalfEdge in[2] = {t, ta};
                HalfEdge out[4];
                //11:    flip23(τ , τa )
                flip23(d, &in, &out);
                //12:    push tetrahedra pabd, pbcd and pacd on stack
                arrPush(stack, out[0]);
                arrPush(stack, out[1]);
                arrPush(stack, out[2]);
                //13: end if
            }
            // 9:  end if
        }
        //10: end while
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
