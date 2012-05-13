
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>

#include <math/predicates.h>

#include <vgt/spectrum.h>

#include <view/renderer.h>

int main(int argc, char* argv[])
{
    call;
    exactinit();
    if (argc != 2) {
        printf("Please provide the path to the configuration file.\n");
        return -1;
    }

   // struct timespec tick, tock;
    Spectrum sp = specCreate(argv[1]);
    (void) fprintf(stderr, "Created!\n"); fflush(stderr);

    usage(sp);
    specStats(sp, stdout);
    printf("\n");

    Renderer r = rCreate("Test spectrum.");
    rDisplaySpectrum(r, sp);
    char key = 13;
    rWaitKey(r, &key);

    specSnap(sp);

/*
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
    clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
    fprintf(stderr, "Snapping  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
    fprintf(stderr, "\n");
    // specStats(sp, stdout);
    fprintf(stderr, "\n");
*/
    while (1) {
        /*
        //        rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specSimplify(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Simplifying  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //    specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //    rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRefine(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Refining  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //        rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specSimplify(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Simplifying  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //    specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //    rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRefine(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Refining  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //        rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specSimplify(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Simplifying  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //    specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //    rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRefine(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Refining  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        //  rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        specRelax(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Relaxing  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");
        // rWaitKey(r, &key);

        clock_gettime( CLOCK_PROCESS_CPUTIME_ID,  &tick);
        bool ret = specProject(sp);
        clock_gettime ( CLOCK_PROCESS_CPUTIME_ID, &tock);
        fprintf(stderr, "Projecting  : %10.6lf\n", (((double)tock.tv_sec - tick.tv_sec) + ((double)tock.tv_nsec - tick.tv_nsec)/1e9) );
        fprintf(stderr, "\n");
        //     specStats(sp, stdout);
        fprintf(stderr, "\n");

        if (!ret) break;
        */

            specProcessFringe(sp);
        if (!specProject(sp)) break;
    }

    (void) fprintf(stdout, "Success!\n"); fflush(stdout);

    rWait(r);
    rDestroy(r);

    specStats(sp, stdout);
    specDestroy(sp);
    sp = 0;


    return 0;
}
