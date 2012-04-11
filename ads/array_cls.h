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


#endif//ADS_ARRAY_CLS_H
