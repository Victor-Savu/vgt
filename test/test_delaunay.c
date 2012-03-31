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
    Delaunay d = delCreate(&tetra);
    
    Renderer r = rCreate("Hello world!");


    rDisplayDelaunay(r, d);

    rDestroy(r);

    return 0;
}
