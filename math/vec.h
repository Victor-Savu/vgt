#ifndef MATH_VEC_H
#define MATH_VEC_H

#include <math/types.h>

#include <stdio.h>

Vec vCreate(real x, real y, real z);
Vec vSet(real x, real y, real z, Vec a);
real vGetX(Vec a);
real vGetY(Vec a);
real vGetZ(Vec a);
//const Vec const vZero(void);
//const Vec vX(void);
//const Vec vY(void);
//const Vec vZ(void);
void vDestroy(Vec a);
real vDot(const Vec a, const Vec b);
real vNorm(const Vec a);
Vec vCross(const Vec a, const Vec b, Vec c);
Vec vAdd(const Vec a, const Vec b, Vec c);
Vec vScale(const Vec a, real s, Vec b);
Vec vMinus(const Vec a, Vec b);
Vec vSub(const Vec a, const Vec b, Vec c);
Vec vNormalize(const Vec a, Vec b);
Vec vCopy(const Vec from, Vec to);


void vPrint(const Vec a, FILE* fp);
char* vStrPrint(const Vec a, char* s);

#endif//VGT_VEC_H

