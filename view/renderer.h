#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <view/types.h>

Renderer rCreate(const char* winname);

void rDestroy(Renderer r);

void rDisplay(Renderer r, Mesh m);

void rWait(Renderer r);

#endif//VIEW_RENDERER_H
