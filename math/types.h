#ifndef MATH_TYPES_H
#define MATH_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

//#define restrict __restrict__
#define restrict

#define SAFE_MODE           1
#define SAFE_MODE_VERBOSE   1

#define conjecture(x, fail_message)  do { if (!(x)) { fprintf(stderr, "[x] %s:%s:%d: %s\n", __FILE__, __func__, __LINE__, (fail_message)); fflush(stderr); exit(EXIT_FAILURE); } } while (0)


#ifdef SAFE_MODE
# define safe(x)     do {x} while (0)
# define check(x)    conjecture((x), "Check failed.")
# define stub        do { fprintf(stderr, "[!] %s:%s:%d: Not yet implemented.\n", __FILE__, __func__, __LINE__); fflush(stderr); } while (0)
# ifdef SAFE_MODE_VERBOSE
#  define call       do { fprintf(stderr, "[i] %s:%s:%d: Calling function [%s].\n", __FILE__, __func__, __LINE__, __func__); fflush(stderr); } while (0)
# else
#  define call
# endif
#else
# define safe(x)
# define check(x)   unused(x)
# define call
# define stub
#endif


#define NONE    (0x0)
#define FIRST   (0x1)
#define SECOND  (0x2)
#define THIRD   (0x4)
#define FOURTH  (0x8)
#define FIFTH   (0x10)
#define SIXTH   (0x11)
#define SEVENTH (0x12)

/*
typedef float           real;
#define glNormal3v       glNormal3fv
#define glVertex3v       glVertex3fv
*/

typedef double           real;
#define glNormal3v       glNormal3dv
#define glVertex3v       glVertex3dv

typedef uint8_t         byte;
typedef uint8_t         bool;
typedef uint8_t         mask8;
typedef real            Vertex[3];
typedef Vertex             Mat[3];
typedef void*           Obj;

typedef void (*ObjPrint)(Obj, FILE*);

typedef struct Spherical*   Spherical;
typedef struct Frame*       Frame;

typedef struct Mesh* Mesh;

#endif//MATH_TYPES_H

