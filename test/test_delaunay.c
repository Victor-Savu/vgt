#include <vgt/delaunay.h>
#include <view/renderer.h>

#include <vgt/delaunay_cls.h>
#include <vgt/tet_cls.h>
#include <math/predicates.h>
#include <math/vertex.h>
#include <ads/array.h>

void test0();
void test1();

int main()
{
    test0();
   // test1();
    return 0;
}

void test1()
{
    Vertex tetra[4] = {
        {-10.0, -10.0, -10.0},
        {0.0, 20.0, 0.0},
        {20.0, 0.0, 0.0},
        {0.0, 0.0, 20.0}
    };

    Vertex v[4] = {
      {0.0, 0.0, 0.0},
      {10.0, 1.0, 1.0},
      {1.0, 10.0, 1.0},
      {1.0, 1.0, 10.0}
    };

    Delaunay d = delCreate(&tetra);


    Renderer r = rCreate("Hello world!");

    rDisplayDelaunay(r, d);

    uint8_t i = 0;
    char enter = 13;
    for (i=0; i<4; i++) {
        rWaitKey(r, &enter);
       // pthread_mutex_lock(&d->mutex);
        delInsert(d, &v[i]);
       // pthread_mutex_unlock(&d->mutex);
        printf("%s Delaunay tetrahedrization after inserting vertex #%d.\n", (delCheck(d))?("Correct"):("Incorrect"), i);
        fflush(stdout);
    }

    rWait(r);

    rDestroy(r);

    delDestroy(d);

}

void test0()
{
    Vertex tetra[4] = {
        {-10.0, -10.0, -10.0},
        {0.0, 20.0, 0.0},
        {20.0, 0.0, 0.0},
        {0.0, 0.0, 20.0}
    };

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
