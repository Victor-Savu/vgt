#ifndef ADS_RED_BLACK_TREE_CLS_H
#define ADS_RED_BLACK_TREE_CLS_H

#include <ads/types.h>

struct RedBlackTree {
    RedBlackNode root;
    CompareMethod cmp;
    DeleteMethod del;
    byte depth;
};


#endif//ADS_RED_BLACK_TREE_CLS_H
