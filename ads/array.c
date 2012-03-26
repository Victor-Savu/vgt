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
            if (arr->nd != (1 << ((arr->s)>>1))) { printf("Buba superblock %u has %u segments per data block!\n", arr->s, arr->nd); fflush(stdout);}
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
    if (arr->n == 0) {
        arr->od = 1;
        arr->os = 0;
        arr->nd = 1;
        arr->ns = 1;
        arr->s = 1;
        arr->d = 0;
        arr->nseg = (1<<arr->fact);
        arr->oseg = arr->nseg;
        arr->empty_db = 1;
    }
}

Array arrCreate(uint8_t elem_size, uint8_t fact)
{
    // TODO: limit the segment size and check that it is at least 8 bytes

    Array arr = oCreate(sizeof (struct Array));
    arr->index = oCreate(4 * sizeof (Obj));
    arr->index_size = 4;
    arr->fact = fact;
    arr->element_size = elem_size;
    arr->segment_size = elem_size << arr->fact;
    arr->index[0] = oCreate(arr->segment_size);
    arr->nseg = (1 << arr->fact);
    arr->oseg = arr->nseg;
    arr->od = 1;
    arr->os = 0;
    arr->nd = 1;
    arr->ns = 1;
    arr->s = 1;
    arr->empty_db = 1;
    return arr;
}

void arrDestroy(Array restrict arr)
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
    if (!arr || (pos>>arr->fact) >= arr->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    memcpy(arrGet(arr, pos), o, arr->element_size);
}

void arrPush(Array restrict arr, Obj restrict o)
{
    // if there is a non-full segment, increase its occupancy
    if (arr->oseg < arr->nseg) arr->oseg++;
    // otherwise, grow the array and set the new segment occupancy to 1
    else { arr_grow(arr); arr->oseg = 1; }

    Obj const dest = arr->index[arr->d-1 - arr->empty_db]+ arr->segment_size * (arr->od-1) + arr->element_size * (arr->oseg-1);

    memcpy(dest, o, arr->element_size);

//    arrSet(arr, o, ((arr->n-1)<<arr->fact) | (arr->oseg-1));
}

void arrPop(Array arr)
{
    if (!arr || !arr->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    // decrease the occupancy of the last segment
    arr->oseg--;

    // if there are no more elements in the segment, shrink the array and set the occupancy of the last segment to full
    if (!arr->oseg) { arr_shrink(arr); arr->oseg = arr->nseg; }
}

Obj arrGet(Array restrict arr, uint64_t p)
{
    uint64_t elm = p & (arr->nseg-1);
    uint64_t pos = p >> arr->fact;

    if (!arr || pos >= arr->n) {
        fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
        exit(EXIT_FAILURE);
    }
    if (pos == 0) return oCast(char*, arr->index[0]) + arr->element_size * elm;
    if (pos == 1) return arr->index[1] + arr->element_size * elm;
    if (pos == 2) return oCast(char*, arr->index[1]) + arr->segment_size + arr->element_size * elm;

    //ignore printf("Accessing element #%lu ", pos); fflush(stdout);
    pos+= 1;
    const uint64_t k = math_log2_uint64(pos); // the number of the superblock
    //ignore printf("from superblock #%u ", k); fflush(stdout);

    const uint64_t kdiv2 = k>>1;
    const uint64_t oneShlKdiv2 = (1<<kdiv2);
    const uint64_t notKdiv2 = oneShlKdiv2-1;

    const uint64_t mask_b = (notKdiv2 << kdiv2) << (k&1); // the first floor(k/2) bits after the most significant bit in k
    const uint64_t mask_seg = (oneShlKdiv2 << (k&1)) - 1; // the least significant ceil(k/2) bits in k

    const uint64_t b = ((pos & mask_b) >> kdiv2) >> (k&1); // the index of the data block in the k-th superblock
    //ignore printf("data block #%lu ", b); fflush(stdout);

    const uint64_t seg = pos & mask_seg; // the index of the data segment in the b-th data block
    //ignore printf("segment  #%lu\n   ---- using mask_b %lu   and mask_e %lu\n", e, mask_b, mask_e); fflush(stdout);

    char* d = arr->index[(notKdiv2 << 1) + (k&1) * oneShlKdiv2 + b]; // access the corresponding data block
    d += arr->element_size * ((seg << arr->fact) | elm);  // access the element within
    return d;
}

uint64_t arrSize(Array restrict arr)
{
    return (arr->n)?(((arr->n-1) << arr->fact)|arr->oseg):(0);
}

void printStatus(Array restrict arr)
{
    //printf("Array of %lu elements, %lu segments, %u superblocks per segment, %u data blocks per superblock, and %shaving an extra data block. index size: %u. segment size: %lu. element size: %lu.\n", ((arr->n-1) << arr->fact) + arr->oseg, arr->n, arr->ns, arr->nd, (arr->empty_db)?(""):("not "), arr->index_size, arr->segment_size, arr->element_size); fflush(stdout);
    printf("Array of %lu elements.\n\t%u superblocks[%u:%u]\n\t%u data blocks[%u:%u] %s\n\t%lu segments[%u:%u]\n\tindex size: %u. segment size: %lu. element size: %lu.\n",
            ((arr->n)?(((arr->n-1) << arr->fact) + arr->oseg):(0)),
            arr->s, arr->os, arr->ns,
            arr->d, arr->od, arr->nd, (arr->empty_db)?("(+1)"):(""),
            arr->n, arr->oseg, arr->nseg,
            arr->index_size, arr->segment_size, arr->element_size); fflush(stdout);
}
