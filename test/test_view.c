#include <view/renderer.h>
#include <vgt/mesh.h>
#include <vgt/delaunay.h>

#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("please provide exactly one .off file as input.\n");
        return -1;
    }
    Renderer r = rCreate("Hello");

    Mesh m = mReadOff(0, argv[1]);
    rDisplayMesh(r, m);


    Vec tetra[4] = {
        {0.0, 0.0, 0.0},
        {260.0, 0.0, 0.0},
        {0.0, 260.0, 0.0},
        {0.0, 0.0, 260.0}
    };
    Delaunay d = delCreate(&tetra);

    rDisplayDelaunay(r, d);

/*
    m = mReadOff(0, argv[1]);
    rDisplayMesh(r, m);
    m = mReadOff(0, argv[1]);
    rDisplayMesh(r, m);
    m = mReadOff(0, argv[1]);
    rDisplayMesh(r, m);
*/
    rWait(r);

    return 0;
}
