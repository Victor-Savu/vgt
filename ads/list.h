#ifndef ADS_LIST_H
#define ADS_LIST_H

#include <ads/types.h>

/*
   Creates an empty list.

   @arg cmp takes two objects stored in the list and returns 1 if they are in
   the desired order and 0 if not.

   @arg del is called on each object upplon removal from the list. We say
   that the list "owns" the object of the method frees the memory
   allocated for that object.

   @return an empty list 
*/
List lCreate(const CompareMethod cmp, const DeleteMethod del);

/*
   Destroys the list after removing all elements.

   @arg l the list to be destroyed.
*/
void lDestroy(List l);


/*
   Inserts an object into the list

   @return a handle to the element inserted
*/
ListElement lInsert(List l, ListElement at, Obj o);

/*
   Inserts a list element object into the list. This does not allocate extra
   memory, since the element is supposedly created.

   @return a handle the inserted element
*/
ListElement lImport(List l, ListElement at, ListElement e);


List lSplit(List l);

List lMerge(List l, List r);

/*
   Sorts the list in-place using the CompareMethod object provided at
   creation time.

   @return the sorted list
*/
List lSort(List l);

/*
   Access the start of the list.
*/
ListElement lBegin(List l);

/*
   Iterate forward through the list.

   @arg l is the list we want to iterate
   @arg e is the element we iterate from. if null, it signals we want our
   first iteration to access the start of the list.

   @return the element after the one given as input. If @e was null, it
   returns the first element in the list. If the list is empty or the
   iteration reached the end, null is returned.
*/
ListElement lNext(List l, ListElement e);

/*
   Querries the size of a list.

   @return the number of elements in the list.
*/
uint64_t lSize(List l);

/*
   Removes the next element from the list.

   @arg l the list the element is to be removed from.
   @arg e the element preceedign the one to be removed. if null, the
   first element in the list is removed.
   @return the removed element
*/
ListElement lRemove(List l, ListElement e);


/*
   Removes and deletes the next element from the list.

   @arg l the list the element is to be deleted from.
   @arg e the element preceedign the one to be deleted. if null, the
   first element in the list is deleted.
*/
void lDelete(List l, ListElement e);

/*
   Retrieve the data from a list element.
*/
Obj leGet(ListElement e);


#endif//ADS_LIST_H
