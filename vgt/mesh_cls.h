#ifndef VGT_MESH_CLS_H
#define VGT_MESH_CLS_H

#include <vgt/types.h>

struct Mesh {
    ind n_edges;
    ind n_vertices;
    Vec* vertices;
    struct Edge* edges;
};

#endif//VGT_MESH_CLS_H
