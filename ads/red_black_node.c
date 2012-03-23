#include <ads/red_black_node.h>
#include <ads/red_black_node_cls.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ads/types.h>


const byte RED = 0x1;
const byte NRED = 0xFD;
const byte COLOR = 0x1;
const byte NCOLOR = 0xFD;


inline RedBlackNode rbnCreate(Obj data, const byte props) {

    RedBlackNode node = malloc(sizeof (struct RedBlackNode));

#ifdef SAFE_MODE
    if (!node) {
        printf("[x] Out of memory!\n");
        return 0;
    }
    memset(node, 0, sizeof (struct RedBlackNode));
#endif//SAFE_MODE

    node->data = data;
    node->props = props;
    node->left = node->right = 0;
    return node;
}

inline bool rbnIsRed(const RedBlackNode node) {
    return (node && (node->props & RED));
}


inline RedBlackNode rbnRotate(RedBlackNode h)
{
    safe( if (!h) return 0; );
    // test if the node has two red children, in which case the rotation cannot be performed
    safe( if ( rbnIsRed(h->left) && rbnIsRed(h->right)  ) return h; );

    // assume we want a right-rotation
    RedBlackNode x = h->left;
    // check assumption and make corrections
    if ( !rbnIsRed(x) ) {
        // assume it is a left-rotation
        x = h->right;
        // check assumption and quit if still false
        if ( !rbnIsRed(x) ) return h;
    }

    h->right = x->left;
    x->left = h;
    // set the color of x to the color of h
    x->props = (x->props & NCOLOR) | (h->props & COLOR);
    // make h red
    h->props |= RED;

    return x;
}

inline void rbnColorFlip(RedBlackNode x)
{
    // the props ^= COLOR operation flips the color bit
    safe( if (!x) return 0; );
    x->props ^= COLOR;
    safe( if (!x->left) return x; );
    x->left->props ^= COLOR;
    safe( if (!x->right) return x; );
    x->right->props ^= COLOR;
}

