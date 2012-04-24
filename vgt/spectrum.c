
#include <vgt/spectrum.h>
#include <vgt/spectrum_cls.h>

#include <math/obj.h>
#include <math/vertex.h>
#include <math/algorithms.h>

#include <ads/array.h>
#include <ads/vector.h>

#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/volumetric_data.h>
#include <vgt/volumetric_data_cls.h>
#include <vgt/scalar_field_cls.h>

#include <GL/gl.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

typedef struct Sample* Sample;
typedef struct Thread* Thread;
typedef struct Triangloid* Triangloid;
typedef struct HalfEdge* HalfEdge;

// a 3D sample point of the scalar field
struct Sample {
    // position of the sample point
    Vertex p;
    // normal at the sample point
    Normal n;
    // isovalue at the sample point
    float iso;
};

// a string of sample points of increaasing isovalues
struct Thread {
    // the sample points of the triangloid in increasing isovalue
    Array samples;
    // the thread id
    uint32_t id;
    // the thread it represents (possibly not itself, but the one it collapsed with)
    Thread t;
    uint32_t depth;
    /*
       the isovalue where the thread got collapsed into another
       this value should be disregarded if t points to this thread itself
     */
    real iso; 

    // the force exerted on the last sample of the thread by its incident triangles
    Vec3 force;
    bool active;
};

// a 3D shape bordered by 3 threads with 2 end-points defined by their isovalues
struct Triangloid {
    // isovalue range for which this triangloid is defined (iso[0] <= iso[1])
    float iso[2];
    // thread IDs defining the vertices of each slice.
    uint32_t t_ids[3];
};

// A half-edge data structure for representing
// the fringe of the spectrum
struct HalfEdge {
    Thread t;
    HalfEdge n;
    HalfEdge o;
    real iso; // the initial isovalue for which it was spawn
    Obj att; // an attached object
};

inline static
Vector heOneCircle(HalfEdge e)
{
    if (!e) return 0;
    const HalfEdge const flag = e;

    Vector v = vecCreate(sizeof (HalfEdge));
    do {
        vecPush(v, &e->n);
        e = e->n->n->o;
        usage(vecSize(v) < 100);
    } while (e && e!=flag);

    if (e==0) {
        e = flag->o;
        while (e) {
            e = e->n;
            vecPush(v, &e->n);
            usage(vecSize(v) < 100);
            e = e->o;
        }
    }

    return v;
}

inline static
int cmp_edges(const void* a, const void* b)
{
    Thread ta = (*oCast(HalfEdge*, a))->t; while (ta->t != ta) ta = ta->t;
    Thread tb = (*oCast(HalfEdge*, b))->t; while (tb->t != tb) tb = tb->t;

    if (ta < tb) return -1; else if (ta == tb) return 0; else return 1;
}

inline static
char* strip(char* line)
{
    if (!line) return 0;
    char* c = line;
    while (*c !=  '\n' && *c && *c != '#') c++;
    *c = '\0';
    while ( (c != line) && ((*c==' ') || (*c=='\t') || (*c=='\r')) ) *(c--) = '\0';
    return line;
}

inline static
int left_cmp(const void* a_, const void* b_) {
    const HalfEdge a = *oCast(const HalfEdge*, a_);
    const HalfEdge b = *oCast(const HalfEdge*, b_);

    if (a->t->id < b->t->id) return -1;
    else if (a->t->id > b->t->id) return  1;
    else if (a->n->t->id < b->n->t->id) return -1;
    else if (a->n->t->id > b->n->t->id) return  1;
    else return 0;
}

inline static
int right_cmp(const void* a_, const void* b_) {

    const HalfEdge a = *oCast(const HalfEdge*, a_);
    const HalfEdge b = *oCast(const HalfEdge*, b_);

    if (a->n->t->id < b->n->t->id) return -1;
    else if (a->n->t->id > b->n->t->id) return  1;
    else if (a->t->id < b->t->id) return -1;
    else if (a->t->id > b->t->id) return  1;
    else return 0;
}

