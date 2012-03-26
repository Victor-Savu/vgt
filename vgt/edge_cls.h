#ifndef VGT_EDGE_CLS_H
#define VGT_EDGE_CLS_H

#include <vgt/types.h>

struct Edge {
    ind v;  // the vertex it points to
    ind n;  // the next edge in the (half-)face
    ind o;  // the opposite edge on the 2D manifold
    ind f;  // the edge of the opposite face in the 3D manifold
};

#endif//VGT_EDGE_CLS_H
