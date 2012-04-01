#ifndef VGT_DELAUNAY_H
#define VGT_DELAUNAY_H

#include <vgt/types.h>

Delaunay delCreate(Vec (*hull)[4]);

void delDestroy(Delaunay restrict d);

void delDisplay(Delaunay restrict d);

Delaunay delCopy(Delaunay restrict d);

void delInsert(Delaunay restrict d, Vec* restrict p);


#endif//VGT_DELAUNAY_H
