
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
#include <unistd.h>

typedef struct Sample* Sample;
typedef struct Thread* Thread;
typedef struct Triangloid* Triangloid;
typedef struct Triangle* Triangle;
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


inline static
void snap_sample(Sample restrict s, Spectrum restrict sp)
{
    real iso = sfValue(sp->vol->scal, s->p[0], s->p[1], s->p[2]);
    real d_iso = sp->snap_iso - iso;
    Normal dn;
    while (algoAbs(d_iso) > sp->snap_iso_thr) {
        vScale(vfValue(sp->vol->grad, &s->n, s->p[0], s->p[1], s->p[2]), d_iso, &dn);
        vAddI(&dn, &s->p);
        iso = sfValue(sp->vol->scal, dn[0], dn[1], dn[2]);
        while (algoAbs(d_iso) <= algoAbs(sp->snap_iso - iso)) {
            dn[0] = algoRandomDouble(-0.001, 0.001);
            dn[1] = algoRandomDouble(-0.001, 0.001);
            dn[2] = algoRandomDouble(-0.001, 0.001);
            vAddI(&dn, &s->p);
            iso = sfValue(sp->vol->scal, dn[0], dn[1], dn[2]);
        }
        d_iso = sp->snap_iso - iso;
        vCopy(&dn, &s->p);
        s->iso = iso;
    }
}

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
    Vec3 force_t;
    Vec3 force_n;

    // the "relaxed" sample;
    struct Sample relaxed;

    bool active;
};

// a 3D shape bordered by 3 threads with 2 end-points defined by their isovalues
struct Triangloid {
    // isovalue range for which this triangloid is defined (iso[0] <= iso[1])
    float iso[2];
    // thread IDs defining the vertices of each slice.
    uint32_t t_ids[3];
};

