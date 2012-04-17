#include <vgt/iso.h>

#include <vgt/scalar_field.h>
#include <vgt/mesh.h>

#include <ads/array.h>

#include <math/vertex.h>
#include <math/predicates.h>

#include <stdlib.h>


void testMarchingTets(uint64_t x, uint64_t y, uint64_t z, real dx, real dy, real dz, const char* fname, real iso);

int main(int argc, char* argv[])
{
    exactinit();
    if (argc == 9) {
        uint64_t x, y, z;
        double dx, dy, dz;
        double i;
        if (    sscanf(argv[2], "%ld", &x) != 1 ||
                sscanf(argv[3], "%ld", &y) != 1 ||
                sscanf(argv[4], "%ld", &z) != 1 ||
                sscanf(argv[5], "%lf", &dx) != 1 ||
                sscanf(argv[6], "%lf", &dy) != 1 ||
                sscanf(argv[7], "%lf", &dz) != 1 ||
                sscanf(argv[8], "%lf", &i) != 1) return 0;

        testMarchingTets(x, y, z, dx, dy, dz, argv[1], i);
    }
    return 0;

}

void testMarchingTets(  uint64_t x,
        uint64_t y,
        uint64_t z,
        real dx,
        real dy,
        real dz,
        const char* fname,
        real iso)
{
    ScalarField sf = sfCreate(x, y, z, dx, dy, dz);
    if (!sfReadRaw(sf, fname)) exit(EXIT_FAILURE);

    Vertex vertices[8] = {
        {0, 0, 0},
        {4, 0, 0},
        {0, 1.8, 0},
        {2.0, 2.5, 0},
        {0, 0, 5.0},
        {1.0, 0, 3.0},
        {0, 3.0, 2.0},
        {10.0, 10.0, 10.0}
    };
    Vertex pos = {0, 0, 0};

    uint64_t i = 0;
    for (i=0; i<8; i++) vAddI(&vertices[i], &pos);

    Array bounding_box = arrCreate(sizeof(Vertex), 1);
    for (i=0; i<8; i++) arrPush(bounding_box, &vertices[i]);

    Mesh m = isoMarchingTets(sf, bounding_box, (void*)0, iso);

    mDestroy(m);
    sfDestroy(sf);
}
