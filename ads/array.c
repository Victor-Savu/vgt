#include <ads/array.h>
#include <ads/array_cls.h>

#include <math/obj.h>
#include <math/algorithms.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool rand_init = false;

void arr_grow(Array restrict arr) {
  safe(if (!arr) {
    fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
    exit(EXIT_FAILURE);
  });

  // if the last nonempty data block is full
  if (arr->od == arr->nd) {
    // ignore printf("Last non-empty data block was full\n"); fflush(stdout);
    // if the last superblock is full
    if (arr->os == arr->ns) {
      //   ignore printf("Last superblock was full\n"); fflush(stdout);
      // increment s
      arr->s++;
      // if s is odd, double the number of data blocks in a superblock
      if (arr->s & 1) arr->ns <<= 1;
      // otherwise, double the number of segments in a data block
      else
        arr->nd <<= 1;
      // set the occupancy of the last superblock to empty
      arr->os = 0;
    } else {
      // ignore printf("Last superblock was not yet full\n"); fflush(stdout);
    }
    // if there are no empty data blocks
    if (!arr->empty_db) {
      //  ignore printf("There are no empty data blocks.\n"); fflush(stdout);
      // if the index block is full, reallocate it to twice its current size
      if (arr->d == arr->size) {
        arr->size <<= 1;
        arr->begin = realloc(arr->begin, arr->size * sizeof(Obj));
        arr->end = realloc(arr->end, arr->size * sizeof(Obj));
      }
      // allocate a new data block and store its pointer in the index block
      //  ignore printf("New data block with %u segments.\n", arr->nd);
      if (arr->nd != (1 << ((arr->s) >> 1))) {
        printf("Buba superblock %u has %u segments per data block!\n", arr->s,
               arr->nd);
        fflush(stdout);
      }
      arr->end[arr->d] = arr->begin[arr->d] =
          malloc(arr->segment_size * arr->nd);
      // arr->end[arr->d] = oCast(char*, arr->begin[arr->d]) + arr->segment_size
      // * arr->nd;
    } else {
      //    ignore printf("There was an empty data block.\n"); fflush(stdout);
    }
    // >>> personal touch << Now, we know there is no empty data block left
    arr->empty_db = 0;
    // increment d and the number of data blocks in occupying the last
    // superblock
    arr->d++;
    arr->os++;
    // set the occupancy of the last datablock to empty
    arr->od = 0;
  } else {
    //  ignore printf("Last non-empty data block was not full\n");
    //  fflush(stdout);
  }
  // increment n and the number of elements occupying the last non-empty data
  // block
  arr->n++;
  arr->od++;
}