// a temporary struct for storing active triangles
struct Triangle {
    Vertex g;
    real r4;
    real AA16;
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
Vector heOneRing(HalfEdge e)
{
    if (!e) return 0;
    const HalfEdge const flag = e;
    Thread thr = e->t; while(thr->t != thr) thr = thr->t;
    uint32_t id = thr->id;

    Vector v = vecCreate(sizeof (HalfEdge));
    do {
        vecPush(v, &e->n);
        e = e->n->n->o;
        safe(thr = e->t; while(thr->t != thr) thr = thr->t;);
        if (id != thr->id)
            check(id == thr->id);
        usage(vecSize(v) < 1000); // a vertex should not have >= 1000 incident triangles
    } while (e && e!=flag);

    if (e==0) {
        check(0);
        e = flag->o;
        while (e) {
            e = e->n;
            vecPush(v, &e->n);
            usage(vecSize(v) < 1000); // a vertex should not have >= 1000 incident triangles
            e = e->o;
        }
    }

    return v;
}

inline static
void hePrintOneRing(HalfEdge e){
    Vector oc_a = heOneRing(e);
    Thread thr = e->t; while (thr != thr->t) thr = thr->t;
    fprintf(stderr, "One ring of #%u: <", thr->id);
    uint64_t i;
    for (i=0; i+1<vecSize(oc_a); i++) {
        HalfEdge* half_e = vecGet(oc_a, i);
        Thread thr = (*half_e)->t; while (thr != thr->t) thr = thr->t;
        fprintf(stderr, "%u, ", thr->id);
    }
    HalfEdge* half_e = vecGet(oc_a, i);
    thr = (*half_e)->t; while (thr != thr->t) thr = thr->t;
    fprintf(stderr, "%u", thr->id);
    fprintf(stderr, ">\n"); fflush(stderr);
    vecDestroy(oc_a);
}

inline static
int cmp_edges(const void* a, const void* b)
{
    Thread ta = (*oCast(HalfEdge*, a))->t; while (ta->t != ta) ta = ta->t;
    Thread tb = (*oCast(HalfEdge*, b))->t; while (tb->t != tb) tb = tb->t;

    if (ta->id < tb->id) return -1; else if (ta == tb) return 0; else return 1;
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

inline static
void snap_thread_endpoint(uint64_t i, Obj o, Obj d)
{
    Thread* t = o;
    snap_sample(arrBack((*t)->samples), d);
}

inline
void specSnap(Spectrum restrict sp)
{
    arrForEach(sp->active_threads, snap_thread_endpoint, sp);
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
    fprintf(stderr, "%u vertices, %u faces\n", vert, faces); fflush(stderr);

    // read vertices
    uint64_t i = 0;
    struct Sample smpl = { .p = VERT_0, .n = VERT_0, .iso = initial_isovalue };
    for (i = 0; i < vert; i++) {
        conjecture(fgets(line, sizeof(line), fin), "Error reading from off file.");
        conjecture(sscanf(line, "%f %f %f\n", &x, &y, &z) == 3, "failed to read vertex from off file.");
        //x+= 0.2; y+=0.2; z+=0.2;
        vfValue(v->grad, &smpl.n, x, y, z);
/*
        while (vIsZero(vfValue(v->grad, &smpl.n, x, y, z))) {
            x += algoRandomDouble(0.0001, 0.001);
            y += algoRandomDouble(0.0001, 0.001);
            z += algoRandomDouble(0.0001, 0.001);
        }
*/
        vSet(&smpl.p, x+0.2, y+0.2, z+0.2);

        struct Thread tmp_thr = {
            .samples = arrCreate(sizeof (struct Sample), 1),
            .id = i,
            .iso = initial_isovalue
        };
        ignore arrPush(tmp_thr.samples, &smpl);
        Thread t = arrPush(thr, &tmp_thr);
        t->t = t; // for now, the thread represents itself
        ignore arrPush(active_thr, &t);
    }

    // read faces
    unsigned int a, b, c, cnt;
//    Normal p, q, norm;

    fprintf(stderr, "Reading faces.\n"); fflush(stderr);
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
    }

    fprintf(stderr, "Eliminating inactive threads.\n"); fflush(stderr);
    // eliminate disconnected vertices
    arrForEach(active_thr, remove_inactive_threads, active_thr);

    fprintf(stderr, "Computing the opposing edges.\n"); fflush(stderr);
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
        .active_triangles = 0,
        .snap_iso = initial_isovalue,
        .snap_iso_thr = 10./arrSize(active_thr),
        .ref_norm_thr = 0.2
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
    arrDestroy(sp->active_triangles);
    pthread_mutex_destroy(&sp->mutex);
    oDestroy(sp);
}

inline static
void relax(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread* t = o;
    conjecture((*t)->t == *t, "Not the representing thread.");
    Sample s = arrBack((*t)->samples);
    Vec3* p = &s->p;

    vAddI(p, &(*t)->force_t);
    vAddI(p, &(*t)->force_n);

    vSet(&(*t)->force_t, 0, 0, 0);
    vSet(&(*t)->force_n, 0, 0, 0);
}

inline static
void compute_triangles(uint64_t i, Obj o, Obj d)
{
    Spectrum sp = d;
    HalfEdge ha = o;
    HalfEdge hb = ha->n;
    HalfEdge hc = hb->n;
    // only compute for one of the three half-edges in a triangle
    if (ha > hb || ha > hc) return;

    Thread a = ha->t; while (a != a->t) a = a->t;
    Thread b = hb->t; while (b != b->t) b = b->t;
    Thread c = hc->t; while (c != c->t) c = c->t;

    Sample sa = arrBack(a->samples);
    Sample sb = arrBack(b->samples);
    Sample sc = arrBack(c->samples);

    Vertex* pa = &sa->p;
    Vertex* pb = &sb->p;
    Vertex* pc = &sc->p;

    struct Triangle tr = { .g = VERT_0, .r4 = 0, .AA16 = 0 };
    vScaleI(vAddI(vAdd(pa, pb, &tr.g), pc), 1./3.);
    Vec3 ab, ac;
    tr.AA16 = vNormSquared(vCrossI(vSub(pb, pa, &ab), vSub(pc, pa, &ac)));

//    check(tr.AA16 > 1e-3);

    /*
    Vertex tmp;
    real ab2 = vNormSquared(vSub(b, a, &tmp));
    real bc2 = vNormSquared(vSub(c, b, &tmp));
    real ca2 = vNormSquared(vSub(a, c, &tmp));

    if (ab2 >= bc2)
        if (bc2 >= ca2)
            // ab2 >= bc2 >= ca2
            x = ab2; y = bc2; z = ca2;
        else
            if (ab2 >= ca2)
                // ab2 >= ca2 > bc2
                x = ab2; y = ca2; z = bc2;
            else
                // ca2 > ab2 > bc2
                x = ca2; y = ab2; z = bc2;
    else
        if (ab2 >= ca2)
            // bc2 > ab2 >= ca2
            x = bc2; y = ab2; z = ca2;
        else
            if (bc2 >= ca2)
                // bc2 >= ca2 > ab2
                x =  bc2 y = ca2; z = ab2;
            else
                // ca2 > bc2 > ab2
                x = ca2; y = bc2; z = ab2;

    tr.AA16 = (x-y)*(x-y) - z(4*sqrt(x*y)+z);
    */
    tr.r4 = tr.AA16/27;

    ha->att = hb->att = hc->att = arrPush(sp->active_triangles, &tr);
}

inline static
void compute_idealizing_force(uint64_t i, Obj o, Obj d)
{
    unused(d);
    unused(i);
    HalfEdge e = o;

    if (e > e->n || e > e->n->n) return;

    Thread a = e->t; while (a != a->t) a = a->t;
    Thread b = e->n->t; while (b != b->t) b = b->t;
    Thread c = e->n->n->t; while (c != c->t) c = c->t;

    Vec3* fa = &a->force_t;
    Vec3* fb = &b->force_t;
    Vec3* fc = &c->force_t;

    Vertex* pa = &a->relaxed.p;
    Vertex* pb = &b->relaxed.p;
    Vertex* pc = &c->relaxed.p;

    Vec3 l;
    Vec3 m; // median

    vSubI(vScaleI(vAdd(pb, pa, &m), 0.5), pc);
    vNormalizeI(vSub(pb, pa, &l));
    vScaleI(&l, vDot(&m, &l)/2);
    vAddI(fc, &l);  vScaleI(&l, 0.5);  vSubI(fa, &l); vSubI(fb, &l);

    vSubI(vScaleI(vAdd(pc, pb, &m), 0.5), pa);
    vNormalizeI(vSub(pc, pb, &l));
    vScaleI(&l, vDot(&m, &l)/2);
    vAddI(fa, &l);  vScaleI(&l, 0.5);  vSubI(fb, &l); vSubI(fc, &l);

    vSubI(vScaleI(vAdd(pa, pc, &m), 0.5), pb);
    vNormalizeI(vSub(pa, pc, &l));
    vScaleI(&l, vDot(&m, &l)/2);
    vAddI(fb, &l);  vScaleI(&l, 0.5);  vSubI(fc, &l); vSubI(fa, &l);

    Triangle t = e->att;
    vNormalizeI(vCrossI(&m, &l)); // m is now parallel to the triangle's normal
    vAddI(vScaleI(&m, vDot(&m, vSub(pa, &t->g, &l))), &t->g); // m is the projection of e->g on the triangle

    real d2;
    d2 = vNormSquared(vSub(pa, &m, &l)); vAddI(fa, vScaleI(&l, t->r4/d2 - d2));
    d2 = vNormSquared(vSub(pb, &m, &l)); vAddI(fb, vScaleI(&l, t->r4/d2 - d2));
    d2 = vNormSquared(vSub(pc, &m, &l)); vAddI(fc, vScaleI(&l, t->r4/d2 - d2));
}

inline static
void extract_tangent_force(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    Sample s = &t->relaxed;
    Vec3 n;

    if (vNormSquared(&s->n) == 0) {
        Sample st = arrBack(t->samples);
        fprintf(stderr, "Has zero normal: relaxed:"); vPrint(&s->p, stderr); fprintf(stderr, " initial:"); vPrint(&st->p, stderr); fprintf(stderr, "\n"); fflush(stderr);
    }
    vNormalize(&s->n, &n);
    vScaleI(&n, vDot(&n, &t->force_t));
    vSubI(&t->force_t, &n);
}

inline static
void compute_stress(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    check(t->active);
    Spectrum sp = d;
    Sample s = &t->relaxed;

    sp->current_stress += vNormSquared(&t->force_t);
    real diso = algoAbs(s->iso - sfValue(sp->vol->scal, s->p[0], s->p[1], s->p[2]));
    sp->current_stress += vNormSquared(&s->n) * diso * diso;
}


inline static
void compute_normal_force(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    check(t->active);
    Sample s = &t->relaxed;

    snap_sample(s, d);

    return;
}


inline static
void set_relaxed_to_current(uint64_t i, Obj o, Obj d)
{
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    check(t->active);
    oCopyTo(&t->relaxed, arrBack(t->samples), sizeof (struct Sample));
    vSet(&t->force_t, 0, 0, 0);
    vSet(&t->force_n, 0, 0, 0);
}

inline static
void set_current_to_relaxed(uint64_t i, Obj o, Obj d)
{
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    check(t->active);
    oCopyTo(arrBack(t->samples), &t->relaxed, sizeof (struct Sample));
    vSet(&t->force_n, 0, 0, 0);
}


void specRelax(Spectrum restrict sp)
{
    call;

    sp->scale = 0.3;
    sp->scale_threshold = 1e-5;

    arrDestroy(sp->active_triangles);
    sp->active_triangles = arrCreate(sizeof(struct Triangle), 4);


    // pre-compute the barycenter and the radius of the ideal triangle
    // debug(fprintf(stderr, "Precomputing barycenter.\n"); fflush(stderr););
    arrForEach(sp->fringe, compute_triangles, sp);

    // set the "relaxed" point to be the current point
    // debug(fprintf(stderr, "Setting relaxed point to current.\n"); fflush(stderr););
    arrForEach(sp->active_threads, set_relaxed_to_current, sp);

    // compute idealizing force
    // debug(fprintf(stderr, "Computing the idealizing force.\n"); fflush(stderr););
    arrForEach(sp->fringe, compute_idealizing_force, sp);

    // extract tangent component
    // debug(fprintf(stderr, "Extracting tangent component.\n"); fflush(stderr););
    arrForEach(sp->active_threads, extract_tangent_force, sp);

    // compute stress
    // debug(fprintf(stderr, "Computing initial stress.\n"); fflush(stderr););
    sp->current_stress = 0;
    arrForEach(sp->active_threads, compute_stress, sp);


    bool relaxing = true;
    while (relaxing) {
        sp->previous_stress = sp->current_stress;

        bool scaling = true;
        while (scaling) {
            sp->scale *= 0.5;
            if (sp->scale < sp->scale_threshold) break;

            // compute the normal component of the force
            // compute the relaxed point
            // debug(fprintf(stderr, "Computing normal force.\n"); fflush(stderr););
            arrForEach(sp->active_threads, compute_normal_force, sp);

            // compute idealizing force
            // debug(fprintf(stderr, "Computing the idealizing force.\n"); fflush(stderr););
            arrForEach(sp->fringe, compute_idealizing_force, sp);
            // extract tangent component
            // debug(fprintf(stderr, "Extracting tangent component.\n"); fflush(stderr););
            arrForEach(sp->active_threads, extract_tangent_force, sp);
            // compute stress
            // debug(fprintf(stderr, "Computing initial stress.\n"); fflush(stderr););
            sp->current_stress = 0;
            arrForEach(sp->active_threads, compute_stress, sp);

            // debug(fprintf(stderr, "<prev, crt> : <%10.4lf, %10.4lf>\n", sp->previous_stress, sp->current_stress);fflush(stderr););
            if (sp->current_stress > sp->previous_stress)
                // set the "relaxed" point to be the current point
                arrForEach(sp->active_threads, set_relaxed_to_current, sp);
            else
                scaling = false;

        }

        // debug(fprintf(stderr, "! <prev, crt> : <%10.4lf, %10.4lf>\n", sp->previous_stress, sp->current_stress););
        if (!scaling) {
            // relax the current point by moving it to the "relaxed" point
            // debug(fprintf(stderr, "Setting current point to relaxed.\n"); fflush(stderr););
            pthread_mutex_lock(&sp->mutex);
            arrForEach(sp->active_threads, set_current_to_relaxed, 0);
            pthread_mutex_unlock(&sp->mutex);
            sp->scale *= 2.5;
        } else {
            relaxing = false;
        }

    }

}

inline static
void simplify(uint64_t ind, Obj o, Obj d)
{
    unused(ind);
    HalfEdge a = o;  check(a);
    HalfEdge b = a->n; check(b);
    HalfEdge c = b->n; check(c);

    Thread ta = a->t;
    while (ta != ta->t) ta = ta->t;
    Thread tb = b->t;
    while (tb != tb->t) tb = tb->t;
    Thread tc = c->t;
    while (tc != tc->t) tc = tc->t;

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
        Vector oa = vecSort(heOneRing(a), algoComparePtr);
        Vector oo = vecSort(heOneRing(a->o), algoComparePtr);

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
        vecDestroy(oo);
        vecDestroy(oa);

    }

