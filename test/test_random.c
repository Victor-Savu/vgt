#include <math/algorithms.h>

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <math/obj.h>

static inline
uint32_t choose(uint32_t n) {
    static bool init = false;
    if (!init) {srand(time(0)); init = true;}

    const uint64_t m = (1 << (algoLog2(n)+1)) -1;
    uint64_t rez;
    do {
        rez = rand() & m;
    } while (rez >= n);
    return rez;
}

void test_distribution(uint64_t n, uint32_t (*rand_algo)(uint32_t));
void test_speed(uint64_t n);

int main(int argc, char* argv[])
{

    uint64_t n = 10;

    if ((argc == 2) && (sscanf(argv[1], "%lu", &n) == 1)) {
        printf("#iterations: %lu.\n", n);
    } else {
        n = 10;
        printf("#iterations: %lu [default].\n", n);
    }

    printf("algoChooseU32:\n");
    test_distribution(n, algoChooseU32);
    printf("choose:\n");
    test_distribution(n, choose);
    test_speed(n);

    return 0;
}


void test_distribution(uint64_t n, uint32_t (*rand_algo)(uint32_t))
{
    uint64_t sz = (uint64_t) sqrt(n);
    printf("range: [0 .. %lu]\n", sz);
    uint64_t* arr = oCreate(sz * sizeof(uint64_t));

    uint64_t sum = 0;
    uint64_t rnd = 0;

    while (n--) {
        rnd = rand_algo(sz);
        sum ++;
        arr[rnd]++;
    }

    double mean = (double)sum / (double)sz;
    double stddev = 0;
    n = sz;

    while (sz--) {
//        printf("%lu ", arr[sz]);
        stddev += (mean - arr[sz]) * (mean - arr[sz]);
    }
    //printf("\n");

    stddev = sqrt(stddev / n);

    oDestroy(arr);

    printf("average: %6.6lf   deviation: %6.6lf\n", mean, stddev);
}

void test_speed(uint64_t n)
{
    uint64_t i = 0;
    struct timespec tick, tock;

    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
    for (i=0; i<n; i++) (void) algoChooseU32(n);
    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    printf("algoChoose: %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );

    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
    for (i=0; i<n; i++) (void)choose(n);
    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    printf("choose    : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );

}

