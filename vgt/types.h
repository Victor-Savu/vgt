#ifndef VGT_TYPES_H
#define VGT_TYPES_H

#include <math/types.h>

/* Types */
// index type for referencing elements in a list it is important that a list
// has at most as many elements as ind can represent
typedef short ind;   // 64k elements
//typedef int  ind;   //  4G elements
//typedef long  ind;   //  2^64 elements

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
typedef struct Edge* Edge;

// to be implemented
typedef struct Spectrum* Spectrum;
typedef struct RedBlackTree *RedBlackTree;
typedef struct RedBlackNode *RedBlackNode;

/* Methods */
typedef bool (*CompareMethod)(const Obj a, const Obj b);
typedef void (*DeleteMethod)(Obj a);

#endif//VGT_TYPES_H
