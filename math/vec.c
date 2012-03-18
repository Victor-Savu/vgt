#include <math/vec.h>
#include <math/vec_cls.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local constants
//static const struct Vec const_zero = {0.0, 0.0, 0.0};
//static const struct Vec const_x = {1.0, 0.0, 0.0};
//static const struct Vec const_y = {0.0, 1.0, 0.0};
//static const struct Vec const_z = {0.0, 0.0, 1.0};

real vDot(Vec* restrict a, const Vec* restrict b)
{
    return (*a)[0] * (*b)[0] + (*a)[1] * (*b)[1] + (*a)[2] * (*b)[2]; 
}

real vNormSquared(Vec* restrict a)
{
    return (*a)[0] * (*a)[0] + (*a)[1] * (*a)[1] + (*a)[2] * (*a)[2];
}

real vNorm(Vec* restrict a)
{
    return sqrt(vNormSquared(a));
}

Vec* vCross(Vec* restrict a, const Vec* restrict b, Vec* restrict c)
{
    (*c)[0] = (*a)[1] * (*b)[2] - (*a)[2] * (*b)[1];
    (*c)[1] = (*a)[2] * (*b)[0] - (*a)[0] * (*b)[2];
    (*c)[2] = (*a)[0] * (*b)[1] - (*a)[1] * (*b)[0];
    return c;
}

Vec* vAdd(Vec* restrict a, const Vec* restrict b, Vec* restrict c)
{
    (*c)[0] = (*a)[0] + (*b)[0];
    (*c)[1] = (*a)[1] + (*b)[1];
    (*c)[2] = (*a)[2] + (*b)[2];
    return c;
}

Vec* vScale(Vec* restrict a, real s, Vec* restrict b)
{
    (*b)[0] = s * (*a)[0];
    (*b)[1] = s * (*a)[1];
    (*b)[2] = s * (*a)[2];
    return b;
}

Vec* vSub(Vec* restrict a, const Vec* restrict b, Vec* restrict c)
{
    (*c)[0] = (*a)[0] - (*b)[0];
    (*c)[1] = (*a)[1] - (*b)[1];
    (*c)[2] = (*a)[2] - (*b)[2];
    return c;
}

Vec* vNormalize(Vec* restrict a, Vec* restrict b)
{
    real n = vNorm(a);
    if (n < eps) {
        fprintf(stderr, "Normalizing zero vector.\n");
        exit(EXIT_FAILURE);
    }
    return vScale(a, 1.0/eps, b);
}

Vec* vCopy(Vec* restrict from, Vec* restrict to)
{
    (*to)[0] = (*from)[0];
    (*to)[1] = (*from)[1];
    (*to)[2] = (*from)[2];
    return to;
}

Vec* vCrossI(Vec* restrict a, const Vec* restrict b)
{
    real a0 = (*a)[0];
    real a1 = (*a)[1];
    (*a)[0] = a1 * (*b)[2] - (*a)[2] * (*b)[1];
    (*a)[1] = (*a)[2] * (*b)[0] - a0 * (*b)[2];
    (*a)[2] = a0 * (*b)[1] - a1 * (*b)[0];
    return a;
}

Vec* vAddI(Vec* restrict a, const Vec* restrict b)
{
    (*a)[0] += (*b)[0];
    (*a)[1] += (*b)[1];
    (*a)[2] += (*b)[2];
    return a;
}

Vec* vScaleI(Vec* restrict a, real s)
{
    (*a)[0] *= s;
    (*a)[1] *= s;
    (*a)[2] *= s;
    return a;
}

Vec* vSubI(Vec* restrict a, const Vec* restrict b)
{
    (*a)[0] -= (*b)[0];
    (*a)[1] -= (*b)[1];
    (*a)[2] -= (*b)[2];
    return a;
}

Vec* vNormalizeI(Vec* restrict a)
{
    real n = vNorm(a);
    if (n < eps) {
        fprintf(stderr, "Normalizing zero vector.\n"); fflush(stderr);
        exit(EXIT_FAILURE);
    }
    return vScaleI(a, 1.0/eps);
}

void vPrint(Vec* restrict a, FILE* restrict fp)
{
    char s[128];
    fprintf(fp, "%s", vStrPrint(a, s));
}

char* vStrPrint(Vec* restrict a, char* restrict s)
{
    float x = (*a)[0], y = (*a)[1], z = (*a)[2];
    sprintf(s, "<%.3f, %.3f, %.3f>", x, y, z);
    return s;
}