inline static
int cross_cmp(const HalfEdge a, const HalfEdge b) {
    if (a->n->t->id < b->t->id) return -1;
    else if (a->n->t->id > b->t->id) return  1;
    else if (a->t->id < b->n->t->id) return -1;
    else if (a->t->id > b->n->t->id) return  1;
    else return 0;
}

inline static
void remove_inactive_threads(uint64_t i, Obj o, Obj d)
{
    static int x = 0;
    unused(i);

    do {
        Thread* t = o;
        if (!(*t)->active) {
            *t = *oCast(Thread*, arrBack(d));
            arrPop(d);
            x++;
        } else break;
    } while (i < arrSize(d));
}

Spectrum specCreate(const char* restrict conf)
{
    call;
    VolumetricData v = oCreate(sizeof (struct VolumetricData));

    usage(conf);

    FILE* fin = fopen(conf, "r");

    conjecture(fin, "Error opening file.");

    // temporary data
    char raw_file_name[1024];
    char off_file_name[1024];
    float initial_isovalue = 0.0;
    char line[1024];

    // read the .raw file name
    do {
        conjecture(fgets(line, 1024, fin), "Error reading the raw file name from configuration file.");
        strip(line);
    } while (sscanf(line, "%s", raw_file_name) != 1);

    unsigned int nx, ny, nz;

    // read the size of the data in the raw file
    do {
        conjecture(fgets(line, 1024, fin), "Error reading the size of the raw file data from configuration file.");
        strip(line);
    } while (sscanf(line, "%u %u %u", &nx, &ny, &nz) != 3);

    float sx, sy, sz;

    // read the voxel size in each dimension
    do {
        conjecture(fgets(line, 1024, fin), "Error reading the voxel size from configuration file.");
        strip(line);
    } while (sscanf(line, "%f %f %f", &sx, &sy, &sz) != 3);

    v->scal = sfCreate(nx, ny, nz, sx, sy, sz);
    sfReadRaw(v->scal, raw_file_name);

    v->grad = sfGradient(v->scal);
    //v->lapl = vfDivergence(v->grad);
    v->lapl = sfLaplacian(v->scal);

    // read the name of the .off file with the initial mesh and the initial isovalue for which it was extracted
    do {
        conjecture(fgets(line, 1024, fin), "Error reading the off file name and initial isovalue from configuration file.");
        strip(line);
    } while (sscanf(line, "%s %f", off_file_name, &initial_isovalue) != 2);
    // scale down the initial isovalue
    initial_isovalue /= 255.0f;

    // read the number of critical points
    do {
        conjecture(fgets(line, 1024, fin), "Error reading the number of critical points from configuration fil.");
        strip(line);
    } while (sscanf(line, "%lu", &v->topology.size) != 1);

    CriticalPoint cp = v->topology.criticalities = malloc(v->topology.size * sizeof (struct CriticalPoint));

    uint64_t cp_read = 0;
    char crit_type[10];
    float isovalue, x, y, z;

    // for the rest of the file, read each criticality
    while (fgets(line, 1024, fin) && (cp_read < v->topology.size)) {
        strip(line);
        if (sscanf(line, "%f %s %f %f %f", &isovalue, crit_type, &x, &y, &z)) {
            cp->isovalue = isovalue / 255.0f;
            cp->pos[0] = x; cp->pos[1] = y; cp->pos[2] = z;
            if (!strncmp(crit_type, "min", 10)) cp->type = MINIMUM;
            else if (!strncmp(crit_type, "max", 10)) cp->type = MAXIMUM;
            else if (!strncmp(crit_type, "saddle", 10)) cp->type = SADDLE;
            else continue;
            cp_read++;
            cp++;
        }
    }

    // read the off file, initialize the threads and construct the fringe
    conjecture(freopen(off_file_name, "r", fin), "Failed to open the off file.");

    Array thr = arrCreate(sizeof (struct Thread), 2);
    Array tri = arrCreate(sizeof (struct Triangloid), 1);
    Array fringe = arrCreate(sizeof (struct HalfEdge), 2);
    Array active_thr = arrCreate(sizeof (Thread), 2);

    // read header
    unsigned int aux, vert, faces;
    conjecture(fgets(line, sizeof(line), fin), "Error reading from off file."); // OFF
    conjecture(fgets(line, sizeof(line), fin), "Error reading from off file."); // #vertices #faces #<unused>
   // fputs(line, stdout);
    conjecture(sscanf(line, "%u %u %u", &vert, &faces, &aux) == 3, "Failed to read #vert #faces #<unused> from off file.");
  //  printf("%u vertices, %u faces\n", vert, faces);

    // read vertices
    uint64_t i = 0;
    struct Sample smpl = { .p = VERT_0, .n = VERT_0, .iso = initial_isovalue };
    for (i = 0; i < vert; i++) {
        conjecture(fgets(line, sizeof(line), fin), "Error reading from off file.");
        conjecture(sscanf(line, "%f %f %f\n", &x, &y, &z) == 3, "failed to read vertex from off file.");
        //x+= 0.2; y+=0.2; z+=0.2;

        while (vIsZero(vfValue(v->grad, &smpl.n, x, y, z))) {
            x += algoRandomDouble(0.0001, 0.001);
            y += algoRandomDouble(0.0001, 0.001);
            z += algoRandomDouble(0.0001, 0.001);
        }

        vSet(&smpl.p, x+0.2, y+0.2, z+0.2);

        struct Thread tmp_thr = {
            .samples = arrCreate(sizeof (struct Sample), 1),
            .id = i,
            .t = 0,
            .depth = 0,
            .iso = initial_isovalue,
            .force = VERT_0,
            .active = false
        };
        ignore arrPush(tmp_thr.samples, &smpl);
        Thread t = arrPush(thr, &tmp_thr);
        t->t = t; // for now, the thread represents itself
        ignore arrPush(active_thr, &t);
    }

    // read faces
    unsigned int a, b, c, cnt;
//    Normal p, q, norm;

    for (i = 0; i < faces; i++) {
        conjecture(fgets(line, sizeof(line), fin), "Error reading from off file.");
        conjecture(sscanf(line, "%u %u %u %u\n", &cnt, &a, &b, &c) == 4, "Failed to read face from off file.");

        usage(a < vert);
        usage(b < vert);
        usage(c < vert);

        // create the half-edges
        struct HalfEdge edge_a = { .t = arrGet(thr, a), .n = 0, .o = 0, .iso = initial_isovalue };
        struct HalfEdge edge_b = { .t = arrGet(thr, b), .n = 0, .o = 0, .iso = initial_isovalue };
        struct HalfEdge edge_c = { .t = arrGet(thr, c), .n = 0, .o = 0, .iso = initial_isovalue };

        HalfEdge ea = arrPush(fringe, &edge_a);
        HalfEdge eb = arrPush(fringe, &edge_b);
        HalfEdge ec = arrPush(fringe, &edge_c);

        ea->n = eb; eb->n = ec; ec->n = ea;

        ea->t->active = true;
        eb->t->active = true;
        ec->t->active = true;

/*
        // create the triangloid
        struct Triangloid t = {
            .iso = {initial_isovalue, initial_isovalue},
                    .t_ids = {ea->t->id, eb->t->id, ec->t->id}
        };
        arrPush(tri, &t);

        Sample sa = oCast(Sample, arrBack(ea->t->samples));
        Sample sb = oCast(Sample, arrBack(eb->t->samples));
        Sample sc = oCast(Sample, arrBack(ec->t->samples));

        sa->iso = initial_isovalue;
        sb->iso = initial_isovalue;
        sc->iso = initial_isovalue;

        ignore vCross( vSub(&sb->p, &sa->p, &p),
                       vSub(&sc->p, &sa->p, &q),
                       &norm);
        vAddI(&sa->n, &norm);
        vAddI(&sb->n, &norm);
        vAddI(&sc->n, &norm);*/
    }

    // eliminate disconnected vertices
    arrForEach(active_thr, remove_inactive_threads, active_thr);

    // find opposing edges
    uint64_t n_edges = arrSize(fringe);
    HalfEdge* left  = arrRefs(fringe);
    HalfEdge* right = oCopy(left, n_edges * sizeof(HalfEdge));

    qsort(left, n_edges, sizeof (HalfEdge), left_cmp);
    qsort(right, n_edges, sizeof (HalfEdge), right_cmp);

    HalfEdge* l = left;
    HalfEdge* r = right;

    HalfEdge* end_l = left + n_edges;
    HalfEdge* end_r = right + n_edges;

    while ((l < end_l) && (r < end_r)) {
        switch (cross_cmp(*l, *r)) {
            case -1:
                l++;
                break;
            case  1:
                r++;
                break;
            default:
                (*l)->o = (*r);
                (*r)->o = (*l);
                l++;r++;
                break;
        }
    }

    oDestroy(left);
    oDestroy(right);
    fclose(fin);

    struct Spectrum sp = {
        .thr = thr,
        .tri = tri,
        .fringe = fringe,
        .vol = v,
        .active_threads = active_thr,
        .max_force = 2,
        .area_sqr = 0.0,
        .lambda = 0.04,
    };


    Spectrum ret = oCopy(&sp, sizeof (struct Spectrum));

    pthread_mutex_init(&ret->mutex, 0);

    return ret;
}

