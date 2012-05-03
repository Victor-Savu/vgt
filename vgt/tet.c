#include <vgt/tet.h>
#include <vgt/tet_cls.h>

#include <stdio.h>
#include <math/vertex.h>
#include <math/predicates.h>


#include <GL/glut.h>


inline
void tetRot(Tet restrict t, TetVertex v)
{
    // swap variables
    const TetVertex X = (v+1)&3; // lambda A=>B | B=>C | C=>D | D=>A
    const TetVertex Y = (((v|4)-1)&2); // lambda A=>C | B=>A | C=>A | D=>C
    const TetVertex Z = ((v^2)|1); // lambda A=>D | B=>D | C=>B | D=>B
    byte m = t->m;
    Obj swap = 0;
    // swap vertices
    swap = t->v[X]; t->v[X] = t->v[Y]; t->v[Y] = t->v[Z]; t->v[Z] = swap;
    // swap neighbors
    swap = t->n[X]; t->n[X] = t->n[Y]; t->n[Y] = t->n[Z]; t->n[Z] = swap;
    // notify neighbors
    tetConnect(t, X, t->n[X], tetReadMap(m, Y));
    tetConnect(t, Y, t->n[Y], tetReadMap(m, Z));
    tetConnect(t, Z, t->n[Z], tetReadMap(m, X));

    check(tetIsLegit(t));
}


void tetPrint(Obj tet, FILE* f)
{
    Tet const t = tet;
    fprintf(f, "[");
    vPrint(t->v[0], f); fprintf(f, ", ");
    vPrint(t->v[1], f); fprintf(f, ", ");
    vPrint(t->v[2], f); fprintf(f, ", ");
    vPrint(t->v[3], f); fprintf(f, "]");
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
    // Reference: http://cgafaq.info/wiki/Tetrahedron_Circumsphere

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

    vScale((const Vertex*)&cBC, vNormSquared((const Vertex*)&rD), &r);
    vAddI(&r, (const Vec3*)vScale((const Vertex*)&cDB, vNormSquared((const Vertex*)&rC), &c)); // using c as an auliliary variable for now
    vAddI(&r, (const Vec3*)vScale((const Vertex*)&cCD, vNormSquared((const Vertex*)&rB), &c)); // using c as an auliliary variable for now

    vScaleI(&r, 0.5/vDot(&rB, &cCD));
    // now setting c
    vAdd(t->v[A], &r, &c);

    glPushMatrix();
    glTranslated(c[0], c[1], c[2]);
    glutWireSphere(vNorm((const Vertex*)&r), 100, 100);
    glPopMatrix();
}

bool tetIsLegit(const Tet t)
{
   // if (!t) return true;
    TetNeighbour n = A, q= A;
    for (n = A; n<=D; n++) {
        if (t->n[n]) {
            if (t->n[n] == t) {
                fprintf(stderr, "Tet is its own neighbor.\n"); fflush(stderr);
                return false;
            }
            if (t->n[n]->n[tetReadMap(t->m, n)] != t) {
                fprintf(stderr, "My map is wrong.\n"); fflush(stderr);
                printf("t [%ld]= ", (size_t)t); tetPrint(t, stdout); printf("\n");
                printf("n [%ld]= ", (size_t)t->n[n]); tetPrint(t->n[n], stdout); printf("\n");
                printf("t*[%ld]= ", (size_t)t->n[n]->n[tetReadMap(t->m, n)]); tetPrint(t->n[n]->n[tetReadMap(t->m, n)], stdout); printf("\n");
                fflush(stdout);
                return false;
            }
            for (q=A; q<=D; q++)
                if (q != n) {
                    if (tetVertexLabel(t->n[n], t->v[q]) == INVALID_FACET) {
                        fprintf(stderr, "Not sharing vertices.\n"); fflush(stderr);
                        return false;
                    }
                    if (tetVertexLabel(t->n[n], t->v[q]) == tetReadMap(t->m, n)) {
                        fprintf(stderr, "This vertex should not be here [%d:%d:%d].\n",q, n, tetVertexLabel(t->n[n], t->v[q])); fflush(stderr);
                        return false;
                    }
                }
        }
    }
    if (!(orient3d(*t->v[B], *t->v[C], *t->v[D], *t->v[A]) < 0)) {
        fprintf(stderr, "Orientation is wrong : A is not above BCD.\n"); fflush(stderr);
        return false;
    }
    if (!(orient3d(*t->v[C], *t->v[A], *t->v[D], *t->v[B]) < 0)) {
        fprintf(stderr, "Orientation is wrong : B is not above CAD.\n"); fflush(stderr);
        return false;
    }
    if (!(orient3d(*t->v[D], *t->v[A], *t->v[B], *t->v[C]) < 0)) {
        fprintf(stderr, "Orientation is wrong : C is not above DAB.\n"); fflush(stderr);
        return false;
    }
    if (!(orient3d(*t->v[A], *t->v[C], *t->v[B], *t->v[D]) < 0)) {
        fprintf(stderr, "Orientation is wrong : D is not above ACB.\n"); fflush(stderr);
        return false;
    }


    return true;
}

bool tetIsAlmostDelaunay(const Tet t)
{
    if (t->n[oB] && insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *t->n[oB]->v[tetReadMap(t->m, oB)]) > 0) {
        fprintf(stderr, "Contains neighbor oB.\n"); fflush(stderr);
        return false;
    }
    if (t->n[oC] && insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *t->n[oC]->v[tetReadMap(t->m, oC)]) > 0) {
        fprintf(stderr, "Contains neighbor oC.\n"); fflush(stderr);
        return false;
    }
    if (t->n[oD] && insphere(*t->v[A], *t->v[B], *t->v[C], *t->v[D], *t->n[oD]->v[tetReadMap(t->m, oD)]) > 0) {
        fprintf(stderr, "Contains neighbor oD.\n"); fflush(stderr);
        return false;
    }
    return true;
}
