#ifndef VGT_DELAUNAY_H
#define VGT_DELAUNAY_H

#include <vgt/types.h>

Delaunay delCreate(Vertex (*hull)[4]);

void delDestroy(Delaunay restrict d);

bool delCheck(Delaunay d);

void delDisplay(Delaunay restrict d, int tet);

void delInsert(Delaunay restrict d, Vertex* restrict p);

bool delIsBounding(Delaunay restrict del, Tet restrict t);

bool delIsOnBoundary(Delaunay restrict del, Vertex* v);


#endif//VGT_DELAUNAY_H
