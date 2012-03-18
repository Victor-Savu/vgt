#ifndef MATH_FRAME_H
#define MATH_FRAME_H

#include <math/types.h>

Frame frCreate();
void frDestroy(Frame restrict f);

Frame frRotate(Frame restrict f, Mat* restrict r, Frame restrict g);

Frame frTranslate(Frame restrict f, Mat* restrict r, Frame restrict g);

Frame frCompose(Frame restrict f, Mat* restrict r, Frame restrict g);


#endif//MATH_FRAME_H
