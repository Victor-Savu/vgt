#ifndef ADS_TYPES_H
#define ADS_TYPES_H

#include <math/types.h>

/* Types */
// index type for referencing elements in a list it is important that a list
// has at most as many elements as ind can represent
typedef uint16_t    ind;   // 64k elements
//typedef uint32_t  ind;   //  4G elements
//typedef uint64_t   ind;   //  2^64 elements


/* Classes */

//to be tested
typedef struct Array* Array;
typedef Array Stack; // TODO: Separate the concepts. The array is more than just a queue
typedef struct Queue* Queue;
typedef struct Vector* Vector;

// under developement
typedef struct List* List;
typedef struct ListElement* ListElement;

// to be implemented
typedef struct RedBlackTree* RedBlackTree;
typedef struct RedBlackNode* RedBlackNode;

/* Methods */
typedef bool (*CompareMethod)(const Obj a, const Obj b);
typedef void (*DeleteMethod)(Obj a);
typedef void (*ArrOperation)(uint64_t, Obj, Obj);

#endif//ADS_TYPES_H
