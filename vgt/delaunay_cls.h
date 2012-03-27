#ifndef VGT_DELAUNAY_CLS_H
#define VGT_DELAUNAY_CLS_H

#include <vgt/types.h>

struct Delaunay {
    Array v;
    // TODO: Use a RedBlackTree for the edges in order to save memory when deleting
    Array e;
    ind valid;
};

#endif//VGT_DELAUNAY_CLS_H
