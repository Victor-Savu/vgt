#ifndef ADS_VECTOR_H
#define ADS_VECTOR_H

#include <ads/types.h>
#include <stddef.h>

Vector vecCreate(size_t elem_size);

void vecDestroy(Vector restrict v);

Obj vecGet(Vector restrict v, uint64_t pos);

void vecPush(Vector restrict v, Obj restrict o);

void vecPop(Vector restrict v);

Obj vecBegin(Vector restrict v);

Obj vecEnd(Vector restrict v);

uint64_t vecSize(Vector restrict v);

#endif//ADS_VECTOR_H

