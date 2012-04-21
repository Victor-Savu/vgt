
#include <stdio.h>
#include <stdlib.h>

#include <vgt/spectrum.h>

#include <view/renderer.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Please provide the path to the configuration file.\n");
        return -1;
    }

    int count = 1;
    while (count-- > 0) {
        call;
        Spectrum sp = specCreate(argv[1]);
        (void) printf("Created!\n"); fflush(stdout);
        specStats(sp);

        if (!sp) continue;

        Renderer r = rCreate("Test spectrum.");

        rDisplaySpectrum(r, sp);

        char key = 13;
        rWaitKey(r, &key);

        specRelax(sp);
        specStats(sp);

        (void) printf("Relaxed!\n"); fflush(stdout);

        rWait(r);
        rDestroy(r);

        specDestroy(sp);
        (void) printf("Success!\n"); fflush(stdout);
        sp = 0;

    }

    return 0;
}
