
#include <vgt/vector_field.h>
#include <vgt/scalar_field.h>
#include <vgt/types.h>

void testConstructorsAndDestructors();
void testDivergence();


int main()
{
    testConstructorsAndDestructors();
    testDivergence();
    return 0;
}

void testConstructorsAndDestructors()
{
    VectorField v = vfCreate(300, 300, 300, 0.5, 0.5, 0.5);
    vfDestroy(v);
}

void testDivergence()
{
    VectorField v = vfCreate(100, 100, 100, 1, 1, 1);
    ScalarField s = vfDivergence(v);
    sfDestroy(s);
    vfDestroy(v);
}
