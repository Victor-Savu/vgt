#ifndef VGT_DELAUNAY_H
#define VGT_DELAUNAY_H

#include <vgt/types.h>

Delaunay delCreate(Vertex (*hull)[4]);

void delDestroy(Delaunay restrict d);

bool delCheck(Delaunay d);

void delDisplay(Delaunay restrict d);

Delaunay delCopy(Delaunay restrict d);

void delInsert(Delaunay restrict d, Vertex* restrict p);


#endif//VGT_DELAUNAY_H