    pthread_mutex_lock(&sp->mutex);

    // save a pointer to the opposite edge
    HalfEdge opp = a->o; opp->o = opp;
    if (b->o) b->o->o = c->o;
    if (c->o) c->o->o = b->o;
    a->o = a->n = a;
    b->o = b->n = b;
    c->o = c->n = c;


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
    } else  {
        // ta -> tb
        ta->t = tb;
        if (tb->depth == ta->depth) tb->depth ++;
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

    // delete edges
    HalfEdge bk = arrBack(sp->fringe);
    if (a!= bk) {
        if (bk == opp) opp = a;
        oCopyTo(a, bk, sizeof(struct HalfEdge));
        memset(bk, 0, sizeof (struct HalfEdge));
        if (a->o)
            a->o->o = a;
        else
            check(0);
        if (a->n->n) {
            a->n->n->n = a;
        } else {
            a->n = a;
            a->o = a;
        }
    }
    arrPop(sp->fringe);

    bk = arrBack(sp->fringe);
    if (b!= bk) {
        if (bk == opp) opp = b;
        oCopyTo(b, bk, sizeof(struct HalfEdge));
        memset(bk, 0, sizeof (struct HalfEdge));
        if (b->o) b->o->o = b;
        else
            check(0);
        if(b->n->n) {
            b->n->n->n = b;
        } else {
            b->n = b;
            b->o = b;
        }
    }
    arrPop(sp->fringe);

    bk = arrBack(sp->fringe);
    if (c!= bk) {
        if (bk == opp) opp = c;
        oCopyTo(c, bk, sizeof(struct HalfEdge));
        memset(bk, 0, sizeof (struct HalfEdge));
        if (c->o) c->o->o = c;
        else
            check(0);
        if (c->n->n) {
            c->n->n->n = c;
        } else {
            c->n = c;
            c->o = c;
        }
    }
    arrPop(sp->fringe);

    if (opp) {
        a = opp;
        b = a->n;
        c = b->n;
        if (b->o) b->o->o = c->o;
        if (c->o) c->o->o = b->o;
        a->o = a->n = a;
        b->o = b->n = b;
        c->o = c->n = c;

        // store the triangloid with the initial ids
        struct Triangloid tria = {
            .iso = { a->iso , sa->iso},
            .t_ids = {a->t->id, b->t->id, c->t->id}
        };
        arrPush(sp->tri, &tria);

        // delete edges
        bk = arrBack(sp->fringe);
        if (a!= bk) {
            oCopyTo(a, bk, sizeof(struct HalfEdge));
            memset(bk, 0, sizeof (struct HalfEdge));
            if (a->o) a->o->o = a;
            else
                check(0);
            if (a->n->n) {
                a->n->n->n = a;
            } else {
                a->n = a;
                a->o = a;
            }
        }
        arrPop(sp->fringe);

        bk = arrBack(sp->fringe);
        if (b!= bk) {
            oCopyTo(b, bk, sizeof(struct HalfEdge));
            memset(bk, 0, sizeof (struct HalfEdge));
            if (b->o) b->o->o = b;
            else
                check(0);
            if (b->n->n) {
                b->n->n->n = b;
            } else {
                b->n = b;
                b->o = b;
            }
        }
        arrPop(sp->fringe);

        bk = arrBack(sp->fringe);
        if (c!= bk) {
            oCopyTo(c, bk, sizeof(struct HalfEdge));
            memset(bk, 0, sizeof (struct HalfEdge));
            if (c->o) c->o->o = c;
            else
                check(0);
            if (c->n->n) {
                c->n->n->n = c;
            } else {
                c->n = b;
                c->o = c;
            }
        }
        arrPop(sp->fringe);
    }

    pthread_mutex_unlock(&sp->mutex);

}

