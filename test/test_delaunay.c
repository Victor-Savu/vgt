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
    test1();
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

    rDisplayDelaunay(r, delCopy(d));

    uint8_t i = 0;
    char enter = 13;
    for (i=0; i<4; i++) {
        rWaitKey(r, &enter);
        delInsert(d, &v[i]);
        rDisplayDelaunay(r, delCopy(d));
        //fgetc(stdin);
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

    rDisplayDelaunay(r, delCopy(d));

    uint8_t i = 0;
    char enter = 13;
    for (i=0; i<14; i++) {
        rWaitKey(r, &enter);
        delInsert(d, &v[i]);
        rDisplayDelaunay(r, delCopy(d));
        //fgetc(stdin);
    }

    rWait(r);

    rDestroy(r);

    delDestroy(d);

}