struct interp_kit {
    real ierror;
    real nerror;
    Spectrum sp;
};

inline static
void interp_error(uint64_t i, Obj o, Obj d)
{
    struct interp_kit* kit = d;
    Thread* t = o;
    conjecture ((*t)->t == *t, "Not the representing thread.");

    Sample s = arrBack((*t)->samples);

    kit->ierror += algoAbs(s->iso - sfValue(kit->sp->vol->scal, s->p[0], s->p[1], s->p[2]));
    Normal n; vCopy(&s->n, &n);
    Normal in; ignore vfValue(kit->sp->vol->grad, &in, s->p[0], s->p[1], s->p[2]);

    Normal c;
    kit->nerror += vNorm(vCross(vNormalizeI(&n), vNormalizeI(&in), &c));

}

inline static
void interp_bar_error(uint64_t i, Obj o, Obj d)
{
    struct interp_kit* kit = d;
    HalfEdge e = o;
    Thread a = e->t; while (a != a->t) a = a->t;
    Thread b = e->n->t; while (b != b->t) b = b->t;
    Thread c = e->n->n->t; while (c != c->t) c = c->t;

    Sample sa = arrBack(a->samples);
    Sample sb = arrBack(b->samples);
    Sample sc = arrBack(c->samples);
    Vertex g;
    vAdd(&sa->p, &sb->p, &g);
    vAddI(&g, &sc->p);
    vScaleI(&g, 1./3.);

    real iso = (sa->iso + sb->iso + sc->iso)/3.;

    kit->ierror += algoAbs(iso - sfValue(kit->sp->vol->scal, g[0], g[1], g[2]));

}

