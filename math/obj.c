#include <math/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Obj oCreate(size_t size)
{
    Obj o = malloc(size);
    if (o) {
        memset(o, 0, size);
    } else {
        fprintf(stderr, "Out of memory.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    return o;
}

void oDestroy(Obj o)
{
    if (o) free(o);
}
