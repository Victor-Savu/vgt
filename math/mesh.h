
#ifndef VGT_MESH_H
#define VGT_MESH_H

// public class
#include <math/mesh_cls.h>

bool sxIsZeroSimplex(Mesh a);
bool sxIsOneSimplex(Mesh a);
bool sxIsTwoSimplex(Mesh a);
bool sxIsThreeSimplex(Mesh a);

const Mesh sxZeroSimplex(Vec* pos);
const Mesh sxOneSimplex(Mesh a, Mesh b);
const Mesh sxTwoSimplex(Mesh a, Mesh b, Mesh c);
const Mesh sxThreeSimplex(Mesh a, Mesh b, Mesh c, Mesh d);


#endif//VGT_MESH_H
