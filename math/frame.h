#ifndef MATH_FRAME_H
#define MATH_FRAME_H

#include <math/types.h>

/* Public class */
#include <math/frame_cls.h>


Frame frCreate();
void frDestroy(Frame restrict f);

Frame frRotate(Frame restrict f, Mat* restrict r, Frame restrict g);

Frame frTranslate(Frame restrict f, Vertex* restrict p, Frame restrict g);

Frame frCompose(Frame restrict f, Frame restrict r, Frame restrict g);

Vertex* frTransform(Frame restrict f, Vertex* restrict p, Vertex* restrict t);

Vertex* frTransformI(Frame restrict f, Vertex* restrict p);

#endif//MATH_FRAME_H
