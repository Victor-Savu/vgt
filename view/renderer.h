#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <view/types.h>

Renderer rCreate(const char* restrict winname);

void rDestroy(Renderer restrict r);

void rDisplayMesh(Renderer restrict r, Mesh restrict m);

void rDisplayDelaunay(Renderer restrict r, Delaunay restrict m);

void rDisplaySpectrum(Renderer restrict r, Spectrum restrict s);

void rWait(Renderer r);

void rWaitKey(Renderer r, char* c);

#endif//VIEW_RENDERER_H
