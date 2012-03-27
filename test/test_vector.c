#include <ads/vector.h>
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
    Vector a = vecCreate(sizeof (int));

    int r = 10;

    struct timespec tick, tock;

    uint64_t t = test;
    while (test--) vecPush(a, &r);

    int* rez = 0;

    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);

    while (t--) rez = oCast(int*, vecGet(a, t));

    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    r = *rez;


    printf("%lf\n", ((double)(tock.tv_sec - tick.tv_sec) + (double)(tock.tv_nsec - tick.tv_nsec)/1e9) );

    vecDestroy(a);

    return 0;
}
