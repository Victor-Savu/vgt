#include <ads/array.h>

#include <stdio.h>
#include <math/obj.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    if (argc != 2) return -1;

    uint64_t test;
    if (sscanf(argv[1], "%lu", &test) != 1) { printf("Failed to read #tests.\n"); return -1; }

    // create an array of integers
    Array a = arrCreate(sizeof (int));
    srand(time(0));


    int r = 10;
    int push = 0, pop = 0;

/*
    printStatus(a);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPush(a, &r);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    arrPop(a);
    printStatus(a);
    arrPush(a, &r);
    printStatus(a);
    arrPush(a, &r);
    printStatus(a);
    arrPush(a, &r);
*/
    clock_t tick = clock();

    while (test--) {
        //printf("#%u:\n", i); fflush(stdout);
      //  if ((rand() & 1) && (arrSize(a) > 0)){
      //      arrPop(a);
      //      pop++;
      //  } else {
      //      r = rand();
            arrPush(a, &r);
            push++;
      //  }
    }

    clock_t tock = clock();
    printf("%d pushes and %d pops in %lf seconds.\n", push, pop, (double)(tock - tick) / CLOCKS_PER_SEC);

    arrDestroy(a);
}
