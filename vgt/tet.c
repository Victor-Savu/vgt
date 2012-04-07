#include <vgt/tet.h>
#include <vgt/tet_cls.h>

#include <stdio.h>
#include <math/vertex.h>

#include <GL/glut.h>

inline
void tetConnect(Tet x, TetFace fx, Tet y, TetFace fy)
{
    if (x) {
        x->n[fx] = y;
        // erase the old note in the map for neighbor in fx
        x->m &= 0xff ^ (3 << (fx << 1));
        // and set the new value
        x->m |= fy << (fx << 1);
    }
    if (y) {
        y->n[fy] = x;
        // erase the old note in the map for neighbor in fx
        y->m &= 0xff ^ (3 << (fy << 1));
        // and set the new value
        y->m |= fx << (fy << 1);
    }
}

inline
TetFace tetReadMap(byte m, TetNeighbour n)
{
    return (m >> (n<<1)) & 3;
}

inline
TetVertex tetVertexLabel(Tet t, Vertex * v)
{
    if (v == t->v[0]) return A;
    if (v == t->v[1]) return B;
    if (v == t->v[2]) return C;
    if (v == t->v[3]) return D;
    conjecture(0, "Requested label of a vertex which is not part of t.");
    return INVALID_FACET;
}

inline
void tetRot(Tet restrict t, TetVertex v)
{
    // swap variables
    const TetVertex m = (v+1)&3;
    const TetVertex n = (m+1)&3;
    const TetVertex o = (n+1)&3;
    Tet const nm = t->n[m];
    const TetVertex mm = tetReadMap(t->m, m);
    
    // roate t->v
    Vertex* const a = t->v[m]; t->v[m] = t->v[n]; t->v[n] = a;

    // rotate t->n & t->m
    tetConnect(t, m, t->n[n], tetReadMap(t->m, n)); // overwrites t->n[m] and t->m[m]
    tetConnect(t, n, t->n[o], tetReadMap(t->m, o)); // overwrites t->n[n] and t->m[n]
    tetConnect(t, n,      nm,                  mm); // overwrites t->n[o] and t->m[o]
}


void tetPrint(Obj tet, FILE* f)
{
    Tet const t = tet;
    fprintf(f, "[");
    vPrint(t->v[0], f); fprintf(f, ", ");
    vPrint(t->v[1], f); fprintf(f, ", ");
    vPrint(t->v[2], f); fprintf(f, ", ");
    vPrint(t->v[3], f); fprintf(f, "]\n");
}

void tetRenderSolid(Tet t)
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
    glVertex3v(*(t->v[C]));   glVertex3v(*(t->v[D]));
    glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
    glEnd();
}

void tetRenderWireframe(Tet t)
{
    glBegin(GL_LINE_STRIP);
    glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[B]));
    glVertex3v(*(t->v[D]));   glVertex3v(*(t->v[C]));
    glVertex3v(*(t->v[A]));   glVertex3v(*(t->v[D]));
    glEnd();

    glBegin(GL_LINES);
    glVertex3v(*(t->v[B]));   glVertex3v(*(t->v[C]));
    glEnd();
}

void tetRenderCircumsphere(Tet t)
{
    // relative positions of vertices B, C, and D with respect to A
    Vertex rB, rC, rD;

    vSub(t->v[B], t->v[A], &rB);
    vSub(t->v[C], t->v[A], &rC);
    vSub(t->v[D], t->v[A], &rD);

    Vertex cCD;
    ignore vCross(&rC, &rD, &cCD);
    Vertex cDB;
    ignore vCross(&rD, &rB, &cDB);
    Vertex cBC;
    ignore vCross(&rB, &rC, &cBC);


    // the inverse radius pointing from A to the sphere's centre
    Vertex r;
    // the sphere's centre
    Vertex c;

    vScale(&cBC, vNormSquared(&rD), &r);
    vAddI(&r, vScale(&cDB, vNormSquared(&rC), &c)); // using c as an auliliary variable for now
    vAddI(&r, vScale(&cCD, vNormSquared(&rB), &c)); // using c as an auliliary variable for now

    vScaleI(&r, 0.5/vDot(&rB, &cCD));
    // now setting c
    vAdd(t->v[A], &r, &c);

    glPushMatrix();
    glTranslated(c[0], c[1], c[2]);
    glutWireSphere(vNorm(&r), 100, 100);
    glPopMatrix();
}
