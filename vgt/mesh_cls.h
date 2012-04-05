#ifndef VGT_MESH_CLS_H
#define VGT_MESH_CLS_H

#include <vgt/types.h>

struct Mesh {
    ind n_vert;
    Vertex* vert;
    Vertex* norm;

    ind n_edges;
    struct Edge* edges;
};

#endif//VGT_MESH_CLS_H
