#ifndef ADS_LIST_ELEMENT_H
#define ADS_LIST_ELEMENT_H

#include <ads/types.h>

/*
   Retrieve the data from a list element.
*/
Obj leGet(ListElement e);

ListElement leSplit(ListElement e);

ListElement leMerge(ListElement e, ListElement f, const CompareMethod cmp);

ListElement leSort(ListElement e, const CompareMethod cmp);

#endif//ADS_LIST_ELEMENT_H