inline
void specStats(Spectrum restrict s)
{
    struct interp_kit kit = { .ierror = 0.0, .nerror = 0.0, .sp = s };
    arrForEach(s->active_threads, interp_error, &kit);
    arrForEach(s->fringe, interp_bar_error, &kit);

    fprintf(stderr, "Samples: %lu\n", arrSize(s->active_threads));
    fprintf(stderr, "Triangles: %lu\n", arrSize(s->fringe)/3);
    fprintf(stderr, "Interp error: %lf\n", oCast(double, kit.ierror));
}

inline static
void del_samples(uint64_t i, Obj o, Obj d)
{
    arrDestroy(oCast(Thread, o)->samples);
}

void specDestroy(Spectrum restrict sp)
{
    arrForEach(sp->thr, del_samples, 0);
    arrDestroy(sp->thr);
    arrDestroy(sp->tri);
    vdDestroy(sp->vol);
    arrDestroy(sp->fringe);
    arrDestroy(sp->active_threads);
    pthread_mutex_destroy(&sp->mutex);
    oDestroy(sp);
}

inline static
void exert_tangent_force(uint64_t i, Obj o, Obj d)
{
    unused(d);
    unused(i);
    HalfEdge e = o;
    Thread a = e->t; while (a != a->t) a = a->t;
    Thread b = e->n->t; while (b != b->t) b = b->t;
    Thread c = e->n->n->t; while (c != c->t) c = c->t;

    Vec3* fa = &a->force;
    Vec3* fb = &b->force;
    Vec3* fc = &c->force;

    Vertex* pa = &oCast(Sample, arrBack(a->samples))->p;
    Vertex* pb = &oCast(Sample, arrBack(b->samples))->p;
    Vertex* pc = &oCast(Sample, arrBack(c->samples))->p;

    Vec3 m; // median cm
    vSubI(vScaleI(vAdd(pa, pb, &m), 0.5), pc);

    Vec3 ab;
    vNormalizeI(vSub(pb, pa, &ab));
    vScaleI(&ab, vDot(&m, &ab)/2);

    vAddI(fc, &ab);
    vScaleI(&ab, 0.5);
    vSubI(fa, &ab);
    vSubI(fb, &ab);
}
/*
struct triangle {
    Vertex g;
    real rsqr;
};

inline static
void exert_circumpet_force(uint64_t, Obj o, Obj d)
{
    unused(d);
    unused(i);
    HalfEdge e = o;
    Thread a = e->t; while (a != a->t) a = a->t;

    Vec3* fa = &a->force;
    Sample sa = arrBack(a->samples);
    Vertex* pa = &sa->p;

    struct triangle* tri = e->att;
    Vec3 og; arrSub(pa, &tri->g, &og);
    real n = vNormSquared(&og);


}
*/