void specSimplify(Spectrum restrict sp)
{
    call;
    sp->area_sqr = 0.02;
    sp->lambda = 0.04;
    arrForEach(sp->fringe, simplify, sp);
    arrForEach(sp->active_threads, remove_inactive_threads, sp->active_threads);
}

struct refinement_kit {
    Spectrum sp;
    Array q; // the current queue
};

inline static
void stage1(uint64_t i, Obj o, Obj d)
{
    struct refinement_kit* ref = d;
    while (i < arrSize(ref->q)) {
        HalfEdge* pe = o;
        HalfEdge e = *pe;

        Thread t = e->t; while (t->t != t) t = t->t;
        Thread tn = e->n->t; while (tn->t != tn) tn = tn->t;

        // check if the opposite was already processed
        if (e->o && e->o->o == 0) {
            e->o->o = e;
            e->att = e->o->att;
            if (!e->att) {
                HalfEdge* bk = arrBack(ref->q);
                if (bk != pe) oCopyTo(pe, bk, sizeof (HalfEdge));
                arrPop(ref->q);
                continue;
            }
            return;
        }

        // only need to handle one of the two opposing half edges,
        e->o = 0;

        Sample s = arrBack(t->samples);
        Sample sn = arrBack(tn->samples);

        // compute midpoint sample
        struct Sample sm;
        vScaleI(vAdd(&s->p, &sn->p, &sm.p), .5);
        vScaleI(vAdd(&s->n, &sn->n, &sm.n), .5);
        sm.iso = (s->iso + sn->iso)/2;

        // get the real midpoint values
        Normal n; vfValue(ref->sp->vol->grad, &n, sm.p[0], sm.p[1], sm.p[2]);
        real iso = sfValue(ref->sp->vol->scal, sm.p[0], sm.p[1], sm.p[2]);

        if (    algoAbs(iso - sm.iso) < 2 * ref->sp->snap_iso_thr &&
                vDot(&n, &sm.n) > ref->sp->ref_norm_thr ) {
            // it does not have to be split
            e->att = 0;
            HalfEdge* bk = arrBack(ref->q);
            if (bk != pe) oCopyTo(pe, bk, sizeof (HalfEdge));
            arrPop(ref->q);
            continue;
        } else {
            // it needs to be split
            snap_sample(&sm, ref->sp);
            struct Thread thr = {
                .samples = arrCreate(sizeof(struct Sample), 1),
                .id = arrSize(ref->sp->thr),
                .t = 0,
                .depth = 0,
                .iso = 0,
                .force_t = VERT_0,
                .force_n = VERT_0,
                .active = 1
            };
            arrPush(thr.samples, &sm);
            oCopyTo(&thr.relaxed, &sm, sizeof (struct Sample));
            e->att = arrPush(ref->sp->thr, &thr);
            oCast(Thread, e->att)->t = e->att;
            arrPush(ref->sp->active_threads, &e->att);
            return;
        }
    }
}

