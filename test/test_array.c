#include <ads/array.h>
#include <stdio.h>
#include <math/obj.h>
#include <time.h>
#include <sys/time.h>

int main(int argc, char* argv[])
{
    if (argc != 2) return -1;

    uint64_t test;
    if (sscanf(argv[1], "%lu", &test) != 1) { printf("Failed to read #tests.\n"); return -1; }

    // create an array of integers
    Array a = arrCreate(sizeof (int));

    int r = 10;
    int push = 0, pop = 0;

    struct timespec tick, tock;

    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID,  &tick);

    while (test--) {
            arrPush(a, &r);
            push++;
    }

    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);

    printf("%d pushes and %d pops in %lf seconds.\n", push, pop,
            ((double)(tock.tv_sec - tick.tv_sec) + (double)(tock.tv_nsec - tick.tv_nsec)/1e9) );

    arrDestroy(a);

    return 0;
}
