#ifndef VGT_TYPES_H
#define VGT_TYPES_H

#include <math/types.h>
#include <ads/types.h>

/* Types */
typedef Vertex Vec3;
typedef Vertex Normal;

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

// to be implemented
typedef struct Spectrum* Spectrum;


#endif//VGT_TYPES_H
