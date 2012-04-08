#ifndef VGT_DELAUNAY_CLS_H
#define VGT_DELAUNAY_CLS_H

#include <vgt/types.h>

#include <math/obj.h>
#include <math/vertex.h>
#include <ads/array.h>
#include <vgt/tet_cls.h>
#include <vgt/tet.h>

#include <pthread.h>

struct Delaunay {
    Array v;
    Array t;
    // the bounding tetrahedron
    Vertex* A;
    Vertex* B;
    Vertex* C;
    Vertex* D;

    pthread_mutex_t mutex;
    bool render_circ;
};

inline
Delaunay delCreate(Vertex (*hull)[4])
{
    Delaunay d = oCreate(sizeof (struct Delaunay));
    d->v = arrCreate(sizeof (Vertex), 2);
    d->t = arrCreate(sizeof (struct Tet), 2);

    struct Tet t = {
        {   arrPush(d->v, &(*hull)[0]),
            arrPush(d->v, &(*hull)[1]),
            arrPush(d->v, &(*hull)[2]),
            arrPush(d->v, &(*hull)[3]) },
        {0, 0, 0, 0}, 0 };

    d->A = t.v[A];
    d->B = t.v[B];
    d->C = t.v[C];
    d->D = t.v[D];
    d->render_circ = false;

    arrPush(d->t, &t);

    pthread_mutex_init(&d->mutex, 0);

    return d;
}

inline
void delDestroy(Delaunay restrict d)
{
    arrDestroy(d->v);
    arrDestroy(d->t);
    pthread_mutex_destroy(&d->mutex);
    oDestroy(d);
}


inline
bool delIsBounding(Delaunay restrict del, Tet restrict t) {
    return delIsOnBoundary(del, t->v[0])
        || delIsOnBoundary(del, t->v[1])
        || delIsOnBoundary(del, t->v[2])
        || delIsOnBoundary(del, t->v[3]);
}

inline
bool delIsOnBoundary(Delaunay restrict del, Vertex* v) {
    return (v == del->A) || (v == del->B) || (v == del->C) || (v == del->D);
}

#endif//VGT_DELAUNAY_CLS_H
