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
        Obj o = v->dat;
        v->dat = oCreate(v->elem_size * v->mem);
        memcpy(v->dat, o, v->n * v->elem_size);
    }

    v->n++;
}

void vec_shrink(Vector v)
{
    if ((v->n <= (v->mem >> 2)) && (v->mem > VEC_MIN_SIZE)) {
        v->mem >>= 1;
        Obj o = v->dat;
        v->dat = oCreate(v->elem_size * v->mem);
        memcpy(v->dat, o, v->n * v->elem_size);
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

Obj vecGet(Vector restrict v, uint64_t pos)
{
    if (!v || pos >= v->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    return oCast(char*, v->dat) + pos * v->elem_size;
}

void vecPush(Vector restrict v, Obj restrict o)
{
    vec_grow(v);
    memcpy(vecGet(v, v->n-1), o, v->elem_size);
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
    return v->dat + v->n * v->elem_size;
}

uint64_t vecSize(Vector restrict v)
{
    return v->n;
}

