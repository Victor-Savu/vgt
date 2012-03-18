#include <math/mat.h>
#include <math/mat_cls.h>

#include <math/obj.h>

Mat* matCopy(Mat* restrict a, Mat* restrict b)
{
    memcpy(b, a, sizeof(Mat));
    return b;
}

Mat* matScale(Mat* restrict a, real s, Mat* restrict b)
{
    Vec* restrict v0 = (*a);
    Vec* restrict v1 = (*a)+1;
    Vec* restrict v2 = (*a)+2;
    vScale(v0, s, (*b));
    vScale(v1, s, (*b)+1);
    vScale(v2, s, (*b)+2);
    return b;
}

Mat* matTranspose(Mat* restrict a, Mat* restrict b)
{
                             (*b)[0][1] = (*a)[1][0]; (*b)[0][2] = (*a)[2][0];
    (*b)[1][0] = (*a)[0][1];                          (*b)[1][2] = (*a)[2][1];
    (*b)[2][0] = (*a)[0][2]; (*b)[2][1] = (*a)[2][1];

    return b;
}

Mat* matMul(Mat* restrict a, Mat* restrict b, Mat* restrict c)
{
    (*c)[0][0] = (*a)[0][0] * (*b)[0][0] + (*a)[0][1] * (*b)[1][0] + (*a)[0][2] * (*b)[2][0];
    (*c)[0][1] = (*a)[0][0] * (*b)[0][1] + (*a)[0][1] * (*b)[1][1] + (*a)[0][2] * (*b)[2][1];
    (*c)[0][2] = (*a)[0][0] * (*b)[0][2] + (*a)[0][1] * (*b)[1][2] + (*a)[0][2] * (*b)[2][2];

    (*c)[1][0] = (*a)[1][0] * (*b)[0][0] + (*a)[1][1] * (*b)[1][0] + (*a)[1][2] * (*b)[2][0];
    (*c)[1][1] = (*a)[1][0] * (*b)[0][1] + (*a)[1][1] * (*b)[1][1] + (*a)[1][2] * (*b)[2][1];
    (*c)[1][2] = (*a)[1][0] * (*b)[0][2] + (*a)[1][1] * (*b)[1][2] + (*a)[1][2] * (*b)[2][2];

    (*c)[2][0] = (*a)[2][0] * (*b)[0][0] + (*a)[2][1] * (*b)[1][0] + (*a)[2][2] * (*b)[2][0];
    (*c)[2][1] = (*a)[2][0] * (*b)[0][1] + (*a)[2][1] * (*b)[1][1] + (*a)[2][2] * (*b)[2][1];
    (*c)[2][2] = (*a)[2][0] * (*b)[0][2] + (*a)[2][1] * (*b)[1][2] + (*a)[2][2] * (*b)[2][2];

    return c;
}

Vec* matCross(Mat* restrict a, Vec* restrict b, Vec* restrict c)
{
    (*c)[0] = vDot((*a)[0], b);
    (*c)[1] = vDot((*a)[1], b);
    (*c)[2] = vDot((*a)[2], b);
    return c;
}

Mat* matScaleI(Mat* restrict a, real s)
{
    Vec* restrict v0 = (*a);
    Vec* restrict v1 = (*a)+1;
    Vec* restrict v2 = (*a)+2;
    vScaleI(v0, s);
    vScaleI(v1, s);
    vScaleI(v2, s);
}

Mat* matTransposeI(Mat* restrict a)
{
    real r;
    r = (*a)[0][1]; (*a)[0][1] = (*a)[1][0]; (*a)[1][0] = r;
    r = (*a)[0][2]; (*a)[0][2] = (*a)[2][0]; (*a)[2][0] = r;
    r = (*a)[1][2]; (*a)[1][2] = (*a)[2][1]; (*a)[2][1] = r;
    return a;
}

Mat* matMulI(Mat* restrict a, Mat* restrict b)
{
    Mat c;
    return matCopy(matMul(a, b, &c), a);
}

Vec* matCrossI(Mat* restrict a, Vec* restrict b)
{
    Vec c;
    return vCopy(matCross(a, b, &c), b);
}

