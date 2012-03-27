#ifndef QUAKE_ROBUST_PREDICATES_H
#define QUAKE_ROBUST_PREDICATES_H

#include <math/types.h>

real orient2d(real* pa, real* pb, real* pc);

real orient2dfast(real* pa, real* pb, real* pc);

real orient3d(real* pa, real* pb, real* pc, real* pd);

real orient3dfast(real* pa, real* pb, real* pc, real* pd);

real incircle(real* pa, real* pb, real* pc, real* pd);

real incirclefast(real* pa, real* pb, real* pc, real* pd);

real insphere(real* pa, real* pb, real* pc, real* pd, real* pe);

real inspherefast(real* pa, real* pb, real* pc, real* pd, real* pe);

#endif//QUAKE_ROBUST_PREDICATES_H
