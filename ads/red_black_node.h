#ifndef VGT_RED_BLACK_NODE_H
#define VGT_RED_BLACK_NODE_H

#include <vgt/types.h>


RedBlackNode rbnCreate(Obj data, const byte props);

bool rbnIsRed(const RedBlackNode node);

RedBlackNode rbnRotate(RedBlackNode h);

void rbnColorFlip(RedBlackNode x);


#endif//VGT_RED_BLACK_NODE_H
