#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <view/types.h>

Renderer rCreate(const char* winname);

void rDestroy(Renderer r);

void rDisplayMesh(Renderer r, Mesh m);

void rDisplayDelaunay(Renderer r, Delaunay m);

void rWait(Renderer r);

void rWaitKey(Renderer r, char* c);

#endif//VIEW_RENDERER_H
