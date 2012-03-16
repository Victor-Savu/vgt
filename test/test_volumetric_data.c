
#include <stdio.h>
#include <stdlib.h>

#include <vgt/vec.h>
#include <vgt/volumetric_data.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Please provide the path to the configuration file.\n");
        return -1;
    }

    int count = 10;
    while (count-- > 0) {
        VolumetricData f = vdCreate(argv[1]);

        if (!f) continue;

        (void) printf("Success!\n");

        vdDestroy(f);
        f = 0;
    }



    return 0;

}
