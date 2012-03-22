#include <vgt/mesh.h>
#include <stdio.h>

void test_create_destroy(const char* restrict fname);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("please provide exactly one .off file as input.\n");
        return -1;
    }

    test_create_destroy(argv[1]);
    return 0;
}

void test_create_destroy(const char* restrict fname)
{
    Mesh m = mReadOff(0, fname);
    mDestroy(m);
}