static real scale = 0.3;

inline static
void exert_normal_force(uint64_t i, Obj o, Obj d)
{
    unused(i);

    Thread* t = o;
    conjecture((*t)->t == *t, "Not the representing thread.");
    Sample s = arrBack((*t)->samples);

    Normal n; vNormalize(&s->n, &n);

    vSubI(&(*t)->force, vScaleI(&n, vDot(&n, &(*t)->force)));

    real di = s->iso - sfValue(d, s->p[0], s->p[1], s->p[2]);
    ScalarField sf = d;

    vScale(&s->n, di, &n);
    vAddI(&(*t)->force, &n);

    Vec3 f; vAddI(vScale(&(*t)->force, scale, &f), &s->p);
    if (! ( f[0] > 0 && f[1] > 0 && f[2] > 0 && f[0] < sf->nx * sf->dx && f[1] < sf->ny * sf->dy && f[2] < sf->nz * sf->dz))
        vSet(&(*t)->force, 0, 0, 0);

}

inline static
void relax(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread* t = o;
    conjecture((*t)->t == *t, "Not the representing thread.");
    Sample s = arrBack((*t)->samples);
    Vec3* p = &s->p;
    Vec3* f = &(*t)->force;


    real sz = vNorm(f);

    vAddI(vScaleI(f, scale), p);

    //if (sz > *oCast(real*, d)) *oCast(real*, d) = sz;

    *oCast(real*, d) += sz;
    vSet(f, 0, 0, 0);
}


