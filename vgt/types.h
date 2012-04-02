#ifndef VGT_TYPES_H
#define VGT_TYPES_H

#include <math/types.h>
#include <ads/types.h>


/* Classes */

//to be tested
typedef struct ScalarField* ScalarField;
typedef struct VectorField* VectorField;
typedef struct VolumetricData* VolumetricData;
typedef struct Mesh* Mesh;
typedef struct Edge* Edge;
typedef struct Tet* Tet;

// under developement
typedef struct Delaunay* Delaunay;
typedef struct Victor* Victor;
typedef struct HalfEdge* HalfEdge;

// to be implemented
typedef struct Spectrum* Spectrum;


#endif//VGT_TYPES_H
