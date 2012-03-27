#include <vgt/delaunay.h>
#include <vgt/half_edge_cls.h>

int main()
{
    Vec tetra[4] = {
        {0.0, 0.0, 0.0},
        {10.0, 0.0, 0.0},
        {0.0, 10.0, 0.0},
        {0.0, 0.0, 10.0}
    };
    Delaunay d = delCreate(&tetra);
    unused(d);
    return 0;
}
