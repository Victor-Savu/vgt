#ifndef ADS_VECTOR_H
#define ADS_VECTOR_H

#include <ads/types.h>
#include <stddef.h>

Vector vecCreate(size_t elem_size);

void vecDestroy(Vector restrict v);

Obj vecGet(Vector restrict v, uint64_t pos);

#define vecPush(vector, object)   vecPushSafe((vector), (object), sizeof (*object), __FILE__, __func__, __LINE__)
Obj vecPushSafe(Vector restrict v, const Obj restrict o, size_t objsize, const char* restrict filename, const char* restrict funcname, int lineno);

void vecPop(Vector restrict v);

Obj vecBegin(Vector restrict v);

Obj vecEnd(Vector restrict v);

uint64_t vecSize(Vector restrict v);

//void vecSort(Vector restrict v, CompareMethod cmp);
Vector vecSort(Vector restrict v, int(*cmp)(const void *, const void *));

#endif//ADS_VECTOR_H

