#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <math/vec.h>

int main()
{
    uint i, j, k;
    ScalarField s = sfCreate(3, 3, 3, 1.0, 1.0, 1.0);
    for ( k=0; k<3; k++)
        for ( j=0; j<3; j++)
            for ( i=0; i<3; i++)
            {
                *sfAt(s, i, j, k) = 1.0;
            }

    *sfAt(s, 1, 1, 1) = 2.0;

    VectorField v = sfGradient(s);

    for ( k=0; k<3; k++) {
        for ( j=0; j<3; j++) {
            for ( i=0; i<3; i++)
            {
                vPrint(vfAt(v, i, j, k), stdout);
                printf(" ");
            }
            printf("\n");
        }
        printf("\n");
    }

    ScalarField l = vfDivergence(v);

    printf("\n");
    printf("Laplacian:\n");
    for ( k=0; k<3; k++) {
        for ( j=0; j<3; j++) {
            for ( i=0; i<3; i++)
            {
                printf("%f" , *sfAt(l, i, j, k));
                printf(" ");
            }
            printf("\n");
        }
        printf("\n");
    }



    sfDestroy(s);
    vfDestroy(v);

    return 0;
}

