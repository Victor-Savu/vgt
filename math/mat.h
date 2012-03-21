#ifndef MATH_MAT_H
#define MATH_MAT_H

#include <math/types.h>


Mat* matCopy(Mat* restrict a, Mat* restrict b);
Mat* matScale(Mat* restrict a, real s,  Mat* restrict b);
Mat* matTranspose(Mat* restrict a, Mat* restrict b);
Mat* matMul(Mat* restrict a, Mat* restrict b, Mat* restrict c);
Vec* matCross(Mat* restrict a, Vec* restrict b, Vec* restrict c);

Mat* matScaleI(Mat* restrict a, real s);
Mat* matTransposeI(Mat* restrict a);
Mat* matMulI(Mat* restrict a, Mat* restrict b);
Vec* matCrossI(Mat* restrict a, Vec* restrict b);

#endif//MATH_MAT_H