void arr_shrink(Array restrict arr) {
  safe(if (!arr || !arr->n) {
    fprintf(stderr, "[x] %s: Shrinking an empty or void array.\n", __func__);
    exit(EXIT_FAILURE);
  });

  // decrement n and the number of elements occupying the last nonempty data
  // block
  arr->n--;
  arr->od--;
  // if the last nonempty datablock is now empty
  if (arr->od == 0) {
    // if there is another empty data block, deallocate it
    if (arr->empty_db) free(arr->begin[arr->d]);
    // if the index block is a quarter full, reallocate it to half its size
    if (arr->size > (arr->d << 2)) {
      arr->size >>= 1;
      arr->begin = realloc(arr->begin, arr->size * sizeof(Obj));
      arr->end = realloc(arr->end, arr->size * sizeof(Obj));
    }
    // decrement d and the number of data blocks occupying the last superblock
    arr->d--;
    arr->os--;
    // if the last superblock is empty
    if (arr->os == 0) {
      // decrement s
      arr->s--;
      // if s is even, half the number of data blocks in a superblock
      if (!(arr->s & 1)) arr->ns >>= 1;
      // otherwise, halve the number of segments in a data block
      else
        arr->nd >>= 1;
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
    arr->nseg = (1 << arr->fact);
    arr->oseg = arr->nseg;
    arr->empty_db = 1;
  }
}

Array arrCreate(uint8_t elem_size, uint8_t fact) {
  if (!rand_init) {
    srand(time(0));
    rand_init = true;
  }

  // TODO: limit the segment size and check that it is at least 8 bytes

  Array arr = oCreate(sizeof(struct Array));
  arr->begin = oCreate(4 * sizeof(Obj));
  arr->end = oCreate(4 * sizeof(Obj));
  arr->size = 4;
  arr->fact = fact;
  arr->element_size = elem_size;
  arr->segment_size = elem_size << arr->fact;
  arr->end[0] = arr->begin[0] = oCreate(arr->segment_size);
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

void arrDestroy(Array restrict arr) {
  if (!arr) return;
  Obj* const end = arr->begin + arr->d;
  Obj* i = arr->begin;
  for (i = arr->begin; i < end; i++) oDestroy(*i);
  if (arr->empty_db) oDestroy(*end);
  oDestroy(arr->begin);
  oDestroy(arr->end);
  oDestroy(arr);
}

Array arrCopy(Array restrict arr) {
  check(arr);
  Array c = oCopy(arr, sizeof(struct Array));
  c->begin = oCopy(arr->begin, arr->size * sizeof(Obj));
  c->end = oCopy(arr->end, arr->size * sizeof(Obj));
  uint32_t i = 0;
  if (arr->d) {
    for (i = 0; i < arr->d - 1; i++) {
      c->begin[i] = oCopy(arr->begin[i], oCast(char*, arr->end[i]) -
                                             oCast(char*, arr->begin[i]));
      c->end[i] = oCast(char*, c->begin[i]) +
                  (oCast(char*, arr->end[i]) - oCast(char*, arr->begin[i]));
    }
    c->begin[i] = oCopy(arr->begin[i], arr->nd * arr->segment_size);
    c->end[i] = oCast(char*, c->begin[i]) + (arr->od - 1) * arr->segment_size +
                (arr->oseg) * arr->element_size;
  }
  if (arr->empty_db)
    c->end[arr->d] = c->begin[arr->d] =
        oCopy(arr->begin[arr->d],
              (arr->nd << ((arr->os == arr->ns) && (arr->ns & 1))) *
                  arr->segment_size);

  return c;
}

Obj arrSet(Array restrict arr, Obj restrict o, uint64_t pos) {
  if (!arr || (pos >> arr->fact) >= arr->n) {
    fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
    exit(EXIT_FAILURE);
  }
  Obj dest = arrGet(arr, pos);
  memcpy(dest, o, arr->element_size);
  return dest;
}

Obj arrPushSafe(Array restrict arr, const Obj restrict o, size_t objsize,
                const char* filename, const char* funcname, int lineno) {
  if (objsize != arr->element_size) {
    fprintf(stderr,
            "[x] %s:%s:%d: Trying to push element of different size onto the "
            "array.\n",
            filename, funcname, lineno);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  // if there is a non-full segment, increase its occupancy
  if (arr->oseg < arr->nseg) arr->oseg++;
  // otherwise, grow the array and set the new segment occupancy to 1
  else {
    arr_grow(arr);
    arr->oseg = 1;
  }

  Obj const dest = arr->end[arr->d - 1];
  arr->end[arr->d - 1] = oCast(char*, arr->end[arr->d - 1]) + arr->element_size;

  memcpy(dest, o, arr->element_size);

  return dest;
}

void arrPop(Array arr) {
  if (!arr || !arr->n) {
    fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
    exit(EXIT_FAILURE);
  }
  // there should always be something in the last non-empty segment
  check(arr->oseg);
  // decrease the occupancy of the last segment
  arr->oseg--;
  arr->end[arr->d - 1] = oCast(char*, arr->end[arr->d - 1]) - arr->element_size;

  // if there are no more elements in the segment, shrink the array and set the
  // occupancy of the last segment to full
  if (!arr->oseg) {
    arr_shrink(arr);
    arr->oseg = arr->nseg;
  }
}

Obj arrGet(Array restrict arr, uint64_t p) {
  usage(p < arrSize(arr));

  uint64_t elm = p & (arr->nseg - 1);
  uint64_t pos = p >> arr->fact;

  if (pos == 0) return oCast(char*, arr->begin[0]) + arr->element_size * elm;
  if (pos == 1) return arr->begin[1] + arr->element_size * elm;
  if (pos == 2)
    return oCast(char*, arr->begin[1]) + arr->segment_size +
           arr->element_size * elm;

  // ignore printf("Accessing element #%lu ", pos); fflush(stdout);
  pos += 1;
  const uint64_t k = algoLog2(pos);  // the number of the superblock
  // ignore printf("from superblock #%u ", k); fflush(stdout);

  const uint64_t kdiv2 = k >> 1;
  const uint64_t oneShlKdiv2 = (1 << kdiv2);
  const uint64_t notKdiv2 = oneShlKdiv2 - 1;

  const uint64_t mask_b =
      (notKdiv2 << kdiv2)
      << (k &
          1);  // the first floor(k/2) bits after the most significant bit in k
  const uint64_t mask_seg = (oneShlKdiv2 << (k & 1)) -
                            1;  // the least significant ceil(k/2) bits in k

  const uint64_t b =
      ((pos & mask_b) >> kdiv2) >>
      (k & 1);  // the index of the data block in the k-th superblock
  // ignore printf("data block #%lu ", b); fflush(stdout);

  const uint64_t seg =
      pos & mask_seg;  // the index of the data segment in the b-th data block
  // ignore printf("segment  #%lu\n   ---- using mask_b %lu   and mask_e %lu\n",
  // e, mask_b, mask_e); fflush(stdout);

  char* d = arr->begin[(notKdiv2 << 1) + (k & 1) * oneShlKdiv2 +
                       b];  // access the corresponding data block
  d += arr->element_size *
       ((seg << arr->fact) | elm);  // access the element within
  return d;
}

void arrForEach(Array arr, const ArrOperation op, Obj data) {
  if (!(arr && op)) return;
  uint64_t i = 0;
  // Obj* const stop_begin = arr->begin + arr->d;
  // Obj* const stop_end = arr->end + arr->d;
  Obj element;
  uint64_t j = 0;
  for (j = 0; j < arr->d; j++)
    for (element = arr->begin[j]; element < arr->end[j];
         i++, element += arr->element_size)
      op(i, element, data);
}

struct printing_kit {
  FILE* f;
  ObjPrint print;
  uint64_t nelem;
};

static inline void op(uint64_t i, Obj o, Obj k) {
  // Obj* const o = arrGet(arr, i);
  struct printing_kit* const kit = k;
  kit->print(o, kit->f);
  if (i + 1 != kit->nelem) fprintf(kit->f, ", ");
}
void arrPrint(const Array restrict arr, FILE* restrict f,
              const ObjPrint print) {
  check(arr && f && print);

  struct printing_kit kit = {f, print, arrSize(arr)};
  arrForEach(arr, op, &kit);

  safe(fflush(f););
}

inline void arrRandomSwap(Array restrict arr, const ObjRelocator relocate) {
  check(arr);

  const uint64_t size = arrSize(arr);
  if (size <= 1) return;
  const uint64_t rn = algoChooseU64(size - 1);
  Obj a = arrGet(arr, rn);
  Obj b = arrBack(arr);
  char tmp[arr->element_size];

  memcpy(tmp, a, arr->element_size);
  if (relocate) relocate(tmp);
  memcpy(a, b, arr->element_size);
  if (relocate) relocate(a);
  memcpy(b, tmp, arr->element_size);
  if (relocate) relocate(b);
}

struct dest_c_array {
  char* a;
  size_t elem_size;
};

inline static void copy_elem(uint64_t i, Obj o, Obj d) {
  const struct dest_c_array* dest = oCast(struct dest_c_array*, d);
  oCopyTo(dest->a + i * dest->elem_size, o, dest->elem_size);
}

inline Obj arrToC(const Array restrict arr) {
  struct dest_c_array c = {.a = oCreate(arr->element_size * arrSize(arr)),
                           .elem_size = arr->element_size};
  arrForEach(arr, copy_elem, &c);
  return c.a;
}

inline static void copy_ref(uint64_t i, Obj o, Obj d) { oCast(Obj*, d)[i] = o; }

inline Obj arrRefs(const Array restrict arr) {
  Obj* refs = oCreate(sizeof(Obj) * arrSize(arr));
  arrForEach(arr, copy_ref, refs);
  return refs;
}

inline static void push_ref(uint64_t i, Obj o, Obj d) {
  // arrPushSafe(d, o, sizeof (Obj), __FILE__, __func__, __LINE__);
  arrPush(d, &o);
}

inline Obj arrRefsArr(const Array restrict arr) {
  Array refs = arrCreate(sizeof(Obj), 1);
  arrForEach(arr, push_ref, refs);
  return refs;
}

inline size_t arrElementSize(const Array restrict arr) {
  return arr->element_size;
}

inline Obj arrFront(Array restrict arr) {
  if (!arrSize(arr)) {
    fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
    exit(EXIT_FAILURE);
  }
  return arr->begin[0];
}

inline bool arrIsEmpty(const Array restrict arr) { return (!arrSize(arr)); }

inline Obj arrBack(Array restrict arr) {
  if (arrIsEmpty(arr)) {
    fprintf(stderr, "[x] %s: Illegal memory access.\n", __func__);
    exit(EXIT_FAILURE);
  }
  // return oCast(char*, arr->begin[arr->d-1 - arr->empty_db]) +
  // arr->segment_size * (arr->od-1) + arr->element_size * (arr->oseg-1);
  if (arr->begin[arr->d - 1] == arr->end[arr->d - 1])
    check((arr->begin[arr->d - 1] != arr->end[arr->d - 1]));
  return oCast(char*, arr->end[arr->d - 1]) - arr->element_size;
}

struct object_finder {
  Obj what;
  uint64_t index;
  Obj where;
  size_t size;
};

inline static void compare_and_report(uint64_t i, Obj o, Obj x) {
  if (memcmp(o, oCast(struct object_finder*, x)->what,
             oCast(struct object_finder*, x)->size) == 0) {
    oCast(struct object_finder*, x)->where = o;
    oCast(struct object_finder*, x)->index = i;
  }
}

inline Obj arrFindSafe(const Array restrict arr, Obj what, size_t objsize,
                       uint64_t* i, const char* restrict filename,
                       const char* restrict funcname, int lineno) {
  if (arr->element_size < objsize) objsize = arr->element_size;
  struct object_finder fndr = {
      .what = what, .where = 0, .index = 0, .size = objsize};
  arrForEach(arr, compare_and_report, &fndr);
  if (i) *i = fndr.index;
  return fndr.where;
}

inline uint64_t arrSize(const Array restrict arr) {
  return (arr->n) ? (((arr->n - 1) << arr->fact) + arr->oseg) : (0);
}

inline void printStatus(const Array restrict arr) {
  printf(
      "Array of %lu elements.\n\t%u superblocks[%u:%u]\n\t%u data "
      "blocks[%u:%u] %s\n\t%lu segments[%u:%u]\n\tindex size: %u. segment "
      "size: %lu. element size: %lu.\n",
      ((arr->n) ? (((arr->n - 1) << arr->fact) + arr->oseg) : (0)), arr->s,
      arr->os, arr->ns, arr->d, arr->od, arr->nd,
      (arr->empty_db) ? ("(+1)") : (""), arr->n, arr->oseg, arr->nseg,
      arr->size, arr->segment_size, arr->element_size);
  fflush(stdout);
}
