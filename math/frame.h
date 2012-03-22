#ifndef MATH_FRAME_H
#define MATH_FRAME_H

#include <math/types.h>

/* Public class */
#include <math/frame_cls.h>


Frame frCreate();
void frDestroy(Frame restrict f);

Frame frRotate(Frame restrict f, Mat* restrict r, Frame restrict g);

Frame frTranslate(Frame restrict f, Vec* restrict p, Frame restrict g);

Frame frCompose(Frame restrict f, Frame restrict r, Frame restrict g);

Vec* frTransform(Frame restrict f, Vec* restrict p, Vec* restrict t);

Vec* frTransformI(Frame restrict f, Vec* restrict p);

#endif//MATH_FRAME_H
