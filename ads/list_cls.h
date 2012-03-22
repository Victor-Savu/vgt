#ifndef VGT_LIST_CLS_H
#define VGT_LIST_CLS_H

#include <vgt/types.h>


struct List {
   ListElement head;
   uint size;
   CompareMethod cmp;
   DeleteMethod del;
};

#endif//VGT_LIST_CLS_H
