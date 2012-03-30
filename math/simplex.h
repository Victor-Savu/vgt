
#ifndef VGT_SIMPLEX_H
#define VGT_SIMPLEX_H

// public class
#include <math/simplex_cls.h>

bool sxIsZeroSimplex(Simplex a);
bool sxIsOneSimplex(Simplex a);
bool sxIsTwoSimplex(Simplex a);
bool sxIsThreeSimplex(Simplex a);

const Simplex sxZeroSimplex(Vec* pos);
const Simplex sxOneSimplex(Simplex a, Simplex b);
const Simplex sxTwoSimplex(Simplex a, Simplex b, Simplex c);
const Simplex sxThreeSimplex(Simplex a, Simplex b, Simplex c, Simplex d);


#endif//VGT_SIMPLEX_H
