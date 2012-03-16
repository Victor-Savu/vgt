#ifndef VGT_RED_BLACK_TREE_CLS_H
#define VGT_RED_BLACK_TREE_CLS_H

#include <vgt/types.h>

struct RedBlackTree {
    RedBlackNode root;
    CompareMethod cmp;
    DeleteMethod del;
    byte depth;
};


#endif//VGT_RED_BLACK_TREE_CLS_H
