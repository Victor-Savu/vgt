
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <vgt/spectrum.h>

#include <view/renderer.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Please provide the path to the configuration file.\n");
        return -1;
    }

    struct timespec tick, tock;

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

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        printf("Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        specStats(sp);

        rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specSimplify(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        printf("Simplifying  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        specStats(sp);

        rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        printf("Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        specStats(sp);

        //        (void) printf("Relaxed!\n"); fflush(stdout);

        rWait(r);
        rDestroy(r);

        specDestroy(sp);
        (void) printf("Success!\n"); fflush(stdout);
        sp = 0;

    }

    return 0;
}
