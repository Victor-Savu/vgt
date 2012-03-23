#ifndef ADS_RED_BLACK_H
#define ADS_RED_BLACK_H

#include <ads/types.h>

/*
   Dynamically allocates a RedBlackTree structure and sets
   the method used for comparing the stored data.

   @arg cmp is a method which, given pointers to two data
   elements, casts them to the appropriate type, extracts
   the values of the keys and compares them, returning
   a 0 on equality, a negative value if the former key is
   smaller and a positive value if the latter key is smaller.

   @arg del is a method which frees the memory allocated for the
   data blocks stored in the nodes of the tree. If null, the standard
   free() method will be used.

   @return A pointer to the RedBlackTree structure.
 */
RedBlackTree rbtCreate(const CompareMethod cmp, const DeleteMethod del);

/*
   Performs the insertion operation of a data element in the
   tree.
*/
void    rbtInsert(RedBlackTree tree, Obj data);

/*
   Performs a delete operation of one or more data elements. The memory is
   freed using the delete method provided at the creation of the tree.

   @arg tree is a pointer to the tree structure.
   @arg data is a pointer to a block of data whose key will compare
   equal to the keys of the elements which are to be deleted, according
   to the comparison method provided at the creation of the tree.
*/
void    rbtDelete(RedBlackTree tree, Obj data);

/*
   Performs a search operation on the tree.

   @arg tree is a pointer to the tree structure.
   @arg data is a pointer to a data element whose key will compare
   equal to the keys of the elements which are searched, according
   to the comparison method provided at the creation of the tree.
   The key is enclosed in the element itself and the method
   relies on the CompareMethod provided at the creation of the
   tree for extracting the key and performing the necessary
   comparisons.

   @return a pointer to a search result structure containing the data
   elements found.
*/
List rbtSearch(RedBlackTree tree, Obj data);

/*
   Destroys the tree by deallocating all its nodes and its structure. The data in
   each node is deleted using the method provided at tree creation.

   @arg tree is a pointer to the tree structure to be cleared and deallocated.
*/
void    rbtDestroy(RedBlackTree tree);



#endif//ADS_RED_BLACK_H
