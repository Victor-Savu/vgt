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

#ifdef SAFE_MODE
#define safe(x) {x}
#else
#define safe(x)
#endif

typedef float           real;
typedef uint8_t         byte;
typedef uint8_t         bool;
typedef real            Vec[3];
typedef Vec             Mat[3];
typedef void*           Obj;


typedef struct Spherical*   Spherical;
typedef struct Frame*       Frame;

#endif//MATH_TYPES_H