inline static
void stage23(uint64_t i, Obj o, Obj d)
{
    HalfEdge* pe = o;
    HalfEdge e = *pe;
    struct refinement_kit* ref = d;

    check(e->att);

    // it needs to be split
    if ((e->n->att) && !(e->n->n->att)) e = e->n->n;
    else if (!(e->n->att) && (e->n->n->att)) e = e->n->att;
    else return;

    Thread t = e->t; while (t->t != t) t = t->t;
    Thread tn = e->n->t; while (tn->t != tn) tn = tn->t;

    Sample s = arrBack(t->samples);
    Sample sn = arrBack(tn->samples);

    // compute midpoint sample
    struct Sample sm;
    vScaleI(vAdd(&s->p, &sn->p, &sm.p), .5);
    vScaleI(vAdd(&s->n, &sn->n, &sm.n), .5);
    sm.iso = (s->iso + sn->iso)/2;

    snap_sample(&sm, ref->sp);
    struct Thread thr = {
        .samples = arrCreate(sizeof(struct Sample), 1),
        .id = arrSize(ref->sp->thr),
        .t = 0,
        .depth = 0,
        .iso = 0,
        .force_t = VERT_0,
        .force_n = VERT_0,
        .active = 1
    };
    arrPush(thr.samples, &sm);
    oCopyTo(&thr.relaxed, &sm, sizeof (struct Sample));
    e->att = arrPush(ref->sp->thr, &thr);
    oCast(Thread, e->att)->t = e->att;
    arrPush(ref->sp->active_threads, &e->att);

    arrPush(ref->q, &e);
    if (e->o) {
        e->o->att = e->att;
        arrPush(ref->q, &e->o);
    }
}

