#ifndef VGT_HALF_EDGE_CLS_H
#define VGT_HALF_EDGE_CLS_H

#include <vgt/types.h>

struct HalfEdge {
    Vec* v;  // the vertex it points to
    HalfEdge n;  // the next edge in the (half-)face
    HalfEdge o;  // the opposite edge on the 2D manifold
    HalfEdge f;  // the edge of the opposite face in the 3D manifold
};

#endif//VGT_HALF_EDGE_CLS_H
