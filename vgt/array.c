#include <vgt/array.h>

#include <stdlib.h>
#include <string.h>

struct Array {
    Obj* objects;
    unsigned int size;
};

Array arrCreate(unsigned int size)
{
    Array arr = malloc(sizeof (struct Array));
    if (!arr) return 0;
    arr->size = size;
    if (size) {
        arr->objects = malloc(size * sizeof (Obj));
        if (!arr->objects) {
            free(arr);
            return 0;
        }
    }
    return arr;
}

void arrDestroy(Array arr)
{
    free(arr->objects);
    free(arr);
}

Obj arrGet(Array arr, unsigned int pos)
{
    if (pos >= arr->size) return 0;
    return arr->objects[pos];
}

Obj* arrBegin(Array arr)
{
    return arr->objects;
}

Obj* arrEnd(Array arr)
{
    return arr->objects + arr->size;
}
