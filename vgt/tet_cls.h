#ifndef VGT_TET_CLS_H
#define VGT_TET_CLS_H

#include <vgt/types.h>

struct Tet {
    Vec* a;
    Vec* b;
    Vec* c;
    Vec* d;
    Tet oa;
    Tet ob;
    Tet oc;
    Tet od;
};

#endif//VGT_TET_CLS_H
