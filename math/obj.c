#include <math/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Obj oCreate(size_t size) //__attribute__((alloc_size(1)))
{
    Obj restrict o = malloc(size);
    if (o) {
        memset(o, 0, size);
    } else {
        fprintf(stderr, "Out of memory.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    return o;
}

void oDestroy(Obj restrict o)
{
    if (o) free(o);
}

Obj oCopy(Obj restrict o, size_t size)// __attribute__((alloc_size(2)))
{
    Obj restrict c = malloc(size);
    if (c) {
        memcpy(c, o, size);
    } else {
        fprintf(stderr, "Out of memory.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    return c;
}
