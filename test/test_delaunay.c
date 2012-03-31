#include <vgt/delaunay.h>
#include <view/renderer.h>

int main()
{
    Vec tetra[4] = {
        {0.0, 0.0, 0.0},
        {10.0, 0.0, 0.0},
        {0.0, 10.0, 0.0},
        {0.0, 0.0, 10.0}
    };

    Vec v[14] = {
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

    uint8_t i = 0;
    for (i=0; i<14; i++) delInsert(d, &v[i]);

   // delDestroy(d);

    Renderer r = rCreate("Hello world!");


    rDisplayDelaunay(r, d);

    rDestroy(r);

    rWait(r);

    return 0;
}
