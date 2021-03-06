#ifndef VGT_MESH_H
#define VGT_MESH_H

#include <vgt/types.h>

Mesh mCopy(Mesh restrict m);

void mDestroy(Mesh restrict m);

Mesh mClear(Mesh restrict m);

Mesh mReadOff(Mesh restrict m, const char* restrict filename);

void mDisplay(Mesh restrict m);

#endif//VGT_MESH_H
