#ifndef VGT_RED_BLACK_NODE_CLS_H
#define VGT_RED_BLACK_NODE_CLS_H

#include <vgt/types.h>

extern const byte RED;
extern const byte NRED;
extern const byte COLOR;
extern const byte NCOLOR;

struct RedBlackNode {
    Obj data;
    RedBlackNode left;
    RedBlackNode right;
    byte props;
};

#endif//VGT_RED_BLACK_NODE_CLS_H
