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

struct MTData {
    ScalarField sf;
    Mesh m;
    uint64_t ntriangles;
    real isoValue;
};

    inline static
void count_triangles(uint64_t i, Obj o, Obj d)
{
    Tet t = oCast(Tet, o);
    ScalarField sf = oCast(struct MTData*, d)->sf;
    real iso = oCast(struct MTData*, d)->isoValue / 255.0;
    uint64_t* tri = &(oCast(struct MTData*, d)->ntriangles);
    uint8_t cnt = 0;
    if (sfValue(sf, (*t->v[A])[0], (*t->v[A])[1], (*t->v[A])[2]) > iso) {
        cnt++;
    }
    if (sfValue(sf, (*t->v[B])[0], (*t->v[B])[1], (*t->v[B])[2]) > iso) {
        cnt++;
    }
    if (sfValue(sf, (*t->v[C])[0], (*t->v[C])[1], (*t->v[C])[2]) > iso) {
        cnt++;
    }
    if (sfValue(sf, (*t->v[D])[0], (*t->v[D])[1], (*t->v[D])[2]) > iso) {
        cnt++;
    }

    if (cnt & 1) {
        (*tri)+=1;
    } else if (cnt == 2) {
        (*tri)+=2;
    }
}

    inline static
void ins_sample(uint64_t i, Obj o, Obj d)
{
    arrDestroy(delInsert(d, o));
//    fprintf(stderr, "Inserted sample #%lu: ", i); vPrint(o, stderr); fprintf(stderr, "\n");
}

    inline static
void bounding_box(Array restrict crit, Array restrict samples, Vertex* restrict pos, Vertex* restrict size)
{
    Vertex* aux = arrFront(crit);
    Vertex bb[2] = {
        { (*aux)[0], (*aux)[1], (*aux)[2] },
        { (*aux)[0], (*aux)[1], (*aux)[2] }
    };

    arrForEach(crit, find_bounding, &bb);
    arrForEach(samples, find_bounding, &bb);

    vCopy(&bb[0], pos);
    vSub(&bb[1], &bb[0], size);
}

struct proximity_kit {
    bool in_proximity;
    real dist_squared;
    Tet t;
};

inline static
void check_proximity(uint64_t i, Obj o, Obj d)
{
    struct proximity_kit* kit = d;
    TetVertex v;
    for (v = A; v <= D; ++v) {
        Vec3 displ;
        kit->in_proximity |= (vNormSquared((const Vec3*) vSub(o, kit->t->v[v], &displ)) < kit->dist_squared);
    }
}

Delaunay isoSample(const ScalarField const restrict data, Array restrict crit, Array restrict samples, real isoValue, real radius)
{
    call;
    usage(crit);
    usage(samples);
    if ( arrSize(crit) + arrSize(samples) < 4) {
        fprintf(stderr, "#crit: %lu\n#samples: %lu\n", arrSize(crit), arrSize(samples));
        usage( arrSize(crit) + arrSize(samples) >= 4);
    }

    // Manually create a delaunay tetrahedrization to encompass
    // the interesting dataset by using a regular tetrahedron

    Vertex pos;
    Vertex size;

    bounding_box(crit, samples, &pos, &size);

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
/*
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
*/
    const real sqrt3 = sqrt(3);
    const real sqrt2 = sqrt(2);

    real s = 3;
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

    vAddI(&bound[0], (const Vec3*)&pos);
    vAddI(&bound[1], (const Vec3*)&pos);
    vAddI(&bound[2], (const Vec3*)&pos);
    vAddI(&bound[3], (const Vec3*)&pos);

    Delaunay del = delCreate(&bound);

    arrForEach(crit, ins_sample, del);
    arrForEach(samples, ins_sample, del);


  uint64_t i = 0;
/*
//    char key=13;
    Vertex aux;
    for (i=0; i<8; i++) {
//        rWaitKey(r, &key);
        vAdd(&vert[i], &pos, &aux);

        arrDestroy(delInsert(del, &aux));
//      printf("%s Delaunay tetrahedrization after inserting vertex #%ld.\n", (delCheck(del))?("Correct"):("Incorrect"), i); fflush(stdout);
    }
*/

    uint64_t skipped = 0;
    i = 0;
    while (skipped<arrSize(del->t)) {
        Tet t = arrGet(del->t, i);

        // check if at least one of the tet's vertices is in the proximity of a critical point
        struct proximity_kit p_kit = { .in_proximity = false, .dist_squared = radius*radius*4, .t = t };
        arrForEach(crit, check_proximity, &p_kit);
        //p_kit.in_proximity = true;

        if (delIsBounding(del, t) || !p_kit.in_proximity) {
            skipped++;
        } else {
            Vertex g;
            if (    vNormSquared((const Vec3*)vSub(t->v[B], t->v[A], &g)) > min_cell_size_sqr ||
                    vNormSquared((const Vec3*)vSub(t->v[C], t->v[A], &g)) > min_cell_size_sqr ||
                    vNormSquared((const Vec3*)vSub(t->v[D], t->v[A], &g)) > min_cell_size_sqr ||
                    vNormSquared((const Vec3*)vSub(t->v[C], t->v[B], &g)) > min_cell_size_sqr ||
                    vNormSquared((const Vec3*)vSub(t->v[D], t->v[B], &g)) > min_cell_size_sqr ||
                    vNormSquared((const Vec3*)vSub(t->v[D], t->v[C], &g)) > min_cell_size_sqr) {

                skipped = 0;

                vAdd(t->v[A], t->v[B], &g);
                vAddI(&g, (const Vec3*)t->v[C]);
                vAddI(&g, (const Vec3*)t->v[D]);
                vScaleI(&g, 0.25);

                arrDestroy(delInsert(del, &g));
//                rWaitKey(r, &key);
            } else {
                skipped++;
            }

        }
        i++;
        if (i >= arrSize(del->t)) i=0;
    }

    printf("%s Delaunay tetrahedrization.\n", (delCheck(del))?("Correct"):("Incorrect")); fflush(stdout);
    fprintf(stderr, "[i] Done!\n"); fflush(stderr);

//  Renderer r = rCreate("Marching Tets");
//  rDisplayDelaunay(r, del);
//  char key=13;
//  rWaitKey(r, &key);

    delDropBoundary(del);

//  rWait(r);
//  rDestroy(r);

    struct MTData dat = {data, 0, 0, isoValue};

    arrForEach(del->t, count_triangles, &dat);

    fprintf(stdout, "Triangles: %lu\n", dat.ntriangles);

 //   delDestroy(del);

    stub;
    return del;
}
