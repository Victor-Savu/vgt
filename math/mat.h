#ifndef MATH_MAT_H
#define MATH_MAT_H

#include <math/types.h>


Mat* matCopy(Mat* restrict a, Mat* restrict b);
Mat* matScale(const Mat* restrict a, real s,  Mat* restrict b);
Mat* matTranspose(Mat* restrict a, Mat* restrict b);
Mat* matMul(Mat* restrict a, Mat* restrict b, Mat* restrict c);
Vertex* matCross(Mat* restrict a, Vertex* restrict b, Vertex* restrict c);

Mat* matScaleI(Mat* restrict a, real s);
Mat* matTransposeI(Mat* restrict a);
Mat* matMulI(Mat* restrict a, Mat* restrict b);
Vertex* matCrossI(Mat* restrict a, Vertex* restrict b);

#endif//MATH_MAT_H
