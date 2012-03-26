#ifndef QUAKE_ROBUST_PREDICATES_H
#define QUAKE_ROBUST_PREDICATES_H

#define REAL double                      /* float or double */

REAL orient2d(REAL* pa, REAL* pb, REAL* pc);

REAL orient2dfast(REAL* pa, REAL* pb, REAL* pc);

REAL orient3d(REAL* pa, REAL* pb, REAL* pc, REAL* pd);

REAL orient3dfast(REAL* pa, REAL* pb, REAL* pc, REAL* pd);

REAL incircle(REAL* pa, REAL* pb, REAL* pc, REAL* pd);

REAL incirclefast(REAL* pa, REAL* pb, REAL* pc, REAL* pd);

REAL insphere(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe);

REAL inspherefast(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe);

#endif//QUAKE_ROBUST_PREDICATES_H
