#ifndef ADS_QUEUE_H
#define ADS_QUEUE_H

#include <ads/types.h>

Queue qCreate(size_t elem_size);

Queue qDestroy();

#define qPush(queue, object)   qPushSafe((queue), (object), sizeof (*(object)), __FILE__, __func__, __LINE__)
void qPushSafe(Queue restrict q, const Obj restrict o, size_t objsize, const char* restrict filename, const char* restrict funcname, int lineno);

void qPushArray(Queue restrict q, Array restrict a);

Obj qFront(Queue restrict q);

Obj qBack(Queue restrict q);

void qPop(Queue restrict q);

uint64_t qSize(Queue restrict q);

bool qIsEmpty(Queue restrict q);

#endif//ADS_QUEUE_CLS_H
