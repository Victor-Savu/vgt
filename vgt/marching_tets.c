#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>
#include <vgt/mesh.h>
#include <vgt/scalar_field.h>
#include <vgt/scalar_field_cls.h>
#include <vgt/tet.h>

#include <view/renderer.h>

#include <ads/array.h>
#include <ads/queue.h>

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

    real a = data->nx * data->dx;
    real b = data->ny * data->dy;
    real c = data->nz * data->dz;

    Vertex vert[8] = {
        { 0, 0, 0 },
        { b, 0, 0 },
        { b, c, 0 },
        { b, 0, a },
        { 0, c, 0 },
        { 0, c, a },
        { 0, 0, a },
        { b, c, a }
    };
/*
    const real sqrt3 = sqrt(3);
    const real sqrt2 = sqrt(2);

    a*=1.01; b*= 1.01, c*=1.01;
    Vertex bound[4] = {
        {0.5*a/sqrt3 + b/3          ,   a*sqrt2/sqrt3+2*b*sqrt2/3 + c   ,   0.5 * a                         },
        {-0.5*c/sqrt2               ,   0                               ,   -b/sqrt3 - 0.5*c*sqrt3/sqrt2    },
        {-0.5*c/sqrt2               ,   0                               ,   a+b/sqrt3+0.5*c*sqrt3/sqrt2     },
        {a*sqrt3/2.+b/3+c/sqrt2     ,   0                               ,   0.5* a                          }
    };


    Vertex trans = {0.005*b, 0.005*c, 0.005*a};

    vSubI(&bound[0], &trans);
    vSubI(&bound[1], &trans);
    vSubI(&bound[2], &trans);
    vSubI(&bound[3], &trans);

    */

    Vertex bound[4] = {
            {    0,  10000,      0},
            {-3000,   -100, -10000},
            {-3000,   -100,  10000},
            { 6000,   -100,      0}
    };



    Delaunay del = delCreate(&bound);

   // tetPrint(arrFront(del->t), stdout); printf("\n");

    Renderer r = rCreate("Marching Tets");

    Queue q = qCreate(sizeof(Tet));

    rDisplayDelaunay(r, del);


   // qPushArray(q, delInsert(del, &vert[0]));
    uint8_t i = 0;
    char key=13;
    for (i=0; i<8; i++) {
        rWaitKey(r, &key);
        qPushArray(q, delInsert(del, &vert[i]));
        printf("%s Delaunay tetrahedrization after inserting vertex #%d.\n", (delCheck(del))?("Correct"):("Incorrect"), i);
    }
    rWaitKey(r, &key);
//  qPushArray(q, delInsert(del, &vert[2]));
//  qPushArray(q, delInsert(del, &vert[3]));
//  qPushArray(q, delInsert(del, &vert[4]));
//  qPushArray(q, delInsert(del, &vert[5]));
//  qPushArray(q, delInsert(del, &vert[6]));
//  qPushArray(q, delInsert(del, &vert[7]));

    while (!qIsEmpty(q))
    {
      //  Tet* t = qFront(q);
      //  tetPrint(*t, stdout); printf("\n");
        qPop(q);
    }


    rWait(r);

    rDestroy(r);

    delDestroy(del);


    stub;
    return 0;
}
