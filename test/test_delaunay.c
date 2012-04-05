#include <vgt/delaunay.h>
#include <view/renderer.h>

#include <vgt/delaunay_cls.h>
#include <vgt/tet_cls.h>
#include <math/predicates.h>
#include <math/vertex.h>
#include <ads/array.h>

int main()
{
    Vertex tetra[4] = {
        {0.0, 0.0, 0.0},
        {20.0, 0.0, 0.0},
        {0.0, 20.0, 0.0},
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
    /*
    int i;
    for (i=0; i < arrSize(d->t); i++) {
        Tet t = arrGet(d->t, i);
        char v0[30]; char v1[30]; char v2[30]; char v3[30];
        printf("Tet #%d: %s, %s, %s, %s : %s\n", i, vPrintStr(t->v[0], v0), vPrintStr(t->v[1], v1), vPrintStr(t->v[2], v2), vPrintStr(t->v[3], v3),
                (orient3d(*t->v[0], *t->v[1], *t->v[2], *t->v[3]) < 0)?("good"):("bad"));
    }

    Vertex p = {5.0, 5.0, 5.0};
    delInsert(d, &p);
    Vertex q = {5.0, 10.0, 5.0};
    delInsert(d, &q);

    for (i=0; i < arrSize(d->t); i++) {
        Tet t = arrGet(d->t, i);
        char v0[30]; char v1[30]; char v2[30]; char v3[30];
        printf("Tet #%d: %s, %s, %s, %s : %s\n", i, vPrintStr(t->v[0], v0), vPrintStr(t->v[1], v1), vPrintStr(t->v[2], v2), vPrintStr(t->v[3], v3),
                (orient3d(*t->v[0], *t->v[1], *t->v[2], *t->v[3]) < 0)?("good"):("bad"));
    }
*/
/*
    uint8_t i = 0;
    for (i=0; i<14; i++) {
        delInsert(d, &v[i]);
        Delaunay disp = delCopy(d);
        delDestroy(disp);
    }
*/


    Renderer r = rCreate("Hello world!");

    uint8_t i = 0;
    for (i=0; i<14; i++) {
        delInsert(d, &v[i]);
        rDisplayDelaunay(r, delCopy(d));
        fgetc(stdin);
    }

    rWait(r);

    rDestroy(r);

    delDestroy(d);


    return 0;
}
