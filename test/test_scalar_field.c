
#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/types.h>


void testConstructorsAndDestructors();
void testGradient();
void testLaplacian();


int main()
{
    testConstructorsAndDestructors();
    testGradient();
    testLaplacian();
    return 0;
}

void testConstructorsAndDestructors()
{
    ScalarField v = sfCreate(3, 3, 3, 0.5, 0.5, 0.5);
    sfDestroy(v);
}

void testGradient()
{
    ScalarField s = sfCreate(30, 30, 30, 1, 1.5, 1);
    VectorField v = sfGradient(s);
    vfDestroy(v);
    sfDestroy(s);
}

void testLaplacian()
{
    ScalarField s = sfCreate(30, 30, 30, 1, 1.5, 1);
    ScalarField l = sfLaplacian(s);
    sfDestroy(l);
    sfDestroy(s);
}
