#ifndef ADS_LIST_CLS_H
#define ADS_LIST_CLS_H

#include <ads/types.h>


struct List {
   ListElement head;
   uint64_t size;
   CompareMethod cmp;
   DeleteMethod del;
};

#endif//ADS_LIST_CLS_H
