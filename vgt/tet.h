#ifndef VGT_TET_H
#define VGT_TET_H

#include <vgt/types.h>

enum TetEdge {AB, AC, AD, BD, DC, CB};
enum TetFacet {A = 0x0, oA = 0x0, BDC = 0x0, B = 0x1, oB = 0x1, ACD = 0x1 , C= 0x2, oC = 0x2, ADB = 0x2, D = 0x3, oD = 0x3, ABC = 0x3};
typedef enum TetFacet TetNeighbour;
typedef enum TetFacet TetFace;
typedef enum TetFacet TetVertex;

// Tet x tells Tet y:
// Hi! I'm your new neighbor. For me, you live across fx.
// Tet y replies:
// Nice to meet you, x! For me, you live across fy. 
void tetConnect(Tet x, TetFace fx, Tet y, TetFace fy);

// After visiting a neighbor, a Tet wants to know which face of
// the neighbor leads to "home", so it consults the map
TetFace tetReadMap(byte m, TetNeighbour n);

#endif//VGT_TET_H