inline static
void split_match(HalfEdge a)
{
    // case *1.1: b does not exist
    if (!a->o) return;
    HalfEdge b = a->o;

    // case *1.2: b was not yet split
    if (a->n->t == b->att) return;

    // case *2: b exists and was split
    HalfEdge a_ = a->att;
    HalfEdge b_ = b->att;

    a->o = b_; a_->o = b;
    b->o = a_; b_->o = a;
}

inline static
Triangloid storeTriangloid(HalfEdge a, Array tri)
{
    // compute the final isovalue for each thread
    Thread ta = a->t; while (ta!=ta->t) ta = ta->t;
    Thread tb = a->n->t; while (tb!=tb->t) tb = tb->t;
    Thread tc = a->n->n->t; while (tc!=tc->t) tc = tc->t;

    Sample sa = arrBack(ta->samples);
    Sample sb = arrBack(tb->samples);
    Sample sc = arrBack(tc->samples);

    real iso = (sa->iso < sb->iso)?(sa->iso):(sb->iso);
    if (sc->iso < iso) iso = sc->iso;

    // store the triangloid with the initial ids
    struct Triangloid tria = {
        .iso = { a->iso , iso},
        .t_ids = {a->t->id, a->n->t->id, a->n->n->t->id}
    };
    return arrPush(tri, &tria);
}

