#ifndef MATH_FRAME_CLS_H
#define MATH_FRAME_CLS_H

#include <math/types.h>

#define FRAME_I  { MAT_I, VEC_0 }

struct Frame {
    Mat rot;
    Vec trans;
};


#endif//MATH_FRAME_CLS_H
