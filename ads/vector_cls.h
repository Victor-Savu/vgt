#ifndef ADS_VECTOR_CLS_H
#define ADS_VECTOR_CLS_H

#include <stdint.h>
#include <stddef.h>

struct Vector {
    void* dat;
    uint64_t mem;
    
    uint64_t n;
    size_t elem_size;
};

#endif//ADS_VECTOR_CLS_H