inline static
void split_case_1(HalfEdge a, struct refinement_kit* ref)
{
    Triangloid t = storeTriangloid(a, ref->sp->tri);
    HalfEdge a1, a2, a3;
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = t->iso[1], .att = 0 };
        a1 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->n->n->t, .n = 0, .o = 0, .iso = t->iso[1], .att = 0 };
        a2 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = t->iso[1], .att = 0 };
        a3 = arrPush(ref->sp->fringe, &tmp);
    }
    a1->n = a->n->n; a1->o = a2;
    a2->n = a3; a2->o = a1;
    a3->n = a->n;

    a->n->n = a2; a->n = a1; a->att = a3;
}

inline static
void split_case_2(HalfEdge a, struct refinement_kit* ref)
{
}

inline static
void stage45(uint64_t i, Obj o, Obj d)
{
    HalfEdge* pe = o;
    HalfEdge e = *pe;

    // check if it is still marked
    if (!e->att) return;

    conjecture((e->n->att == 0) ^ (e->n->n->att == 0), "The triangle is neither in case 1 nor in case 2.");

    if (e->n->att && e->n->n->att) split_case_1(e, d);
    else split_case_2(e, d);
}

inline static
void stage6(uint64_t i, Obj o, Obj d)
{
    HalfEdge* pe = o;
    HalfEdge e = *pe;
    struct refinement_kit* ref = d;

    arrPush(ref->q, pe);
    if (e->o) arrPush(ref->q, &e->o);
}

