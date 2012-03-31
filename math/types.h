#ifndef MATH_TYPES_H
#define MATH_TYPES_H

#include <stdint.h>

#define false   (0)
#define true    (1)
#define eps     (1e-9)

#ifndef M_PI
#define M_PI    (3.14159265358979323846)
#endif//M_PI
#define M_2PI   (6.28318530717958647692)

#define VEC_X   { 1.0, 0.0, 0.0 }
#define VEC_Y   { 0.0, 1.0, 0.0 }
#define VEC_Z   { 0.0, 0.0, 1.0 }
#define VEC_0   { 0.0, 0.0, 0.0 }

#define MAT_I   { VEC_X, VEC_Y, VEC_Z }
#define MAT_0   { VEC_0, VEC_0, VEC_0 }

#define unused(x) (void)(x)
#define ignore    (void)

#define restrict __restrict__

#define SAFE_MODE   1

#ifdef SAFE_MODE
#include <stdio.h>
#include <stdlib.h>
#define safe(x)     do {x} while (0)
#define check(x)    do { if (!x) { fprintf(stderr, "[x] %s:%s:%d: Check failed.", __FILE__, __func__, __LINE__); } } while (0)
#define stub        do { printf("[!] %s:%s:%d: Not yet implemented.", __FILE__, __func__, __LINE__); } while (0)
#else
#define safe(x)
#define check(x)
#endif


#define NONE    (0x0)
#define FIRST   (0x1)
#define SECOND  (0x2)
#define THIRD   (0x4)
#define FOURTH  (0x8)
#define FIFTH   (0x10)
#define SIXTH   (0x11)
#define SEVENTH (0x12)

typedef float           real;
typedef uint8_t         byte;
typedef uint8_t         bool;
typedef uint8_t         mask8;
typedef real            Vec[3];
typedef Vec             Mat[3];
typedef void*           Obj;


typedef struct Spherical*   Spherical;
typedef struct Frame*       Frame;

typedef struct Mesh* Mesh;

#endif//MATH_TYPES_H