inline static
void recompute_normal(uint64_t i, Obj o, Obj d)
{
    unused(i);

    Thread* t = o;
    conjecture((*t)->t == *t, "Not the representing thread.");
    Sample s = arrBack((*t)->samples);
    while (vIsZero(vfValue(d, &s->n, s->p[0], s->p[1], s->p[2]))) {
        s->p[0] += algoRandomDouble(-0.001, 0.001);
        s->p[1] += algoRandomDouble(-0.001, 0.001);
        s->p[2] += algoRandomDouble(-0.001, 0.001);
    }
}


void specRelax(Spectrum restrict sp)
{
    call;
    real force = 1e7;
    uint64_t maxit = 15e6/arrSize(sp->fringe);
    //scale = 0.3;
    scale = 1e-2;
    real e = force;

    fprintf(stderr, "force: %lf\tscale: %lf\n", oCast(double, force), oCast(double, scale));
    fflush(stderr);

    //while (force > sp->max_force && maxit--) {
    while (scale>1e-11 && maxit--) {
        // compute the exerted force
        arrForEach(sp->fringe, exert_tangent_force, 0);
        arrForEach(sp->active_threads, exert_normal_force, sp->vol->scal);

        force = 0.0;
        //arrForEach(sp->active_threads, pre_relax, &force);

        pthread_mutex_lock(&sp->mutex);
        arrForEach(sp->active_threads, relax, &force);
        pthread_mutex_unlock(&sp->mutex);

        e /= force; // finite
        e *= e;

        if (e < 1)
            scale *= 1.01*e;
        else
            scale *= 1.0 + 0.01/e;

        fprintf(stderr, "force: %lf\tscale: %lf\n", oCast(double, force), oCast(double, scale));
        fflush(stderr);

        e = force;

        // recompute normal
        arrForEach(sp->active_threads, recompute_normal, sp->vol->grad);
    }

}

