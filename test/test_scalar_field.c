
#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/types.h>


void testConstructorsAndDestructors();
void testGradient();
void testLaplacian(uint64_t x, uint64_t y, uint64_t z, const char* fname);
void testReadWrite(uint64_t x, uint64_t y, uint64_t z, const char* fname);
void testSuperSampling(uint64_t x, uint64_t y, uint64_t z, const char* fname);

int main(int argc, char* argv[])
{
    testConstructorsAndDestructors();
    testGradient();
    if (argc == 5) {
        uint64_t x, y,z;
        if (    sscanf(argv[2], "%ld", &x) != 1 ||
                sscanf(argv[3], "%ld", &y) != 1 ||
                sscanf(argv[4], "%ld", &z) != 1) return 0;
       // testReadWrite(x, y, z, argv[1]);
        testSuperSampling(x, y, z, argv[1]);
      //  testLaplacian(x, y, z, argv[1]);
    }
    return 0;
}

void testConstructorsAndDestructors()
{
    call;
    ScalarField v = sfCreate(3, 3, 3, 0.5, 0.5, 0.5);
    sfDestroy(v);
}

void testGradient()
{
    call;
    ScalarField s = sfCreate(30, 30, 30, 1, 1.5, 1);
    VectorField v = sfGradient(s);
    vfDestroy(v);
    sfDestroy(s);
}

void testLaplacian(uint64_t x, uint64_t y, uint64_t z, const char* fname)
{
    call;
    ScalarField s = sfCreate(x, y, z, 1, 1, 1);
    sfReadRaw(s, fname);

    ScalarField l = sfLaplacian(s);

    sfWriteRaw(l, "laplacian.raw");

    sfDestroy(s);
    sfDestroy(l);
}

void testReadWrite(uint64_t x, uint64_t y, uint64_t z, const char* fname)
{
    call;
    ScalarField s = sfCreate(x, y, z, 1, 1, 1);
    sfReadRaw(s, fname);
    sfWriteRaw(s, "output.raw");
    sfDestroy(s);
}



void testSuperSampling(uint64_t x, uint64_t y, uint64_t z, const char* fname)
{
    call;
    ScalarField s = sfCreate(x, y, z, 1, 1, 1);
    ScalarField ss = sfCreate(x*2, y*2, z*2, 0.5, 0.5, 0.5);
    sfReadRaw(s, fname);
    uint64_t i, j, k;

    for (i=0; i < (x-1)*2; i++)
        for (j=0; j < (y-1)*2; j++)
            for (k=0; k < (z-1)*2; k++)
                *sfAt(ss, i, j, k) = sfValue(s, (real)i / 2.0, (real)j/2.0, (real)k/2.0);

    ScalarField l = sfLaplacian(ss);

    sfWriteRaw(l, "laplacian.raw");

    sfWriteRaw(ss, "supersample.raw");

    sfDestroy(s);
    sfDestroy(ss);
    sfDestroy(l);
}




