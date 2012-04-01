#ifndef MATH_VEC_H
#define MATH_VEC_H

#include <math/types.h>

#include <stdio.h>

Vec* vCreate(real x, real y, real z);
real vGetX(Vec* restrict a);
real vGetY(Vec* restrict a);
real vGetZ(Vec* restrict a);

void vDestroy(Vec* restrict a);
real vDot(Vec* restrict a, Vec* restrict b);
real vNormSquared(Vec* restrict a);
real vNorm(Vec* restrict a);
Vec* vCopy(Vec* restrict from, Vec* restrict to);
Vec* vCross(Vec* restrict a, Vec* restrict b, Vec* restrict c);
Vec* vAdd(Vec* restrict a, Vec* restrict b, Vec* restrict c);
Vec* vScale(Vec* restrict a, real s, Vec* restrict b);
Vec* vSub(Vec* restrict a, Vec* restrict b, Vec* restrict c);
Vec* vNormalize(Vec* restrict a, Vec* restrict b);

Vec* vCrossI(Vec* restrict a, Vec* restrict b);
Vec* vAddI(Vec* restrict a, Vec* restrict b);
Vec* vScaleI(Vec* restrict a, real s);
Vec* vSubI(Vec* restrict a, Vec* restrict b);
Vec* vNormalizeI(Vec* restrict a);

Vec* vSet(Vec* restrict a, real x, real y, real z);

void vPrint(Vec* restrict a, FILE* restrict fp);
char* vPrintStr(Vec* restrict a, char* s);

#endif//VGT_VEC_H