inline static
void simplify(uint64_t i, Obj o, Obj d)
{
    unused(i);
    HalfEdge a = o;  check(a);
    HalfEdge b = a->n; check(b);
    HalfEdge c = b->n; check(c);

    Thread ta = a->t;
    check(ta);
    while (ta != ta->t) {
        ta = ta->t;
        check(ta);
    }
    Thread tb = b->t;
    check(tb);
    while (tb != tb->t) {
        tb = tb->t;
        check(tb);
    }
    Thread tc = c->t;
    check(tc);
    while (tc != tc->t) {
        tc = tc->t;
        check(tc);
    }

    Sample sa = arrBack(ta->samples);
    Sample sb = arrBack(tb->samples);
    Sample sc = arrBack(tc->samples);

    // only collapse the shortest edge
    Vec3 ab, bc, ca;
    real l_ab = vNorm(vSub(&sb->p, &sa->p, &ab));
    real l_bc = vNorm(vSub(&sc->p, &sb->p, &bc));
    if (l_ab > l_bc) return;
    real l_ca = vNorm(vSub(&sa->p, &sc->p, &ca));
    if (l_ab > l_ca) return;

    // compute squared area, circumradius^2 and inradius^2
    real p = (l_ab+l_bc+l_ca)/2;
    real area_sqr = p * (p-l_ab) * (p-l_bc) * (p-l_ca);

    // inradius^2
    real irad = area_sqr / p/p;

    // circumradius
    real crad = 0.5*l_ab*l_ab*0.5*l_bc*l_bc*0.5*l_ca*l_ca*0.5/area_sqr;

    Spectrum sp = d;
    if (area_sqr > sp->area_sqr && irad/crad > sp->lambda) return;

    // TODO: simplifying this triangle is desirable if manifold prezervation is possible

    if (a->o) {
        Vector oa = vecSort(heOneCircle(a), algoComparePtr);
        Vector oo = vecSort(heOneCircle(a->o), algoComparePtr);

        uint64_t ia = 0, io = 0;

        while (ia < vecSize(oa) && io < vecSize(oo)) {
            HalfEdge* ea = vecGet(oa, ia);
            HalfEdge* eo = vecGet(oo, io);
            if (algoComparePtr(ea, eo) > 0) io++;
            else if (algoComparePtr(ea, eo) < 0) ia++;
            else {// they share an edge
                vecDestroy(oo);
                vecDestroy(oa);
                return;
            }
        }

        oa = vecSort(oa, cmp_edges);
        oo = vecSort(oo, cmp_edges);

        ia = 0, io = 0;

        while (ia < vecSize(oa) && io < vecSize(oo)) {
            HalfEdge* ea = vecGet(oa, ia);
            HalfEdge* eo = vecGet(oo, io);
            if (cmp_edges(ea, eo) > 0) io++;
            else if (cmp_edges(ea, eo) < 0) ia++;
            else if ((cmp_edges(&a->n->n, ea) != 0) && (cmp_edges(&a->o->n->n, ea) != 0)) { // they share a thread other than the two intersections of the one-circles
                    vecDestroy(oo);
                    vecDestroy(oa);
                    return;
                } else {io++; ia++;}
        }

    }


    fprintf(stderr, "collapsing <%u, %u> A = %lf  rR = %lf/%lf  %lf %lf %lf\n", ta->id, tb->id, area_sqr, irad, crad, l_ab, l_bc, l_ca);

    pthread_mutex_lock(&sp->mutex);

    if (b->o) b->o->o = c->o;
    if (c->o) c->o->o = b->o;
    ignore vScaleI(vAddI(&sb->p, &sa->p), 0.5);
    vCopy(&sb->p, &sa->p);
    sa->iso = sb->iso = sfValue(sp->vol->scal, sa->p[0], sa->p[1], sa->p[2]);
    vCopy(vfValue(sp->vol->grad, &sb->n, sa->p[0], sa->p[1], sa->p[2]), &sa->n);

    // store the collapse of ta->id and tb->id
    if (ta->depth > tb->depth) {
        // tb -> ta
        tb->t = ta;
        tb->iso = sb->iso;
        tb->active = false;
    } else if (tb->depth > ta->depth) {
        // ta -> tb
        ta->t = tb;
        ta->iso = sa->iso;
        ta->active = false;
    } else {
        // ta -> tb
        ta->t = tb;
        tb->depth ++;
        ta->iso = sa->iso;
        ta->active = false;
    }


    // store the triangloid with the initial ids
    {
        struct Triangloid tria = {
            .iso = { a->iso , sa->iso},
            .t_ids = {a->t->id, b->t->id, c->t->id}
        };
        arrPush(sp->tri, &tria);
    }
    // save a pointer to the opposite edge
    HalfEdge opp = a->o;

    // delete edges
    a->o = 0; b->o = 0; c->o = 0;
    Obj bk = arrBack(sp->fringe);
    if (a!= bk) {
        oCopyTo(a, bk, sizeof(struct HalfEdge));
        if (a->o) a->o->o = a;
        if (bk != b && bk != c) a->n->n->n = a;
    }
    arrPop(sp->fringe);
    bk = arrBack(sp->fringe);
    if (b!= bk) {
        oCopyTo(b, bk, sizeof(struct HalfEdge));
        if (b->o) b->o->o = b;
        if (bk != c) b->n->n->n = b;
    }
    arrPop(sp->fringe);
    bk = arrBack(sp->fringe);
    if (c!= bk) {
        oCopyTo(c, bk, sizeof(struct HalfEdge));
        if (c->o) c->o->o = c;
        b->n->n->n = b;
    }
    arrPop(sp->fringe);

    if (opp) {
        a = opp;
        b = a->n;
        c = b->n;
        if (b->o) b->o->o = c->o;
        if (c->o) c->o->o = b->o;
        ignore vScaleI(vAddI(&sb->p, &sa->p), 0.5);
        vCopy(&sb->p, &sa->p);
        sa->iso = sb->iso = sfValue(sp->vol->scal, sa->p[0], sa->p[1], sa->p[2]);
        vCopy(vfValue(sp->vol->grad, &sb->n, sa->p[0], sa->p[1], sa->p[2]), &sa->n);

        // store the triangloid with the initial ids
        struct Triangloid tria = {
            .iso = { a->iso , sa->iso},
            .t_ids = {a->t->id, b->t->id, c->t->id}
        };
        arrPush(sp->tri, &tria);

        // delete edges
        a->o = 0; b->o = 0; c->o = 0;
        bk = arrBack(sp->fringe);
        if (a!= bk) {
            oCopyTo(a, bk, sizeof(struct HalfEdge));
            if (a->o) a->o->o = a;
            if (bk != b && bk != c) a->n->n->n = a;
        }
        arrPop(sp->fringe);
        bk = arrBack(sp->fringe);
        if (b!= bk) {
            oCopyTo(b, bk, sizeof(struct HalfEdge));
            if (b->o) b->o->o = b;
            if (bk != c) b->n->n->n = b;
        }
        arrPop(sp->fringe);
        bk = arrBack(sp->fringe);
        if (c!= bk) {
            oCopyTo(c, bk, sizeof(struct HalfEdge));
            if (c->o) c->o->o = c;
            b->n->n->n = b;
        }
        arrPop(sp->fringe);

    }

    pthread_mutex_unlock(&sp->mutex);

}

