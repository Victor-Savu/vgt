#include <ads/queue.h>
#include <ads/queue_cls.h>

#include <ads/array.h>

#include <math/obj.h>

Queue qCreate(size_t elem_size)
{
    Queue q = oCreate(sizeof(struct Queue));
    q->enqueue = arrCreate(elem_size, 1);
    q->dequeue = arrCreate(elem_size, 1);
    return q;
}

Queue qDestroy();

#define qPush(queue, object)   qPushSafe((queue), (object), sizeof (*(object)), __FILE__, __func__, __LINE__)
inline
void qPushSafe(Queue restrict q, const Obj restrict o, size_t objsize, const char* restrict filename, const char* restrict funcname, int lineno)
{
    ignore arrPushSafe(q->enqueue, o, objsize, filename, funcname, lineno);
}

inline static
void push_one(uint64_t i, Obj o, Obj q)
{
    qPushSafe(q, o, arrElementSize(oCast(Queue, q)->enqueue), __FILE__, __func__, __LINE__);
}

void qPushArray(Queue restrict q, Array restrict a)
{
    conjecture(q, "Null argument q"".");
    conjecture(a, "Null argument a"".");
    conjecture(arrElementSize(a) == arrElementSize(q->enqueue), "Tried to push the elements of an array onto a queue which has different element size.");
    arrForEach(a, push_one, q);
}


inline
Obj qFront(Queue restrict q)
{
    conjecture(q, "Null argument q"".");

    if (arrIsEmpty(q->dequeue)) {
        while (!arrIsEmpty(q->enqueue)) {
            arrPushSafe(q->dequeue, arrBack(q->enqueue), arrElementSize(q->enqueue), __FILE__, __func__, __LINE__);
            arrPop(q->enqueue);
        }
    }

    conjecture(!arrIsEmpty(q->dequeue), "Tried to access an element in an empty queue.");

    return arrBack(q->dequeue);
}

inline
Obj qBack(Queue restrict q)
{
    if (arrIsEmpty(q->enqueue)) return arrFront(q->dequeue); else return arrBack(q->enqueue);
}

inline
void qPop(Queue restrict q)
{
    if (qFront(q)) arrPop(q->dequeue);
}

inline
uint64_t qSize(Queue restrict q)
{
    usage(q);
    return arrSize(q->enqueue) + arrSize(q->dequeue);
}

inline
bool qIsEmpty(Queue restrict q)
{
    usage(q);
    return (qSize(q) == 0);
}
