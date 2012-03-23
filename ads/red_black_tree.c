#include <ads/red_black_tree.h>
#include <ads/red_black_tree_cls.h>

#include <ads/red_black_node.h>
#include <ads/red_black_node_cls.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ads/types.h>
#include <ads/list.h>
#include <math/obj.h>

RedBlackTree
rbtCreate(const CompareMethod cmp, const DeleteMethod del)
{
    safe( if (!cmp) return 0; );
    RedBlackTree tree = oCreate(sizeof (struct RedBlackTree));

    tree->cmp = cmp;
    tree->del = (del)?(del):(free);

    return tree;
}

void
rbtInsert(RedBlackTree tree, Obj data)
{
    safe( if (!tree) return; ); // TODO: should data be freed in this case?

    if (!tree->root) {
        if( (tree->root = rbnCreate(data, RED)) ) tree->depth = 1;
        return;
    }

    // create a stack for traversing the tree and returning along the same path
    RedBlackNode* stack[tree->depth +1];
    safe( memset(stack, 0, tree->height * sizeof (RedBlackNode*)); );
    stack[0] = &tree->root;

    byte depth = 1;
    RedBlackNode** top = stack;

    // go down the tree and up the stack
    while (**top) {
        if (rbnIsRed((**top)->left) && rbnIsRed((**top)->right)) rbnColorFlip(**top);
        if (tree->cmp(data, (**top)->data) < 0) {
            // insert into the left subtree
            *(top+1) = &(**top)->left;
            depth++; top++;
        } else {
            // insert into the right subtree
            *(top+1) = &(**top)->right;
            depth++; top++;
        }
    }

    // add the new node
    **top = rbnCreate(data, RED);
    if (**top && (tree->depth < depth)) tree->depth = depth;

    // go up the tree and down the stack
    while (depth--) {
        top--;
        if (rbnIsRed((**top)->right)) **top = rbnRotate(**top);
        if (rbnIsRed((**top)->left) && rbnIsRed((**top)->left->left)) **top = rbnRotate(**top);
    }

}

void
rbtDelete(RedBlackTree tree, Obj data)
{
}

List
rbtSearch(RedBlackTree tree, Obj data)
{
    return 0;
}

void
rbtDestroy(RedBlackTree tree)
{
}