void specRefine(Spectrum restrict sp)
{

    // specSnapFringeToIso(sp);

    struct refinement_kit ref_kit = {
        .sp = sp,
        .q = 0,
    };

    Array q1 = arrRefsArr(sp->fringe);

    while (!arrIsEmpty(q1)) {
        // stage #1
        ref_kit.q = q1;
        arrForEach(q1, stage1, &ref_kit);

        // stage #2
        Array q2 = ref_kit.q = arrCreate(sizeof (HalfEdge), 1);
        arrForEach(q1, stage23, &ref_kit);

        // stage #3
        arrForEach(q2, stage23, &ref_kit);

        // stage #4
        Array q3 = ref_kit.q = arrCreate(sizeof (HalfEdge), 1);
        arrForEach(q1, stage45, &ref_kit);

        // stage #5
        arrForEach(q2, stage45, &ref_kit);

        // stage #6
        arrForEach(q1, stage6, &ref_kit);

        arrDestroy(q1);
        q1 = q3;
        arrDestroy(q2);
    }
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
    if ((e->t > e->n->t) || (e->t > e->n->n->t)) return;
    Thread ta = e->t; while (ta!=ta->t) ta = ta->t;
    Thread tb = e->n->t; while (tb!=tb->t) tb = tb->t;
    Thread tc = e->n->n->t; while (tc!=tc->t) tc = tc->t;

    Sample sa = arrBack(ta->samples);
    Sample sb = arrBack(tb->samples);
    Sample sc = arrBack(tc->samples);

//  Vec3 n;

//  vScale(&sa->n, -1, &n);
//  glNormal3v(n);
    glNormal3v(sa->n);
    glVertex3v(sa->p);
//  vScale(&sc->n, -1, &n);
//  glNormal3v(n);
    glNormal3v(sc->n);
    glVertex3v(sc->p);
//  vScale(&sb->n, -1, &n);
//  glNormal3v(n);
    glNormal3v(sb->n);
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
    glLineWidth(1.0);

    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0);

    pthread_mutex_lock(&sp->mutex);

    arrForEach(sp->thr, display_thread, 0);

    glEnable(GL_LIGHTING);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);
    arrForEach(sp->fringe, display_edge, 0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glDisable(GL_LIGHTING);
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
