#ifndef MATH_VEC_H
#define MATH_VEC_H

#include <math/types.h>

#include <stdio.h>

Vertex* vCreate(real x, real y, real z);
real vGetX(Vertex* restrict a);
real vGetY(Vertex* restrict a);
real vGetZ(Vertex* restrict a);

void vDestroy(Vertex* restrict a);
real vDot(Vertex* restrict a, Vertex* restrict b);
real vNormSquared(Vertex* restrict a);
real vNorm(Vertex* restrict a);
Vertex* vCopy(Vertex* restrict from, Vertex* restrict to);
Vertex* vCross(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c);
Vertex* vAdd(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c);
Vertex* vScale(Vertex* restrict a, real s, Vertex* restrict b);
Vertex* vSub(Vertex* restrict a, Vertex* restrict b, Vertex* restrict c);
Vertex* vNormalize(Vertex* restrict a, Vertex* restrict b);

Vertex* vCrossI(Vertex* restrict a, Vertex* restrict b);
Vertex* vAddI(Vertex* restrict a, Vertex* restrict b);
Vertex* vScaleI(Vertex* restrict a, real s);
Vertex* vSubI(Vertex* restrict a, Vertex* restrict b);
Vertex* vNormalizeI(Vertex* restrict a);

Vertex* vSet(Vertex* restrict a, real x, real y, real z);

void vPrint(Vertex* restrict a, FILE* restrict fp);
char* vPrintStr(Vertex* restrict a, char* s);

#endif//VGT_VEC_H

