#ifndef VGT_TET_H
#define VGT_TET_H

#include <vgt/types.h>

enum TetEdge {AB=0x0, AC=0x1, AD=0x2, BD=0x3, DC=0x4, CB=0x5};
enum TetFacet {A = 0x0, oA = 0x0, BDC = 0x0, B = 0x1, oB = 0x1, ACD = 0x1 , C= 0x2, oC = 0x2, ADB = 0x2, D = 0x3, oD = 0x3, ABC = 0x3, INVALID_FACET = 0x4};
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

// Checks the vertices of a tetrahedron against a given vertex and returns
// the vertex label (A, B, C, D) uppon a match and INVALID_FACET on no match
TetVertex tetVertexLabel(Tet t, Vertex * v);

void tetRot(Tet restrict t, TetVertex v);

void tetPrint(Obj tet, FILE* f);

void tetRenderSolid(Tet t);
void tetRenderWireframe(Tet t);
void tetRenderCircumsphere(Tet t);

bool tetIsLegit(Tet t);

#endif//VGT_TET_H
