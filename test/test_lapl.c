
#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/vec.h>

#include <math.h>
#include <stdio.h>
#include <sys/time.h>

int main(int argc, char* argv[])
{
    uint64_t N;

    if (argc != 2) {
        printf("Input N.\n");
        return (-1);
    }

    sscanf(argv[1], "%u", &N);

    if (N > 500) N = 500;

    ScalarField s = sfCreate(N, N, N, 1.0, 1.0, 1.0);

    real* e = sfAt(s, 0, 0, 0);
    uint64_t i, j, k;
    for (k=0; k<N; k++)
        for (j=0; j<N; j++)
            for (i=0; i<N; i++) {
                *e = (i * 1.0 / N )*(i* 1.0 / N)*(j* 1.0 / N)*(j* 1.0 / N)*(k* 1.0 / N)*(k* 1.0 / N);
                e = sfRel(s, e, 1, 0, 0);
            }
    printf("Initialized scalar field.\n"); fflush(stdout);

    VectorField v = sfGradient(s);
    printf("Computed gradient.\n"); fflush(stdout);
    ScalarField l = vfDivergence(v);
    printf("Computed divergence.\n"); fflush(stdout);
    ScalarField L = sfLaplacian(s);
    printf("Computed laplacian.\n"); fflush(stdout);

    double delta = 0;

    real* el = sfAt(l, 0, 0, 0);
    real* eL = sfAt(L, 0, 0, 0);
    real m = 0.0;
    for (k=0; k<N; k++)
        for (j=0; j<N; j++)
            for (i=0; i<N; i++) {
                if (m < fabs(*el - *eL)) m = fabs(*el - *eL);
                delta += (*el - *eL)*(*el - *eL);
                el = sfRel(l, el, 1, 0, 0);
                eL = sfRel(L, eL, 1, 0, 0);
            }

    printf("delta sqr: %lf\n", delta);
    printf("max error: %lf\n", m);

    sfDestroy(s);
    vfDestroy(v);
    sfDestroy(l);
    sfDestroy(L);

    return 0;
}

