#include <ads/array.h>

#include <math/math.h>
#include <math/obj.h>

#include <math/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Array {
    Obj* index;
    uint32_t index_size;
    uint32_t s;
    uint32_t d;
    uint64_t n;
    uint64_t segment_size;
    uint32_t od;  // occupancy of the last non-empty data block
    uint32_t nd; // #segments in the last non-empty data block
    uint32_t os;  // occupancy of the last superblock
    uint32_t ns; // #data_blocks in the last superblock
    bool     empty_db; // is there an empty data block?
};

void arr_grow(Array restrict arr)
{
    safe(
    if (!arr) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    });

    // if the last nonempty data block is full
    if (arr->od == arr->nd) {
       // ignore printf("Last non-empty data block was full\n"); fflush(stdout);
        // if the last superblock is full
        if (arr->os == arr->ns) {
         //   ignore printf("Last superblock was full\n"); fflush(stdout);
            //increment s
            arr->s++;
            // if s is odd, double the number of data blocks in a superblock
            if (arr->s & 1) arr->ns <<= 1;
            // otherwise, double the number of elements in a data block
            else arr->nd <<= 1;
            // set the occupancy of the last superblock to empty
            arr->os = 0;
        } else {
           // ignore printf("Last superblock was not yet full\n"); fflush(stdout);
        }
        // if there are no empty data blocks
        if (!arr->empty_db) {
          //  ignore printf("There are no empty data blocks.\n"); fflush(stdout);
            // if the index block is full, reallocate it to twice its current size
            if (arr->d == arr->index_size) { arr->index_size <<= 1; arr->index = realloc(arr->index, arr->index_size * sizeof(Obj)); }
            // allocate a new data block and store its pointer in the index block
          //  ignore printf("New data block with %u segments.\n", arr->nd);
            arr->index[arr->d] = malloc(arr->segment_size * arr->nd);
        } else {
        //    ignore printf("There was an empty data block.\n"); fflush(stdout);
        }
        // >>> personal touch << Now, we know there is no empty data block left
        arr->empty_db = 0;
        // increment d and the number of data blocks in occupying the last superblock
        arr->d++; arr->os++;
        // set the occupancy of the last datablock to empty
        arr->od = 0;
    } else {
      //  ignore printf("Last non-empty data block was not full\n"); fflush(stdout);
    }
    // increment n and the number of elements occupying the last non-empty data block
    arr->n++; arr->od++;
}

void arr_shrink(Array restrict arr)
{
    safe(
    if (!arr || !arr->n) {
        fprintf(stderr, "[x] %s: Shrinking an empty or void array.\n", __func__);
        exit(EXIT_FAILURE);
    });

    //decrement n and the number of elements occupying the last nonempty data block
    arr->n--;  arr->od--;
    // if the last nonempty datablock is now empty
    if (arr->od == 0) {
        // if there is another empty data block, deallocate it
        if (arr->empty_db) free(arr->index[arr->d]);
        // if the index block is a quarter full, reallocate it to half its size
        if (arr->index_size > (arr->d << 2)) { arr->index_size >>=1; arr->index = realloc(arr->index, arr->index_size * sizeof(Obj)); }
        // decrement d and the number of data blocks occupying the last superblock
        arr->d--;  arr->os--;
        // if the last superblock is empty
        if (arr->os == 0) {
            // decrement s
            arr->s--;
            // if s is even, half the number of data blocks in a superblock
            if (!(arr->s&1)) arr->ns >>=1;
            // otherwise, halve the number of elements in a data block
            else arr->nd >>= 1;
            // set the occupancy of the last superblock to full
            arr->os = arr->ns;
        }
        // set the occupancy of the last data block to full
        arr->od = arr->nd;
        // >>> personal touch << Now, we know there is an empty data block
        arr->empty_db = 1;
    }
}

Array arrCreate(uint8_t block_size)
{
    Array arr = oCreate(sizeof (struct Array));
    arr->index = oCreate(sizeof (Obj));
    arr->index_size = 1;
    arr->index[0] = oCreate(block_size);
    arr->segment_size = block_size;
    arr->od = 1;
    arr->os = 0;
    arr->nd = 1;
    arr->ns = 1;
    arr->s = 1;
    arr->empty_db = 1;
    return arr;
}

void arrDestroy(Array arr)
{
    if (!arr) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    Obj* const end = arr->index + arr->d;
    Obj* i = arr->index;
    for (i=arr->index; i < end; i++) oDestroy(*i);
    if (arr->empty_db) oDestroy(*end);
    oDestroy(arr->index);
    oDestroy(arr);
}

void arrSet(Array restrict arr, Obj restrict o, uint64_t pos)
{
    if (!arr || pos >= arr->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    memcpy(arrGet(arr, pos), o, arr->segment_size);
}

void arrPush(Array restrict arr, Obj restrict o)
{
    arr_grow(arr);
    arrSet(arr, o, arr->n-1);
}

void arrPop(Array restrict arr)
{
    arr_shrink(arr);
}

Obj arrGet(Array arr, uint64_t pos)
{
    if (!arr || pos >= arr->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    if (pos == 0) return arr->index[0];
    if (pos == 1) return arr->index[1];
    if (pos == 2) return oCast(char*, arr->index[1]) + arr->segment_size;

    //ignore printf("Accessing element #%lu ", pos); fflush(stdout);
    pos+= 1;
    const uint64_t k = math_log2_uint64(pos); // the number of the superblock
    //ignore printf("from superblock #%u ", k); fflush(stdout);

    const uint64_t mask_b = (((1 << (k >> 1)) -1) << ( k >> 1 )) << (k&1); // the first floor(k/2) bits after the most significant bit in k
    const uint64_t mask_e = ((1 << ( k >> 1 )) << (k&1)) - 1; // the least significant ceil(k/2) bits in k

    const uint64_t b = ((pos & mask_b) >> ( k >> 1 )) >> (k&1); // the index of the data block in the k-th superblock
    //ignore printf("data block #%lu ", b); fflush(stdout);

    const uint64_t e = pos & mask_e; // the index of the data segment in the b-th data block
    //ignore printf("segment  #%lu\n   ---- using mask_b %lu   and mask_e %lu\n", e, mask_b, mask_e); fflush(stdout);

    char* d = arr->index[(((1 << (k>>1))-1) << 1) + (k&1) * (1 << (k>>1)) + b]; // access the corresponding data block
    d += arr->segment_size * e; // access the element within
    return d;
}

uint64_t arrSize(Array arr)
{
    return arr->n;
}