void specSimplify(Spectrum restrict sp)
{
    call;
    arrForEach(sp->fringe, simplify, sp);
    arrForEach(sp->active_threads, remove_inactive_threads, sp->active_threads);
}

void specRefine(Spectrum restrict sp)
{
}

void specProject(Spectrum restrict sp)
{
}

void specSample(Spectrum restrict sp)
{
}

void specMerge(Spectrum restrict sp)
{
}

void display_edge(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);

    HalfEdge e = o;
    if ((e->t->id > e->n->t->id) || (e->t->id > e->n->n->t->id)) return;

    Sample sa = arrBack(e->t->samples);
    Sample sb = arrBack(e->n->t->samples);
    Sample sc = arrBack(e->n->n->t->samples);

    Vec3 n;

    vScale(&sa->n, -1, &n);
    glNormal3v(n);
    glVertex3v(sa->p);
    vScale(&sc->n, -1, &n);
    glNormal3v(n);
    glVertex3v(sc->p);
    vScale(&sb->n, -1, &n);
    glNormal3v(n);
    glVertex3v(sb->p);
}

void display_sample(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);
    glVertex3v(oCast(Sample, o)->p);
}

void display_thread(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);
    glBegin(GL_LINE_STRIP);
    arrForEach(oCast(Thread, o)->samples, display_sample, 0);
    glEnd();
}

void display_vert(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);
    Thread* t = o;
    Sample s = arrBack((*t)->samples);
    if (vIsZero(&s->n)) glVertex3v(s->p);
}

void specDisplay(Spectrum restrict sp)
{
    glTranslatef(-32, -32, -32);
    glLineWidth(1.0);

    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0);

    pthread_mutex_lock(&sp->mutex);

    arrForEach(sp->thr, display_thread, 0);


    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);
    arrForEach(sp->fringe, display_edge, 0);
    glEnd();

    //glLineWidth(2.0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    arrForEach(sp->fringe, display_edge, 0);
    glEnd();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glPointSize(40.0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POINTS);
    arrForEach(sp->active_threads, display_vert, 0);

    pthread_mutex_unlock(&sp->mutex);

    glEnd();

    glEnable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
}

void specObserve(Spectrum restrict sp)
{
    pthread_mutex_lock(&sp->mutex);
    sp->observers++;
    pthread_mutex_unlock(&sp->mutex);
}

void specForget(Spectrum restrict sp)
{
    pthread_mutex_lock(&sp->mutex);
    if (sp->observers) sp->observers--;
    pthread_mutex_unlock(&sp->mutex);
}
