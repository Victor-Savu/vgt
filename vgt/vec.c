#include <vgt/vec.h>
#include <vgt/vec_cls.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local constants
//static const struct Vec const_zero = {0.0, 0.0, 0.0};
//static const struct Vec const_x = {1.0, 0.0, 0.0};
//static const struct Vec const_y = {0.0, 1.0, 0.0};
//static const struct Vec const_z = {0.0, 0.0, 1.0};

real vGetX(Vec a)
{
    return a->x;
}

real vGetY(Vec a)
{
    return a->y;
}

real vGetZ(Vec a)
{
    return a->z;
}

inline real vDot(const Vec a, const Vec b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

inline real vNorm(const Vec a)
{
    return sqrt(vDot(a, a));
}

inline Vec vCross(const Vec a, const Vec b, Vec c)
{
    c->x = a->y * b->z - a->z * b->y;
    c->y = a->z * b->x - a->x * b->z;
    c->z = a->x * b->y - a->y * b->x;
    return c;
}

inline Vec vAdd(const Vec a, const Vec b, Vec c)
{
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    c->z = a->z + b->z;
    return c;
}


inline Vec vScale(const Vec a, real s, Vec b)
{
    b->x = s * a->x;
    b->y = s * a->y;
    b->z = s * a->z;
    return b;
}

inline Vec vMinus(const Vec a, Vec b)
{
    return vScale(a, -1.0, b);
}

inline Vec vSub(const Vec a, const Vec b, Vec c)
{
    return vAdd(a, vMinus(b,b), c);
}

inline Vec vNormalize(const Vec a, Vec b)
{
    real n = vNorm(a);
    if (fabs(n) < eps) {
        fprintf(stderr, "Normalizing zero vector.\n");
        exit(EXIT_FAILURE);
    }
    return vScale(a, 1.0/n, b);
}

inline Vec vCopy(const Vec from, Vec to)
{
    memcpy(to, from, sizeof (struct Vec));
    return to;
}


inline Vec vCreate(real x, real y, real z)
{
    Vec a = malloc(sizeof (struct Vec));
    if (!a) {
        fprintf(stderr, "Normalizing zero vector.\n");
        exit(EXIT_FAILURE);
    }
    return vSet(x, y, z, a);
}

inline Vec vSet(real x, real y, real z, Vec a)
{
    a->x = x; a->y = y; a->z = z;
    return a;
}

inline void vDestroy(Vec a)
{
    if (!a) {
        fprintf(stderr, "[!] Tried to destroy an empty vector.\n");
    } else {
        free(a);
    }
}

/*
inline const Vec const vZero(void)
{
    return &const_zero;
}

inline const Vec vX(void)
{
    return &const_x;
}

inline const Vec vY(void)
{
    return &const_y;
}

inline const Vec vZ(void)
{
    return (const Vec const)&const_z;
}
*/
inline void vPrint(const Vec a, FILE* fp)
{
    char s[128];
    fprintf(fp, "%s", vStrPrint(a, s));
}

inline char* vStrPrint(const Vec a, char* s)
{
    float x = a->x, y = a->y, z = a->z;
    sprintf(s, "<%.3f, %.3f, %.3f>", x, y, z);
    return s;
}

