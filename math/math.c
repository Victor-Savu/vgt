#include <math/math.h>


inline uint64_t math_log2_uint64(uint64_t x)
{
    if (!x) raise(SIGFPE);
    uint64_t y;
    __asm__ ( "\tbsr %1, %0\n"
            : "=r"(y)
            : "r" (x)
        );
    return y;
}
