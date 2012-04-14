#ifndef ADS_QUEUE_CLS_H
#define ADS_QUEUE_CLS_H

#include <ads/types.h>

// TODO: Improve the worst-case time bound
struct Queue {
    Stack enqueue;
    Stack dequeue;
};

#endif//ADS_QUEUE_CLS_H
