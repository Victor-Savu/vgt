#ifndef VGT_DELAUNAY_CLS_H
#define VGT_DELAUNAY_CLS_H

#include <vgt/types.h>

#include <math/obj.h>
#include <math/vertex.h>
#include <ads/array.h>
#include <vgt/tet_cls.h>
#include <vgt/tet.h>

#include <pthread.h>

#define DELAUNAY_RENDER_CIRCUMSPHERE    0x1
#define DELAUNAY_RENDER_BOUNDING_TET    0x2


struct Delaunay {
    Array v;
    Array t;
    // the bounding tetrahedron
    Vertex* A;
    Vertex* B;
    Vertex* C;
    Vertex* D;

    pthread_mutex_t mutex;
    bool rendering;
};


#endif//VGT_DELAUNAY_CLS_H
