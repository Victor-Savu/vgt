#include <ads/array.h>
#include <stdio.h>
#include <math/obj.h>
#include <time.h>
#include <sys/time.h>


uint32_t choose(uint32_t pool) {
    static bool randinit = false;
    if (!randinit) { srand(10); randinit = true; }
    uint32_t i;
    double x = (double)rand() / (double)RAND_MAX;
    for (i=0; (double)(i+1) / (double)pool + 0.000000001 < x; i++);
    return i;
}

void speed_test(uint64_t test);
void sanity_check(uint64_t test);
void random_check(uint64_t test);
void ref_check(uint64_t n);

int main(int argc, char* argv[])
{
    uint64_t test = 10;

    if ((argc == 2) && (sscanf(argv[1], "%lu", &test) == 1)) {
        printf("#iterations for speed tests: %lu.\n", test);
        ignore choose(10);
        srand(test);
    } else {
        test = 10;
        printf("#iterations for speed tests: %lu [default].\n", test);
    }

    speed_test(test);
    //sanity_check(test);
    //random_check(test);
    ref_check(test);

    return 0;
}

inline static void op(uint64_t i, Obj o, Obj unused) {
    ignore(i);
    ignore(unused);
    int* restrict rez = oCast(int*, o);
    ignore(rez);
}

void speed_test(uint64_t test)
{
    call;
    // create an array of integers
    Array arr = arrCreate(sizeof (int), 10);

    int r = 10;

    struct timespec tick, tock;

    uint64_t t = test;
    while (test--) arrPush(arr, &r);

    int* rez = 0;

    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);

    while (t--) rez = oCast(int*, arrGet(arr, t));

    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    r = *rez;


    printf("Getting  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );


    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);

    arrForEach(arr, op, 0);

    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    r = *rez;


    printf("Iterating: %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );


    arrDestroy(arr);

}


void sanity_check(uint64_t test)
{
    call;

    // create an array of integers
    Array arr = arrCreate(sizeof (int), 4);

    int x;
    while (test--) {
        switch(choose(3)) {
        case 0: // insert
            printf("Inserting "); fflush(stdout);
            x = choose(10);
            arrPush(arr, &x);
            printf("%d: ", x);
            arrPrint(arr, stdout, oIntPrint);
            printf("\n"); fflush(stdout);
            break;
        case 1:
            if (!arrSize(arr)) {
                printf("Arr empty. Not getting anything.\n");
                break;
            }
            printf("Get"); fflush(stdout);
            x = choose(arrSize(arr));
            printf("[%d]=%d: ", x, *oCast(int*, arrGet(arr, x))); fflush(stdout);
            arrPrint(arr, stdout, oIntPrint);
            printf("\n"); fflush(stdout);
            break;
        case 2:
            if (!arrSize(arr)) {
                printf("Arr empty. Not popping anything.\n");
                break;
            }
            printf("Popping "); fflush(stdout);
            printf("%d: ", *oCast(int*, arrBack(arr))); fflush(stdout);
            arrPop(arr);
            arrPrint(arr, stdout, oIntPrint);
            printf("\n"); fflush(stdout);
            break;
        default:
            check(0);
            break;
        }
    }

    arrDestroy(arr);

}


void random_check(uint64_t n)
{
    call;
    uint64_t i;


    Array arr = arrCreate(sizeof(uint64_t), 4);

    for (i=0; i<n; i++) arrPush(arr, &i);

    for (i=0; i<n; i++) {
        arrRandomSwap(arr, 0);
        printf("%lu ", *oCast(uint64_t*, arrBack(arr)));
        arrPop(arr);
    }

    printf("\n");

    arrDestroy(arr);
}

void ref_check(uint64_t n)
{
    call;
    short i;

    Array arr = arrCreate(sizeof(short), 4);

    for (i=0; i<n; i++) arrPush(arr, &i);

    arrPrint(arr, stdout, oShortPrint);
    printf("\n");

    short** refs = arrRefs(arr);
    for (i=0; i<n; i++) refs[i][0]++;
    oDestroy(refs);

    arrPrint(arr, stdout, oShortPrint);
    printf("\n");

    arrDestroy(arr);
}
