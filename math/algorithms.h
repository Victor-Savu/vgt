#ifndef MATH_ALGORITHMS_H
#define MATH_ALGORITHMS_H

#include <math/types.h>
#include <math/obj.h>

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

static uint16_t algoRandomU16(void);
static uint32_t algoRandomU32(void);
static uint64_t algoRandomU64(void);

static uint64_t algoChooseU64(uint64_t n);

static uint64_t algoLog2(uint64_t x);


static inline
uint64_t algoChooseU64(uint64_t n)
{
    const uint64_t m = ((uint64_t)1 << (algoLog2(n)+1)) -1;
    uint64_t rez;
    do {
        rez = algoRandomU64() & m;
    } while (rez >= n);
    return rez;
}

static inline
uint32_t algoChooseU32(uint32_t n)
{
    const uint32_t m = (((uint64_t)1 << (algoLog2(n)+1)) -1);
    uint32_t rez;
    do {
        rez = algoRandomU32() & m;
    } while (rez >= n);
    return rez;
}

static inline
uint64_t algoRandomU64(void)
{
    return (((uint64_t)algoRandomU32() << 32) | (uint64_t) algoRandomU32());
}

static inline
uint32_t algoRandomU32(void)
{
    return (((uint32_t)algoRandomU16() << 16) | algoRandomU16());
}

static inline
uint16_t algoRandomU16(void)
{
    return (rand() & 32767);
}

static inline
double algoRandom(void)
{
    static const uint16_t s = 0x3ff0; // 1023 * 2^4
//  printf("s = %u\n", s);
    uint64_t r = algoRandomU64();
//  printf("r = %ld\n", r);
    uint16_t *b = oCast(uint16_t*, &r);
//  printf("b = <%u, %u, %u, %u>\n", b[0], b[1],b[2], b[3]);
    b[3] = (b[3] & 0xf) | s;
//  printf("b*= <%u, %u, %u, %u>\n", b[0], b[1],b[2], b[3]);
    return (*oCast(double*, b)) - oCast(double, 1.0);
}

static inline
double algoRandomDouble(double low, double high)
{
    if (low==high) return low;
    return low + (high-low) * algoRandom();
}

static inline
uint64_t algoLog2(uint64_t x)
{
    if (!x) raise(SIGFPE);
    uint64_t y;
    __asm__ ( "\tbsr %1, %0\n"
            : "=r"(y)
            : "r" (x)
        );
    return y;
}

#endif//MATH_ALGORITHMS_H
