#ifndef VGT_TET_CLS_H
#define VGT_TET_CLS_H

#include <vgt/types.h>

struct Tet {
    Vertex* v[4];
    Tet n[4];
    // a map. Each neighbor of this tet sees it as either opposite to a (00), b (01), c (10), or d (11).
    byte m;
};

#endif//VGT_TET_CLS_H
