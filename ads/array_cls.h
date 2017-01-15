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
    uint32_t s;  // index of the current superblock
    uint32_t d;  // index of the current datablock
    uint64_t n;  // number of segments
    size_t   segment_size;
    uint32_t od;  // occupancy of the last non-empty data block
    uint32_t nd;  // capacity of the last non-empty data block
    uint32_t os;  // occupancy of the last superblock
    uint32_t ns;  // capacity the last superblock
    uint32_t oseg; // occupancy of the last segment
    uint32_t nseg; // capacity of the last segment (constant = 1<<fact)
    bool     empty_db; // is there an empty data block?

    size_t element_size;
    uint64_t fact; // base 2 logarithm of the number of elements per segment
};


#endif//ADS_ARRAY_CLS_H
