#ifndef VGT_TYPES_H
#define VGT_TYPES_H

#include <math/types.h>

/* Types */
typedef void*           Obj;


/* Classes */

//to be tested
typedef struct ScalarField* ScalarField;
typedef struct VectorField* VectorField;
typedef struct VolumetricData* VolumetricData;

// under developement
typedef struct List* List;
typedef struct ListElement* ListElement;
typedef struct Mesh* Mesh;
typedef struct Renderer* Renderer;


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
