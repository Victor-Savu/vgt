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

inline static
void find_bounding(uint64_t i, Obj o, Obj d)
{
    Vertex* v = o;
    Vertex (*bb)[2] = d;

    if ( (*v)[0] < (*bb)[0][0] ) (*bb)[0][0] = (*v)[0];
    if ( (*v)[1] < (*bb)[0][1] ) (*bb)[0][1] = (*v)[1];
    if ( (*v)[2] < (*bb)[0][2] ) (*bb)[0][2] = (*v)[2];

    if ( (*v)[0] > (*bb)[1][0] ) (*bb)[1][0] = (*v)[0];
    if ( (*v)[1] > (*bb)[1][1] ) (*bb)[1][1] = (*v)[1];
    if ( (*v)[2] > (*bb)[1][2] ) (*bb)[1][2] = (*v)[2];
}

inline static
void bounding_box(Array restrict border, Array restrict samples, Vertex* restrict pos, Vertex* restrict size)
{
    Vertex* aux = arrFront(border);
    Vertex bb[2] = {
        { (*aux)[0], (*aux)[1], (*aux)[2] },
        { (*aux)[0], (*aux)[1], (*aux)[2] }
    };

    arrForEach(border, find_bounding, &bb);
    arrForEach(samples, find_bounding, &bb);

    vCopy(&bb[0], pos);
    vSub(&bb[1], &bb[0], size);
}

Mesh isoMarchingTets(   const ScalarField const restrict data, Array restrict border, Array restrict samples, real isoValue)
{
    bool sufficient = true;
    if (( border && ((!samples && arrSize(border) < 4) || (arrSize(border) + arrSize(samples) < 4)) ) || (!samples || arrSize(samples)<4) ) {

        fprintf(stderr, "[x] Not enough sampling/border points.\n"); fflush(stderr);
        exit(EXIT_FAILURE);

    }


    // Manually create a delaunay tetrahedrization to encompass
    // the interesting dataset by using a regular tetrahedron

    Vertex pos;
    Vertex size;

    bounding_box(border, samples, &pos, &size);

    real a = size[0];
    real b = size[1];
    real c = size[2];

/*
    real a = data->nx * data->dx;
    real b = data->ny * data->dy;
    real c = data->nz * data->dz;
*/
    real min_cell_size_sqr = (data->dx<data->dy)?(data->dx):(data->dy);
    min_cell_size_sqr = (min_cell_size_sqr<data->dy)?(min_cell_size_sqr):(data->dy);
    min_cell_size_sqr *= min_cell_size_sqr;

    Vertex vert[8] = {
        { b, c, 0 },
        { 0, 0, 0 },
        { b, 0, 0 },
        { b, 0, a },
        { 0, c, 0 },
        { 0, c, a },
        { 0, 0, a },
        { b, c, a }
    };

    const real sqrt3 = sqrt(3);
    const real sqrt2 = sqrt(2);

    real s = 1.25;
    a*=s; b*=s, c*=s;
    Vertex bound[4] = {
        {0.5*a/sqrt3 + b/3          ,   a*sqrt2/sqrt3+2*b*sqrt2/3 + c   ,   0.5 * a                         },
        {-0.5*c/sqrt2               ,   0                               ,   -b/sqrt3 - 0.5*c*sqrt3/sqrt2    },
        {-0.5*c/sqrt2               ,   0                               ,   a+b/sqrt3+0.5*c*sqrt3/sqrt2     },
        {a*sqrt3/2.+b/3+c/sqrt2     ,   0                               ,   0.5* a                          }
    };
    a/=s; b/=s; c/=s;

    Vertex trans = {0.5*(s-1)*b, 0.5*(s-1)*c, 0.5*(s-1)*a};

    vSubI(&bound[0], &trans);
    vSubI(&bound[1], &trans);
    vSubI(&bound[2], &trans);
    vSubI(&bound[3], &trans);

    vSubI(&bound[0], &pos);
    vSubI(&bound[1], &pos);
    vSubI(&bound[2], &pos);
    vSubI(&bound[3], &pos);

    Delaunay del = delCreate(&bound);

    // tetPrint(arrFront(del->t), stdout); printf("\n");

    Renderer r = rCreate("Marching Tets");

    //Queue q = qCreate(sizeof(Tet));

    rDisplayDelaunay(r, del);


    // qPushArray(q, delInsert(del, &vert[0]));
    uint64_t i = 0;
    //   char key=13;
    for (i=0; i<8; i++) {
        //    rWaitKey(r, &key);
        arrDestroy(delInsert(del, &vert[i]));
        printf("%s Delaunay tetrahedrization after inserting vertex #%ld.\n", (delCheck(del))?("Correct"):("Incorrect"), i);
        fflush(stdout);
    }

    uint64_t skipped = 0;
    i = 0;
    while (skipped<arrSize(del->t)) {
        Tet t = arrGet(del->t, i);

        if (delIsBounding(del, t)) {
            skipped++;
        } else {
            Vertex g;
        /*    bool ins = 0;

            ins |= vNormSquared(vSub(t->v[B], t->v[A], &g)) > min_cell_size_sqr;
            ins |= vNormSquared(vSub(t->v[C], t->v[A], &g)) > min_cell_size_sqr;
            ins |= vNormSquared(vSub(t->v[D], t->v[A], &g)) > min_cell_size_sqr;
            ins |= vNormSquared(vSub(t->v[C], t->v[B], &g)) > min_cell_size_sqr;
            ins |= vNormSquared(vSub(t->v[D], t->v[B], &g)) > min_cell_size_sqr;
            ins |= vNormSquared(vSub(t->v[D], t->v[C], &g)) > min_cell_size_sqr;*/
               if (    vNormSquared(vSub(t->v[B], t->v[A], &g)) > min_cell_size_sqr ||
               vNormSquared(vSub(t->v[C], t->v[A], &g)) > min_cell_size_sqr ||
               vNormSquared(vSub(t->v[D], t->v[A], &g)) > min_cell_size_sqr ||
               vNormSquared(vSub(t->v[C], t->v[B], &g)) > min_cell_size_sqr ||
               vNormSquared(vSub(t->v[D], t->v[B], &g)) > min_cell_size_sqr ||
               vNormSquared(vSub(t->v[D], t->v[C], &g)) > min_cell_size_sqr) {
           // if (ins) {
               // skipped = 0;
                vAdd(t->v[A], t->v[B], &g);
                vAddI(&g, t->v[C]);
                vAddI(&g, t->v[D]);
                vScaleI(&g, 0.25);

                arrDestroy(delInsert(del, &g));
            //    printf("%s Delaunay tetrahedrization after inserting vertex #%ld.\n", (delCheck(del))?("Correct"):("Incorrect"), i);
            //    fflush(stdout);
            } else {
                skipped++;
            }

            //      rWaitKey(r, &key);
        }
        i++;
        if (i>arrSize(del->t)) i=0;
    }

    fprintf(stderr, "[i] Done!\n"); fflush(stderr);

    rWait(r);

    rDestroy(r);

    delDestroy(del);



    stub;
    return 0;
}
