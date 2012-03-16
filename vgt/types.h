#ifndef VGT_TYPES_H
#define VGT_TYPES_H

#define false   (0)
#define true    (1)
#define eps     (1e-9)

#define unused(x) (void)(x)

#ifdef SAFE_MODE
#define safe(x) {x}
#else
#define safe(x)
#endif

/* Types */

typedef float         real;
//typedef double          real;
typedef unsigned char   byte;
typedef unsigned char   bool;
typedef void*           Obj;
typedef unsigned int    uint;



/* Classes */

//to be tested
typedef struct ScalarField* ScalarField;
typedef struct VectorField* VectorField;
typedef struct VolumetricData* VolumetricData;
typedef struct Vec* Vec;

// under developement
typedef struct List* List;
typedef struct ListElement* ListElement;
typedef struct Mesh* Mesh;


// to be implemented
typedef struct Spectrum* Spectrum;
typedef struct RedBlackTree *RedBlackTree;
typedef struct RedBlackNode *RedBlackNode;
typedef struct Edge* Edge;
typedef struct Vert* Vert;
typedef struct Face* Face;


/* Methods */
typedef bool (*CompareMethod)(const Obj a, const Obj b);
typedef void (*DeleteMethod)(Obj a);

#endif//VGT_TYPES_H
