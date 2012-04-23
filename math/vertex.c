#include <math/vertex.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline
real vGetX(Vertex* restrict a)
{
    return (*a)[0];
}

inline
real vGetY(Vertex* restrict a)
{
    return (*a)[1];
}

inline
real vGetZ(Vertex* restrict a)
{
    return (*a)[2];
}

inline
real vDot(Vertex* restrict a, Vertex* restrict b)
{
    return (*a)[0] * (*b)[0] + (*a)[1] * (*b)[1] + (*a)[2] * (*b)[2]; 
}

inline
real vNormSquared(Vertex* restrict a)
{
    return (*a)[0] * (*a)[0] + (*a)[1] * (*a)[1] + (*a)[2] * (*a)[2];
}

inline
real vNorm(Vertex* restrict a)
{
    return sqrt(vNormSquared(a));
}

inline
Vertex* vCross(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c)
{
    (*c)[0] = (*a)[1] * (*b)[2] - (*a)[2] * (*b)[1];
    (*c)[1] = (*a)[2] * (*b)[0] - (*a)[0] * (*b)[2];
    (*c)[2] = (*a)[0] * (*b)[1] - (*a)[1] * (*b)[0];
    return c;
}

inline
Vertex* vAdd(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c)
{
    (*c)[0] = (*a)[0] + (*b)[0];
    (*c)[1] = (*a)[1] + (*b)[1];
    (*c)[2] = (*a)[2] + (*b)[2];
    return c;
}

inline
Vertex* vScale(Vertex* restrict a, real s, Vertex* restrict b)
{
    (*b)[0] = s * (*a)[0];
    (*b)[1] = s * (*a)[1];
    (*b)[2] = s * (*a)[2];
    return b;
}

inline
Vertex* vSub(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c)
{
    (*c)[0] = (*a)[0] - (*b)[0];
    (*c)[1] = (*a)[1] - (*b)[1];
    (*c)[2] = (*a)[2] - (*b)[2];
    return c;
}

inline
Vertex* vNormalize(Vertex* restrict a, Vertex* restrict b)
{
    real n = vNorm(a);
    if (n < 1e-10) {
        conjecture (n > eps, "Normalizing zero vector.\n");
    }
    return vScale(a, 1.0/n, b);
}

inline
Vertex* vCopy(Vertex* restrict from, Vertex* restrict to)
{
    (*to)[0] = (*from)[0];
    (*to)[1] = (*from)[1];
    (*to)[2] = (*from)[2];
    return to;
}

inline
Vertex* vCrossI(Vertex* restrict a, Vertex* restrict b)
{
    real a0 = (*a)[0];
    real a1 = (*a)[1];
    (*a)[0] = a1 * (*b)[2] - (*a)[2] * (*b)[1];
    (*a)[1] = (*a)[2] * (*b)[0] - a0 * (*b)[2];
    (*a)[2] = a0 * (*b)[1] - a1 * (*b)[0];
    return a;
}

inline
Vertex* vAddI(Vertex* restrict a, Vertex* restrict b)
{
    (*a)[0] += (*b)[0];
    (*a)[1] += (*b)[1];
    (*a)[2] += (*b)[2];
    return a;
}

inline
Vertex* vScaleI(Vertex* restrict a, real s)
{
    (*a)[0] *= s;
    (*a)[1] *= s;
    (*a)[2] *= s;
    return a;
}

inline
Vertex* vSubI(Vertex* restrict a, Vertex* restrict b)
{
    (*a)[0] -= (*b)[0];
    (*a)[1] -= (*b)[1];
    (*a)[2] -= (*b)[2];
    return a;
}

inline
Vertex* vNormalizeI(Vertex* restrict a)
{
    real n = vNorm(a);
    if (n < 1e-10) {
        fprintf(stderr, "Normalizing zero vector.\n"); fflush(stderr);
        exit(EXIT_FAILURE);
    }
    return vScaleI(a, 1.0/n);
}

inline
Vertex* vSet(Vertex* restrict a, real x, real y, real z)
{
    (*a)[0] = x;
    (*a)[1] = y;
    (*a)[2] = z;
    return a;
}

inline
bool vIsZero(Vertex* restrict a)
{
    return ((*a)[0] == (*a)[1]) && ((*a)[1] == (*a)[2]) && ((*a)[2] == 0.0);
}

inline
void vPrint(Vertex* restrict a, FILE* restrict fp)
{
    char s[128];
    fprintf(fp, "%s", vPrintStr(a, s));
}

inline
char* vPrintStr(Vertex* restrict a, char* restrict s)
{
    if (!a) strcpy(s, "<null>");
    float x = (*a)[0], y = (*a)[1], z = (*a)[2];
    sprintf(s, "<%.3f, %.3f, %.3f>", x, y, z);
    return s;
}

