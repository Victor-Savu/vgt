#include <ads/vector.h>
#include <ads/vector_cls.h>

#include <math/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint64_t VEC_MIN_SIZE = 16;

void vec_grow(Vector v)
{
    if (v->n >= v->mem) {
        v->mem <<= 1;
        v->dat = realloc(v->dat, v->elem_size * v->mem);
    }

    v->n++;
}

void vec_shrink(Vector v)
{
    if ((v->n <= (v->mem >> 2)) && (v->mem > VEC_MIN_SIZE)) {
        v->mem >>= 1;
        v->dat = realloc(v->dat, v->elem_size * v->mem);
    }

    v->n--;
}

Vector vecCreate(size_t elem_size)
{
    Vector v = oCreate(sizeof(struct Vector));
    v->dat = oCreate(elem_size * (VEC_MIN_SIZE));
    v->mem = VEC_MIN_SIZE;
    v->elem_size = elem_size;
    return v;
}

void vecDestroy(Vector restrict v)
{
    oDestroy(v->dat);
    oDestroy(v);
}

void vecClear(Vector restrict v)
{
    v->n = 0;
    v->mem = VEC_MIN_SIZE;
    v->dat = realloc(v->dat, v->elem_size * VEC_MIN_SIZE);
}

Obj vecGet(Vector restrict v, uint64_t pos)
{
    if (!v || pos >= v->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    return oCast(char*, v->dat) + pos * v->elem_size;
}

Obj vecPushSafe(Vector restrict v, const Obj restrict o, size_t objsize, const char* restrict filename, const char* restrict funcname, int lineno)
{
    vec_grow(v);
    return oCopyTo(vecGet(v, v->n-1), o, v->elem_size);
}

void vecPop(Vector restrict v)
{
    vec_shrink(v);
}

Obj vecBegin(Vector restrict v)
{
    return v->dat;
}

Obj vecEnd(Vector restrict v)
{
    return oCast(char*, v->dat) + v->n * v->elem_size;
}

uint64_t vecSize(Vector restrict v)
{
    return v->n;
}

Vector vecSort(Vector restrict v, int(*cmp)(const void *, const void *))
{
    qsort(v->dat, v->n, v->elem_size, cmp);
    return v;
}
