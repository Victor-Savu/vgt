#ifndef VGT_DELAUNAY_H
#define VGT_DELAUNAY_H

#include <vgt/types.h>

Delaunay delCreate(Vertex (*hull)[4]);

void delDestroy(Delaunay restrict d);

bool delCheck(Delaunay d);

void delDisplay(Delaunay d, int tet);

Array delInsert(Delaunay restrict d, Vertex* restrict p);

bool delIsBounding(Delaunay restrict del, Tet restrict t);

bool delIsOnBoundary(Delaunay restrict del, Vertex* v);

void delDropBoundary(Delaunay restrict del);

#endif//VGT_DELAUNAY_H
