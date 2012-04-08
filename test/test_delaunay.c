#include <vgt/delaunay.h>
#include <view/renderer.h>

#include <vgt/delaunay_cls.h>
#include <vgt/tet_cls.h>
#include <math/predicates.h>
#include <math/vertex.h>
#include <ads/array.h>
#include <math/algorithms.h>

void test0();
void test_speed(uint64_t n);

int main(int argc, char* argv[])
{
    uint64_t test = 10;

    if ((argc == 2) && (sscanf(argv[1], "%lu", &test) == 1)) {
        printf("#iterations for speed tests: %lu.\n", test);
    } else {
        test = 10;
        printf("#iterations for speed tests: %lu [default].\n", test);
    }
    test_speed(test);
    return 0;
}

void test_speed(uint64_t n)
{
    Vertex tetra[4] = {
        {-5.0, -5.0, -5.0},
        {0.0, 20.0, 0.0},
        {20.0, 0.0, 0.0},
        {0.0, 0.0, 20.0}
    };

    Vertex *v = oCreate(n * sizeof (Vertex));

    Delaunay del = delCreate(&tetra);

    uint64_t i = 0;

    struct timespec tick, tock;
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);

    for (i=0; i<n; i++) {
        v[i][0] = algoRandomDouble(2.0, 4.0);
        v[i][1] = algoRandomDouble(2.0, 4.0);
        v[i][2] = algoRandomDouble(2.0, 4.0);
    }
    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    printf("Generating: %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );

    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
    for (i=0; i<n; i++) delInsert(del, &v[i]);
    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    printf("Inserting : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );

    delDestroy(del);
    oDestroy(v);
}


void test0()
{
    Vertex tetra[4] = {
        {-10.0, -10.0, -10.0},
        {0.0, 20.0, 0.0},
        {20.0, 0.0, 0.0},
        {0.0, 0.0, 20.0}
    };

/*    Vertex v[4] = {
      {0.0, 0.0, 0.0},
      {10.0, 1.0, 1.0},
      {1.0, 10.0, 1.0},
      {1.0, 1.0, 10.0}
    };*/


    Vertex v[14] = {
      {5.0, 0.0, 0.0},
      {0.0, 5.0, 0.0},
      {0.0, 0.0, 5.0},
      {5.0, 5.0, 0.0},
      {0.0, 5.0, 5.0},
      {5.0, 0.0, 5.0},
      {2.0, 2.0, 0.0},
      {0.0, 2.0, 2.0},
      {2.0, 0.0, 2.0},
      {2.0, 2.0, 2.0},
      {0.0, 0.0, 0.0},
      {10.0, 0.0, 0.0},
      {0.0, 10.0, 0.0},
      {0.0, 0.0, 10.0}
    };

    Delaunay d = delCreate(&tetra);


    Renderer r = rCreate("Hello world!");

    rDisplayDelaunay(r, d);

    uint8_t i = 0;
    char enter = 13;
    for (i=0; i<14; i++) {
        rWaitKey(r, &enter);
      //  pthread_mutex_lock(&d->mutex);
        //printf("Parsel "); fflush(stdout);
        delInsert(d, &v[i]);
       // printf("tongue."); fflush(stdout);
      //  pthread_mutex_unlock(&d->mutex);
        printf("%s Delaunay tetrahedrization after inserting vertex #%d.\n", (delCheck(d))?("Correct"):("Incorrect"), i);
        fflush(stdout);
    }

    rWait(r);

    rDestroy(r);

    delDestroy(d);


}
