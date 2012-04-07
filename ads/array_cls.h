#ifndef ADS_ARRAY_CLS_H
#define ADS_ARRAY_CLS_H

#include <ads/types.h>
#include <ads/array.h>

#include <math/obj.h>
#include <signal.h>

struct Array {
    Obj* begin;
    Obj* end;
    uint32_t size;
    uint32_t s;
    uint32_t d;
    uint64_t n;
    size_t segment_size;
    uint32_t od;  // occupancy of the last non-empty data block
    uint32_t nd; // #segments in the last non-empty data block
    uint32_t os;  // occupancy of the last superblock
    uint32_t ns; // #data_blocks in the last superblock
    bool     empty_db; // is there an empty data block?

    size_t element_size;
    uint64_t fact; // base 2 logarithm of the number of elements per segment
    uint32_t oseg; // occupancy of the last segment
    uint32_t nseg; // #elements per segment = 1<<fact
};


inline
size_t arrElementSize(Array restrict arr)
{
    return arr->element_size;
}


inline
Obj arrFront(Array restrict arr)
{
    if (!arrSize(arr)) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    return arr->begin[0];
}

inline
bool arrIsEmpty(Array restrict arr)
{
    return (!arrSize(arr));
}

inline
Obj arrBack(Array restrict arr)
{
    if (arrIsEmpty(arr)) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    //return oCast(char*, arr->begin[arr->d-1 - arr->empty_db]) + arr->segment_size * (arr->od-1) + arr->element_size * (arr->oseg-1);
    check( (arr->begin[arr->d-1] != arr->end[arr->d-1]) );
    return oCast(char*, arr->end[arr->d-1]) - arr->element_size;
}

inline
uint64_t arrSize(Array restrict arr)
{
    return (arr->n)?(((arr->n-1) << arr->fact)+arr->oseg):(0);
}

inline
void printStatus(Array restrict arr)
{
    printf("Array of %lu elements.\n\t%u superblocks[%u:%u]\n\t%u data blocks[%u:%u] %s\n\t%lu segments[%u:%u]\n\tindex size: %u. segment size: %lu. element size: %lu.\n",
            ((arr->n)?(((arr->n-1) << arr->fact) + arr->oseg):(0)),
            arr->s, arr->os, arr->ns,
            arr->d, arr->od, arr->nd, (arr->empty_db)?("(+1)"):(""),
            arr->n, arr->oseg, arr->nseg,
            arr->size, arr->segment_size, arr->element_size); fflush(stdout);
}

#endif//ADS_ARRAY_CLS_H
