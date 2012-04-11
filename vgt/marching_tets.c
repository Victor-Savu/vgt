#include <vgt/delaunay.h>
#include <vgt/mesh.h>
#include <vgt/scalar_field.h>
#include <vgt/scalar_field_cls.h>
#include <view/renderer.h>

#include <ads/array.h>

#include <math/vertex.h>

#include <math.h>

Mesh isoMarchingTets(   const ScalarField const restrict data,
                        Array restrict border,
                        Array restrict samples,
                        real isoValue
                        )
{
    // Manually create a delaunay tetrahedrization to encompass
    // the entire dataset by using a regular tetrahedron

    const real a = data->nx * data->dx;
    const real b = data->ny * data->dy;
    const real c = data->nz * data->dz;
    const real sqrt3 = 1.73205080756887729353;  // sqrt(3.0)
    const real ctac3 = 0.3535533905932737622;    // ctan(acos(1./3))
    Vertex bound[4] = {
        {0.5*a/sqrt3 + b/3, (a*sqrt3+2*b)*ctac3/6+c, 0.5 * a},
        {-c*ctac3/36., 0, -b/sqrt3 - c*ctac3/(12*sqrt3)},
        {-c*ctac3/36., 0, a + b/sqrt3 + c*ctac3/(12*sqrt3)},
        {a*sqrt3/2. + b + c*ctac3/18., 0, 0.5* a}
    };

    vScaleI(&bound[0], 1.001);
    vScaleI(&bound[1], 1.001);
    vScaleI(&bound[2], 1.001);
    vScaleI(&bound[3], 1.001);

    Delaunay del = delCreate(&bound);

    Renderer r = rCreate("Marching Tets");

    rDisplayDelaunay(r, del);

    rWait(r);

    rDestroy(r);

    delDestroy(del);


    stub;
    return 0;
}
