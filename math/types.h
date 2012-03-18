#ifndef MATH_TYPES_H
#define MATH_TYPES_H

#define false   (0)
#define true    (1)
#define eps     (1e-9)

#define unused(x) (void)(x)

#ifdef SAFE_MODE
#define safe(x) {x}
#else
#define safe(x)
#endif

typedef float           real;
typedef unsigned char   byte;
typedef unsigned char   bool;
typedef unsigned int    uint;
typedef real            Vec[3];
typedef Vec             Mat[3];
typedef void*           Obj;


typedef struct Spherical*   Spherical;
typedef struct Frame*       Frame;

#endif//MATH_TYPES_H

