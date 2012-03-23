#ifndef ADS_RED_BLACK_NODE_H
#define ADS_RED_BLACK_NODE_H

#include <ads/types.h>


RedBlackNode rbnCreate(Obj data, const byte props);

bool rbnIsRed(const RedBlackNode node);

RedBlackNode rbnRotate(RedBlackNode h);

void rbnColorFlip(RedBlackNode x);


#endif//ADS_RED_BLACK_NODE_H
