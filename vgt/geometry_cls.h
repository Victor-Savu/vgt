#ifndef VGT_GEOMETRY_CLS_H
#define VGT_GEOMETRY_CLS_H

#include <vgt/types.h>


struct Edge {
    Vert vert;
    Edge pair;
    Edge next;
    Face face;
};

struct Vert {
    struct Vec pos;
    Edge edge;
};

struct Face {
    Edge edge;
};

#endif//VGT_GEOMETRY_CLS_H
