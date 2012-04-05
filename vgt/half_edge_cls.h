#ifndef VGT_HALF_EDGE_CLS_H
#define VGT_HALF_EDGE_CLS_H

#include <vgt/types.h>

struct HalfEdge {
    Vertex* v;
    HalfEdge n;
    HalfEdge o;
};

#endif// VGT_HALF_EDGE_CLS_H
