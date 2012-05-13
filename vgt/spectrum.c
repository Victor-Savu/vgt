
#include <vgt/spectrum.h>
#include <vgt/spectrum_cls.h>

#include <math/obj.h>
#include <math/vertex.h>
#include <math/algorithms.h>
#include <math/roots3and4.h>

#include <ads/array.h>
#include <ads/vector.h>

#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/volumetric_data.h>
#include <vgt/volumetric_data_cls.h>
#include <vgt/scalar_field_cls.h>
#include <vgt/topology.h>
#include <vgt/iso.h>
#include <vgt/delaunay.h>
#include <vgt/delaunay_cls.h>
#include <vgt/tet.h>
#include <vgt/tet_cls.h>

#include <GL/glut.h>
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
void print_sample(const Sample restrict s, FILE* f)
{
    fprintf(f, "p="); vPrint(&s->p, f);
    fprintf(f, " n="); vPrint(&s->n, f);
    fprintf(f, " iso=%lf", (double)s->iso);
}

inline static
void snap_sample(Sample restrict s, VolumetricData restrict vol, real snap_iso, real snap_iso_thr)
{
   // call;
    check(sfInside(vol->scal, s->p[0], s->p[1], s->p[2]));
    // the real isovalue
    real iso = sfValue(vol->scal, s->p[0], s->p[1], s->p[2]);
    // the distance to the isosurface (in normalized isovalue space)
    real d_iso = snap_iso - iso;

    Normal dn;
    Vertex new_p;
    ignore vfValue(vol->grad, &s->n, s->p[0], s->p[1], s->p[2]);
    vScale((const Vec3*)&s->n, 0.1 * d_iso/vNormSquared((const Vec3*)&s->n), &dn);
    while (algoAbs(d_iso) > snap_iso_thr) {
        vAdd(&dn, &s->p, &new_p);
        if (sfInside(vol->scal, new_p[0], new_p[1], new_p[2])) iso = sfValue(vol->scal, new_p[0], new_p[1], new_p[2]);
        int32_t iterations = 5;
        while (!sfInside(vol->scal, new_p[0], new_p[1], new_p[2]) || (algoAbs(d_iso) <= algoAbs(snap_iso - iso) && iterations--)) {
            vScaleI(&dn, 0.5);
            vAdd(&dn, &s->p, &new_p);
            if (sfInside(vol->scal, new_p[0], new_p[1], new_p[2])) iso = sfValue(vol->scal, new_p[0], new_p[1], new_p[2]);
        }
        iterations = 100;
        while (algoAbs(d_iso) <= algoAbs(snap_iso - iso) && --iterations) {
            dn[0] = algoRandomDouble(-0.05 / iterations, 0.05 / iterations);
            dn[1] = algoRandomDouble(-0.05 / iterations, 0.05 / iterations);
            dn[2] = algoRandomDouble(-0.05 / iterations, 0.05 / iterations);
            vAdd(&dn, &s->p, &new_p);
            if (sfInside(vol->scal, new_p[0], new_p[1], new_p[2])) iso = sfValue(vol->scal, new_p[0], new_p[1], new_p[2]);
        }
        if (iterations <= 0) {
            fprintf(stderr, "Not snapped: "); print_sample(s, stderr); fprintf(stderr, "\n");
            break;
            check(0);
        }
        d_iso = snap_iso - iso;
        vCopy(&new_p, &s->p);
        s->iso = iso;
        ignore vfValue(vol->grad, &s->n, s->p[0], s->p[1], s->p[2]);
        vScale((const Vec3*)&s->n, 0.1 * d_iso/vNormSquared((const Vec3*)&s->n), &dn);
    }
    //usleep(5000000);
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
        check(e->o->o == e);
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
void clear_edge_att(uint64_t i, Obj o, Obj d)
{
    HalfEdge e = o;
    e->att = 0;
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
    do {
        Thread* t = o;
        if (!(*t)->active) {
            *t = *oCast(Thread*, arrBack(d));
            arrPop(d);
        } else break;
    } while (i < arrSize(d));
}

inline static
void remove_marked_edges(uint64_t i, Obj o, Obj d)
{
    while (i < arrSize(d)) {
        HalfEdge a = o;
        HalfEdge bk = arrBack(d);
        check(bk->o != bk);
        check(bk->n != bk);
        while (bk->att && i < arrSize(d)) {
            arrPop(d);
            bk = arrBack(d);
        }
        if (i < arrSize(d) && a->att) {

            if (a!= bk) {
                oCopyTo(a, bk, sizeof(struct HalfEdge));
                memset(bk, 0, sizeof (struct HalfEdge));
                if (a->o) a->o->o = a;
                a->n->n->n = a;
            }
            arrPop(d);

        } else break;
    }
}

inline static
void snap_thread_endpoint(uint64_t i, Obj o, Obj d)
{
    Thread* t = o;
    Spectrum sp = d;
    snap_sample(arrBack((*t)->samples), sp->vol, sp->snap_iso, sp->snap_iso_thr);
}

inline
void specSnap(Spectrum restrict sp)
{
    call;
    arrForEach(sp->active_threads, snap_thread_endpoint, sp);
}

inline static
int cmp_crit(const void* a, const void* b)
{
    const struct CriticalPoint* c1 = a;
    const struct CriticalPoint* c2 = b;

    if (c1->isovalue < c2->isovalue) return -1;
    if (c1->isovalue > c2->isovalue) return 1;
    return 0;
}

Spectrum specCreate(const char* restrict conf)
{
    call;

    usage(conf);

    FILE* fin = fopen(conf, "r");

    conjecture(fin, "Error opening file.");

    VolumetricData v = oCreate(sizeof (struct VolumetricData));
    vdRead(v, fin, conf);

    char off_file_name[1024];
    float initial_isovalue = 0.0;
    char line[1024];

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

    CriticalPoint cp = v->topology.criticalities = oCreate(v->topology.size * sizeof (struct CriticalPoint));

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

        vSet(&smpl.p, x+0.2, y+0.2, z+0.2);
*/
        vSet(&smpl.p, x, y, z);

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
        .snap_iso = initial_isovalue,
        .snap_iso_thr = 5e-4,
        .area_sqr = 0.04,
        .lambda = 0.04,
        .ref_norm_thr = 0.8,
        .min_sampling_iso_distance = 4e-3, // slightly smaller than 2./255.
      //  .min_sampling_iso_distance = 1e-3, // slightly smaller than 0.25/255.
        .min_crit_iso_distance = 8e-4, // 5 times smaller than the maximum sampling iso distance
        .isosamples = arrCreate(sizeof(real), 1),
    };

    usage(v->topology.size > 1);

    // computing the sampling isovalues
    qsort(v->topology.criticalities, v->topology.size, sizeof(struct CriticalPoint), cmp_crit);

    usage(v->topology.criticalities[0].isovalue < initial_isovalue);
    usage(v->topology.criticalities[v->topology.size-1].isovalue > initial_isovalue);

    // compute the samples
    cp = v->topology.criticalities;
    for (i=1; i < v->topology.size; i++) {
        real d_iso = cp[i].isovalue - cp[i-1].isovalue;
        if (d_iso > sp.min_crit_iso_distance) {
            real crt = cp[i-1].isovalue;
            uint64_t n_smpl = floor(d_iso / sp.min_sampling_iso_distance);
            d_iso = d_iso / n_smpl;
            crt += d_iso/2;
            while (n_smpl--) {
                arrPush(sp.isosamples, &crt);
                crt += d_iso;
            }
        }
    }
/*
    arrPrint(sp.isosamples, stdout, oRealPrint);

    fprintf(stderr, "Min: %u\n", oCast(unsigned int, *sfMin(v->scal) * 255u));
    fprintf(stderr, "Max: %u\n", oCast(unsigned int, *sfMax(v->scal) * 255u));
*/
    Spectrum ret = oCopy(&sp, sizeof (struct Spectrum));

    pthread_mutex_init(&ret->mutex, 0);

    return ret;
}

struct interp_kit {
    real ierror;
    real nerror;
    Spectrum sp;
    uint32_t maxdepth;
};

inline static
void interp_error(uint64_t i, Obj o, Obj d)
{
    struct interp_kit* kit = d;
    Thread* t = o;
    conjecture ((*t)->t == *t, "Not the representing thread.");

    if (kit->maxdepth < (*t)->depth) kit->maxdepth = (*t)->depth;

    Sample s = arrBack((*t)->samples);

    kit->ierror += algoAbs(s->iso - sfValue(kit->sp->vol->scal, s->p[0], s->p[1], s->p[2]));
    Normal n; vCopy(&s->n, &n);
    Normal in; ignore vfValue(kit->sp->vol->grad, &in, s->p[0], s->p[1], s->p[2]);

    Normal c;
    //kit->nerror += vNorm((const Vec3*)vCross(vNormalizeI(&n), vNormalizeI(&in), &c));
    kit->nerror += vNorm((const Vec3*)vCross(&n, &in, &c));
/*
    if (vNormSquared((const Normal*)&s->n) > 0.05) {
        fprintf(stderr, "%lu:", i);
        print_sample(s, stderr);
        fprintf(stderr, "\n");
    }
*/
}

inline static
void interp_bar_error(uint64_t i, Obj o, Obj d)
{
    struct interp_kit* kit = d;
    HalfEdge e = o;

    Thread a = e->t; while (a != a->t) a = a->t;
    Thread b = e->n->t; while (b != b->t) b = b->t;
    Thread c = e->n->n->t; while (c != c->t) c = c->t;

    if (e->o) {
        Thread to = e->o->t; while (to != to->t) to = to->t;
        check(to == b);
    }

    Sample sa = arrBack(a->samples);
    Sample sb = arrBack(b->samples);
    Sample sc = arrBack(c->samples);
    Vertex g;
    vAdd(&sa->p, &sb->p, &g);
    vAddI(&g, (const Vec3*)&sc->p);
    vScaleI(&g, 1./3.);

    real iso = (sa->iso + sb->iso + sc->iso)/3.;

    kit->ierror += algoAbs(iso - sfValue(kit->sp->vol->scal, g[0], g[1], g[2]));

}

inline
void specStats(Spectrum restrict s, FILE* f)
{
    //call;
    struct interp_kit kit = { .ierror = 0.0, .nerror = 0.0, .sp = s };
    arrForEach(s->active_threads, interp_error, &kit);
    arrForEach(s->fringe, interp_bar_error, &kit);
    /*
    Normal n;
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.421, 66.270, 89.500), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.421, 66.270, 89.501), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.421, 66.271, 89.500), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.421, 66.271, 89.501), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.422, 66.270, 89.500), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.422, 66.270, 89.501), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.422, 66.271, 89.500), stderr); fprintf(stderr, "\n");
    fprintf(stderr, "Weird: "); vPrint(vfValue(s->vol->grad, &n, 103.422, 66.271, 89.501), stderr); fprintf(stderr, "\n");
    */
    fprintf(f, "Bounding box size: <%lf, %lf, %lf>\n", s->vol->scal->nx * s->vol->scal->dx, s->vol->scal->ny * s->vol->scal->dy, s->vol->scal->nz * s->vol->scal->dz);
    fprintf(f, "Min/Max values: <%lf, %lf>\n", (double)*sfMin(s->vol->scal) * 255, (double)*sfMax(s->vol->scal) * 255);
    fprintf(f, "Samples: %lu\n", arrSize(s->active_threads));
    fprintf(f, "Triangles: %lu\n", arrSize(s->fringe)/3);
    fprintf(f, "Interp error: %lf\n", oCast(double, kit.ierror)/(arrSize(s->active_threads)+arrSize(s->fringe)/3));
    fprintf(f, "Snap isovalue: %lf\n", oCast(double, s->snap_iso) * 255);
    fprintf(f, "Triangloids: %lu\n", arrSize(s->tri));
    fprintf(f, "Max thread depth: %u\n", kit.maxdepth);
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
    arrDestroy(sp->isosamples);
    delDestroy(sp->del);


    arrDestroy(sp->extracted_edges);
    arrDestroy(sp->extracted_threads);

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

    vAddI(p, (const Vec3*)&(*t)->force_t);
    vAddI(p, (const Vec3*)&(*t)->force_n);

    vSet(&(*t)->force_t, 0, 0, 0);
    vSet(&(*t)->force_n, 0, 0, 0);
}
/*
inline static
void compute_triangles(uint64_t i, Obj o, Obj d)
{
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
    vScaleI(vAddI(vAdd(pa, pb, &tr.g), (const Vec3*)pc), 1./3.);
    Vec3 ab, ac;
    tr.AA16 = vNormSquared((const Vec3*)vCrossI(vSub(pb, pa, &ab), vSub(pc, pa, &ac)));

//    check(tr.AA16 > 1e-3);

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

    tr.r4 = tr.AA16/27;

    ha->att = hb->att = hc->att = arrPush(d, &tr);
}
    */

inline static
void compute_idealizing_force(uint64_t i, Obj o, Obj d)
{
    unused(d);
    unused(i);
    HalfEdge e = o;

    //if (e > e->n || e > e->n->n) return;

    Thread t = e->t; while (t != t->t) t = t->t;
    Thread tn = e->n->t; while (tn != tn->t) tn = tn->t;

    if (t->id < tn->id) return;

    Vec3 aux; vSub(&tn->relaxed.p, &t->relaxed.p, &aux);

    vAddI(&t->force_t, (const Vec3*)&aux);
    vSubI(&tn->force_t, &aux);
}
/*
inline static
void compute_idealizing_force(uint64_t i, Obj o, Obj d)
{
    unused(d);
    unused(i);
    HalfEdge e = o;

    //if (e > e->n || e > e->n->n) return;

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
    vAddI(fc, (const Vec3*)&l);  vScaleI(&l, 0.5);  vSubI(fa, &l); vSubI(fb, &l);

    vSubI(vScaleI(vAdd(pc, pb, &m), 0.5), pa);
    vNormalizeI(vSub(pc, pb, &l));
    vScaleI(&l, vDot(&m, &l)/2);
    vAddI(fa, (const Vec3*)&l);  vScaleI(&l, 0.5);  vSubI(fb, &l); vSubI(fc, &l);

    vSubI(vScaleI(vAdd(pa, pc, &m), 0.5), pb);
    vNormalizeI(vSub(pa, pc, &l));
    vScaleI(&l, vDot(&m, &l)/2);
    vAddI(fb, (const Vec3*)&l);  vScaleI(&l, 0.5);  vSubI(fc, &l); vSubI(fa, &l);

    // centripet force
    Triangle t = e->att;
    vNormalizeI(vCrossI(&m, &l)); // m is now parallel to the triangle's normal
    vAddI(vScaleI(&m, vDot(&m, vSub(pa, &t->g, &l))), &t->g); // m is the projection of e->g on the triangle

    real d2;
    d2 = vNormSquared(vSub(pa, &m, &l)); vAddI(fa, vScaleI(&l, t->r4/d2 - d2));
    d2 = vNormSquared(vSub(pb, &m, &l)); vAddI(fb, vScaleI(&l, t->r4/d2 - d2));
    d2 = vNormSquared(vSub(pc, &m, &l)); vAddI(fc, vScaleI(&l, t->r4/d2 - d2));
}
*/

inline static
void extract_tangent_force(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    Sample s = &t->relaxed;
    Vec3 n;

    if (vNormSquared((const Vec3*)&s->n) == 0) {
        Sample st = arrBack(t->samples);
        fprintf(stderr, "Has zero normal: relaxed:"); vPrint(&s->p, stderr); fprintf(stderr, " initial:"); vPrint(&st->p, stderr); fprintf(stderr, "\n"); fflush(stderr);
    }
    vNormalize((const Vec3*)&s->n, &n);
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

    sp->current_stress += vNormSquared((const Vec3*)&t->force_t);
    /*
       Sample s = &t->relaxed;
       real diso = algoAbs(s->iso - sfValue(sp->vol->scal, s->p[0], s->p[1], s->p[2]));
       sp->current_stress += vNormSquared(&s->n) * diso * diso;
     */
}


inline static
void compute_normal_force(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread t = *oCast(Thread*, o);
    check(t = t->t);
    check(t->active);
    Sample s = &t->relaxed;
    Spectrum sp = d;
    snap_sample(s, sp->vol, sp->snap_iso, sp->snap_iso_thr);

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
    //call;

    sp->scale = 1;
    sp->scale_threshold = 1e-2;

    //Array active_triangles = arrCreate(sizeof(struct Triangle), 4);


    // pre-compute the barycenter and the radius of the ideal triangle
    // debug(fprintf(stderr, "Precomputing barycenter.\n"); fflush(stderr););
    //arrForEach(sp->fringe, compute_triangles, active_triangles);

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
            sp->scale *= 0.8;
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
            if (sp->current_stress > algoRandomDouble(0.8, 1.1) * sp->previous_stress)
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
            sp->scale *= 1.1;
        } else {
            relaxing = false;
        }

    }

    //arrForEach(sp->fringe, clear_edge_att, 0);
    //arrDestroy(active_triangles);
}

inline static
void simplify(uint64_t ind, Obj o, Obj d)
{
    HalfEdge a = o;  check(a);
    if (a->att) {
    //    fprintf(stderr, "Skip\n");
        return;
    }
    HalfEdge b = a->n; check(b);
    if (b->att) check(0);
    HalfEdge c = b->n; check(c);
    if (c->att) check(0);

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
    real l_ab = vNorm((const Vec3*)vSub(&sb->p, &sa->p, &ab));
    real l_bc = vNorm((const Vec3*)vSub(&sc->p, &sb->p, &bc));
    if (l_ab > l_bc) return;
    real l_ca = vNorm((const Vec3*)vSub(&sa->p, &sc->p, &ca));
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

    // simplifying this triangle is desirable if manifold prezervation is possible

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


    //alternate begin

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
        if (algoAbs(tria.iso[0] - tria.iso[1]) > eps) arrPush(sp->tri, &tria);
    }

    if (b->o) b->o->o = c->o;
    if (c->o) c->o->o = b->o;
    b->o = c->o = 0;
    a->att = b->att = c->att = oCast(Obj, 1);
    if (a->o) {
        a = a->o;
        a->o->o = 0;
        a->o = 0;

        b = a->n; c = b->n;
        if (b->o) b->o->o = c->o;
        if (c->o) c->o->o = b->o;
        b->o = c->o = 0;
        a->att = b->att = c->att = oCast(Obj, 1);
    }
    pthread_mutex_unlock(&sp->mutex);
    return;

    check(0);
    //alternate end
/*
    // save a pointer to the opposite edge
    HalfEdge opp = a->o; opp->o = opp;
    if (b->o) b->o->o = c->o;
    if (c->o) c->o->o = b->o;
    a->o = a->n = a;
    b->o = b->n = b;
    c->o = c->n = c;


    ignore vScaleI(vAddI(&sb->p, &sa->p), 0.5);
    snap_sample(sb, sp->vol, sp->snap_iso, sp->snap_iso_thr);
    vCopy(&sb->p, &sa->p);
    vCopy(&sb->n, &sa->n);
    sa->iso = sb->iso;

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
        if (a->o && a->o != bk) a->o->o = a;
        if (a->n != bk)
            a->n->n->n = a;
        else
            a->n = a->o = a;
    }
    arrPop(sp->fringe);

    bk = arrBack(sp->fringe);
    if (b!= bk) {
        if (bk == opp) opp = b;
        oCopyTo(b, bk, sizeof(struct HalfEdge));
        memset(bk, 0, sizeof (struct HalfEdge));
        if (b->o && b->o != bk) b->o->o = b;
        if(b->n != bk)
            b->n->n->n = b;
        else
            b->n = b->o = b;
    }
    arrPop(sp->fringe);

    bk = arrBack(sp->fringe);
    if (c!= bk) {
        if (bk == opp) opp = c;
        oCopyTo(c, bk, sizeof(struct HalfEdge));
        memset(bk, 0, sizeof (struct HalfEdge));
        if (c->o && c->o != bk) c->o->o = c;
        if (c->n != bk)
            c->n->n->n = c;
        else
            c->n = c->o = c;
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
            if (a->o && a->o != bk) a->o->o = a;
            if (a->n != bk)
                a->n->n->n = a;
            else
                a->n = a->o = a;
        }
        arrPop(sp->fringe);

        bk = arrBack(sp->fringe);
        if (b!= bk) {
            oCopyTo(b, bk, sizeof(struct HalfEdge));
            memset(bk, 0, sizeof (struct HalfEdge));
            if (b->o && b->o != bk) b->o->o = b;
            if (b->n != bk)
                b->n->n->n = b;
            else
                b->n = b->o = b;
        }
        arrPop(sp->fringe);

        bk = arrBack(sp->fringe);
        if (c!= bk) {
            oCopyTo(c, bk, sizeof(struct HalfEdge));
            memset(bk, 0, sizeof (struct HalfEdge));
            if (c->o && c->o != bk) c->o->o = c;
            if (c->n != bk)
                c->n->n->n = c;
            else
                c->n = c->o = c;
        }
        arrPop(sp->fringe);
    }

    pthread_mutex_unlock(&sp->mutex);
*/
}

void specSimplify(Spectrum restrict sp)
{
    //call;
    uint64_t before = 0;
    uint64_t after = arrSize(sp->fringe);

    while (before!= after) {
        before = after;
        after = 0;
        arrForEach(sp->fringe, simplify, sp);
        //fprintf(stderr, "Loop!\n"); fflush(stderr);

        pthread_mutex_lock(&sp->mutex);
        arrForEach(sp->fringe, remove_marked_edges, sp->fringe);
        arrForEach(sp->active_threads, remove_inactive_threads, sp->active_threads);
        pthread_mutex_unlock(&sp->mutex);

        after = arrSize(sp->fringe);
    }
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

        // check if the edge is long enough to be split
        struct Sample sm;
        real l = vNormSquared((const Vec3*) vSub(&s->p, &sn->p, &sm.p));
        if ( l < 0.04) {
            // it does not have to be split
            e->att = 0;
            HalfEdge* bk = arrBack(ref->q);
            if (bk != pe) oCopyTo(pe, bk, sizeof (HalfEdge));
            arrPop(ref->q);
            continue;
        }

        // compute midpoint sample
        vScaleI(vAdd(&s->p, &sn->p, &sm.p), .5);
        vScaleI(vAdd(&s->n, &sn->n, &sm.n), .5);
        sm.iso = (s->iso + sn->iso)/2;

        // get the real midpoint values
        Normal n; vfValue(ref->sp->vol->grad, &n, sm.p[0], sm.p[1], sm.p[2]);
        real iso = sfValue(ref->sp->vol->scal, sm.p[0], sm.p[1], sm.p[2]);

        //
        real d_n = vDot(&n, &sm.n); d_n *= algoAbs(d_n)/vNormSquared((const Normal*)&n)/vNormSquared((const Normal*)&s->n);
        real d_iso = sm.iso - iso; d_iso *= d_iso;

        //if (    (algoAbs(iso - sm.iso) < 1.5 * ref->sp->snap_iso_thr) &&
        if (    (d_iso / l / vNormSquared((const Normal*)&n) < 4e-2) &&
                 d_n > ref->sp->ref_norm_thr ) {
            // it does not have to be split
            e->att = 0;
            HalfEdge* bk = arrBack(ref->q);
            if (bk != pe) oCopyTo(pe, bk, sizeof (HalfEdge));
            arrPop(ref->q);
            continue;
        } else {
            // it needs to be split
            // fprintf(stderr, "Before: "); vPrint(&sm.p, stderr); fprintf(stderr, "  After: "); vPrint(&sm.p, stderr); fprintf(stderr, "\n");
            //if (i % 10 == 0)
            //  fprintf(stderr, "Split %lu: <%lf, %lf> <%lf, %lf>\n", i, (double) algoAbs(iso - sm.iso), (double) ref->sp->snap_iso_thr, (double) vDot(&n, &sm.n), (double) ref->sp->ref_norm_thr);

            snap_sample(&sm, ref->sp->vol, ref->sp->snap_iso, 0.5 * ref->sp->snap_iso_thr);
/*
            if (vNormSquared((const Normal*)&sm.n) > 0.05) {
                Normal n;
                fprintf(stderr, "Freshman %lu: ", arrSize(ref->sp->thr));
                print_sample(&sm, stderr);
                fprintf(stderr, "Real normal: ");
                vPrint(vfValue(ref->sp->vol->grad, &n, sm.p[0], sm.p[1], sm.p[2]), stderr);
                fprintf(stderr, "\n");
            }
*/
            struct Thread thr = {
                .samples = arrCreate(sizeof(struct Sample), 1),
                .id = arrSize(ref->sp->thr),
                .t = 0,
                .depth = 0,
                .iso = 0,
                .force_t = VERT_0,
                .force_n = VERT_0,
                .active = 0
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

    if ((e->n->att) && (!e->n->n->att)) e = e->n->n;
    else if ((!e->n->att) && (e->n->n->att)) e = e->n;
    else return;

    // it needs to be split
    Thread t = e->t; while (t->t != t) t = t->t;
    Thread tn = e->n->t; while (tn->t != tn) tn = tn->t;

    Sample s = arrBack(t->samples);
    Sample sn = arrBack(tn->samples);

    // compute midpoint sample
    struct Sample sm;
    vScaleI(vAdd(&s->p, &sn->p, &sm.p), .5);
    vScaleI(vAdd(&s->n, &sn->n, &sm.n), .5);
    sm.iso = (s->iso + sn->iso)/2;

    snap_sample(&sm, ref->sp->vol, ref->sp->snap_iso, 0.5 * ref->sp->snap_iso_thr);

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
    check((*oCast(HalfEdge*, arrBack(ref->q)))->t->id == e->t->id);
    if (e->o) {
        e->o->att = e->att;
        arrPush(ref->q, &e->o);
        check((*oCast(HalfEdge*, arrBack(ref->q)))->t->id == e->o->t->id);
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
    a->att = b->att = 0;
    a_->t->active = true;
}

inline static
Triangloid storeTriangloid(HalfEdge a, Spectrum sp)
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
    if (algoAbs(tria.iso[0] - tria.iso[1]) > sp->snap_iso_thr)
        return arrPush(sp->tri, &tria);
    else
        return 0;
}

inline static
void split_case_1(HalfEdge a, struct refinement_kit* ref)
{
    ignore storeTriangloid(a, ref->sp);
    HalfEdge a1, a2, a3;
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a1 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->n->n->t, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a2 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a3 = arrPush(ref->sp->fringe, &tmp);
    }
    a1->n = a->n->n; a1->o = a2;
    a2->n = a3; a2->o = a1;
    a3->n = a->n;

    a->n->n = a2; a->n = a1; a->att = a3;
    split_match(a);
}

inline static
void split_case_2(HalfEdge a, struct refinement_kit* ref)
{
    HalfEdge b = a->n;
    HalfEdge c = b->n;

    ignore storeTriangloid(a, ref->sp);
    HalfEdge a1, a2, a3;
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a1 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = c->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a2 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        a3 = arrPush(ref->sp->fringe, &tmp);
    }

    HalfEdge b1, b2, b3;
    {
        struct HalfEdge tmp = { .t = b->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        b1 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = a->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        b2 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = b->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        b3 = arrPush(ref->sp->fringe, &tmp);
    }

    HalfEdge c1, c2, c3;
    {
        struct HalfEdge tmp = { .t = c->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        c1 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = b->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        c2 = arrPush(ref->sp->fringe, &tmp);
    }
    {
        struct HalfEdge tmp = { .t = c->att, .n = 0, .o = 0, .iso = ref->sp->snap_iso, .att = 0 };
        c3 = arrPush(ref->sp->fringe, &tmp);
    }

    a->n = a1;
    b->n = b1;
    c->n = c1;

    a1->n = c3; a1->o = a2;
    b1->n = a3; b1->o = b2;
    c1->n = b3; c1->o = c2;

    a2->n = b2; a2->o = a1;
    b2->n = c2; b2->o = b1;
    c2->n = a2; c2->o = c1;

    a3->n = b;
    b3->n = c;
    c3->n = a;

    a->att = a3;
    b->att = b3;
    c->att = c3;

    split_match(a);
    split_match(b);
    split_match(c);
}

inline static
void stage45(uint64_t i, Obj o, Obj d)
{
    HalfEdge* pe = o;
    HalfEdge e = *pe;

    // it was already split and matched
    if (!e->att) return;

    // it was split, but not yet matched
    if (e->n->o && (e->n->o->n == e->att || (e->n->o->n->o && e->n->o->n->o->n == e->att))) return;
/*
    // check if it is still marked
    if (e->att == 0 || e->att == e->n->t) return;

    conjecture((e->n->att == e->n->n->t) == (e->n->n->att == e->t), "The triangle is neither in case 1 nor in case 2.");


    if ((e->n->att == e->n->n->t) && (e->n->n->att == e->t)) split_case_1(e, d);
    else split_case_2(e, d);
*/
    if (e->n->att && e->n->n->att) split_case_2(e, d);
    else if (!e->n->att && !e->n->n->att) split_case_1(e, d);
    else conjecture(0, "The triangle is neither in case 1 nor in case 2.");
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

/*
inline static
void check_stage_23(uint64_t i, Obj o, Obj d)
{
    HalfEdge e = o;
    i = 0;
    if (e->att) i++;
    if (e->n->att) i++;
    if (e->n->n->att) i++;
    conjecture(i==0 || i==1 || i==3, "Nooohh!");

}

inline static
void check_stage_1(uint64_t i, Obj o, Obj d)
{
    HalfEdge e = o;
    if (e->att) {
        Thread t = e->att;
        Sample s = arrBack(t->samples);
        Thread t1 = e->t; while (t1 != t1->t) t1 = t1->t;
        Thread t2 = e->n->t; while (t2 != t2->t) t2 = t2->t;
        Sample s1 = arrBack(t1->samples);
        Sample s2 = arrBack(t2->samples);
        Vec3 v; vScaleI(vAdd(&s1->p, &s2->p, &v), 0.5);
        vSubI(&v, &s->p);
        if (vNorm(&v) > 2e-1) {
            fprintf(stderr, "Norm : %lf\n", vNorm(&v));
        }
    }
}
*/

void specRefine(Spectrum restrict sp)
{
    //call;
    struct refinement_kit ref_kit = {
        .sp = sp,
        .q = 0,
    };

    Array q1 = arrRefsArr(sp->fringe);

    uint64_t iterations = 10;

    while (!arrIsEmpty(q1) && iterations--) {
        //fprintf(stderr, "Spuff %lu!\n", arrSize(q1)); fflush(stderr);
        // stage #1
        ref_kit.q = q1;
        arrForEach(q1, stage1, &ref_kit);

//        arrForEach(sp->fringe, check_stage_1, sp);

        // stage #2
        Array q2 = ref_kit.q = arrCreate(sizeof (HalfEdge), 1);
        arrForEach(q1, stage23, &ref_kit);

        // stage #3
        arrForEach(q2, stage23, &ref_kit);

//        arrForEach(sp->fringe, check_stage_23, sp);

    //    usleep(1000000);

        pthread_mutex_lock(&sp->mutex);
        // stage #4
        Array q3 = ref_kit.q = arrCreate(sizeof (HalfEdge), 1);
        arrForEach(q1, stage45, &ref_kit);

        // stage #5
        arrForEach(q2, stage45, &ref_kit);
        pthread_mutex_unlock(&sp->mutex);

//        arrForEach(sp->fringe, check_stage_1, sp);

   //     usleep(3000000);

        // stage #6
        arrForEach(q1, stage6, &ref_kit);

        arrDestroy(q1);
        q1 = q3;
        arrDestroy(q2);
    }

    arrDestroy(q1);
}

/*
real rayVoxelIntersection(const Vec3* orig, const Vec3* dir, real isoValue, unsigned int x, unsigned int y, unsigned int z, Vec3* res, ScalarField sf) {

    int x0 = x, x1 = x + 1, y0 = y, y1 = y + 1, z0 = z, z1 = z + 1;
    real p[8];

    real* e = sfAt(sf, x, y, z); p[0] = *e;
    e = sfRelX(sf, e,  1); p[1] = *e;
    e = sfRelY(sf, e,  1); p[3] = *e;
    e = sfRelX(sf, e, -1); p[2] = *e;
    e = sfRelZ(sf, e,  1); p[6] = *e;
    e = sfRelX(sf, e,  1); p[7] = *e;
    e = sfRelY(sf, e, -1); p[5] = *e;
    e = sfRelX(sf, e, -1); p[4] = *e;

    real ua[2], va[2], wa[2];
    real ub[2], vb[2], wb[2];

    ua[0] = (x1 - (*orig)[0]);
    va[0] = (y1 - (*orig)[1]);
    wa[0] = (z1 - (*orig)[2]);

    ub[0] = -(*dir)[0];
    vb[0] = -(*dir)[1];
    wb[0] = -(*dir)[2];

    ua[1] = ((*orig)[0] - x0);
    va[1] = ((*orig)[1] - y0);
    wa[1] = ((*orig)[2] - z0);

    ub[1] = (*dir)[0];
    vb[1] = (*dir)[1];
    wb[1] = (*dir)[2];


    real A = 0.0f;
    real B = 0.0f;
    real C = 0.0f;
    real D = 0.0f;

    int i,j,k;
    for (i = 0; i <= 1; i++) {
        for (j = 0; j <= 1; j++) {
            for (k = 0; k <= 1; k++) {
                real pval = p[k * 4 + j * 2 + i];
                A += ub[i] * vb[j] * wb[k] * pval;

                B += (ua[i] * vb[j] * wb[k] + ub[i] * va[j] * wb[k] + ub[i] * vb[j] * wa[k]) * pval;

                C += (ub[i] * va[j] * wa[k] + ua[i] * vb[j] * wa[k] + ua[i] * va[j] * wb[k]) * pval;

                D += ua[i] * va[j] * wa[k] * pval;
            }
        }
    }

    D -= isoValue;

    double c[4];
    c[0] = D;
    c[1] = C;
    c[2] = B;
    c[3] = A;

    double s[3];

    real tmin = 1e11;
    //real eps = 1e-5;

    int numRoots = SolveCubic(c, s);
    int r_ind;
    for (r_ind = 0; r_ind < numRoots; r_ind++) {
        if (fabs(s[r_ind]) < fabs(tmin)) {
            Vec3 tmp; vAddI(vScale(dir, s[r_ind], &tmp), (const Vec3*)orig);
            if (tmp[0] >= x0 - eps && tmp[0] <= x1 + eps && tmp[1] >= y0 - eps && tmp[1] <= y1 + eps && tmp[2] >= z0 - eps && tmp[2] <= z1 + eps) {
                vCopy(&tmp, res);
                tmin = algoAbs(s[r_ind]);
            }
        }
    }

    return tmin;
}

inline static
void project_sample(Sample s, VolumetricData vol, real iso)
{
    // determine the cell in which the sample point is
    int64_t i = s->p[0] / vol->scal->dx;
    int64_t j = s->p[1] / vol->scal->dy;
    int64_t k = s->p[2] / vol->scal->dz;
    check(i > 0);
    check(j > 0);
    check(k > 0);
    check(i < vol->scal->nx);
    check(j < vol->scal->ny);
    check(k < vol->scal->nz);
}

struct projection_kit { Spectrum sp; real iso;};

inline static
void project_thread(uint64_t i, Obj o, Obj d)
{
    Thread* pt = o;
    Thread t = *pt;
    struct projection_kit* kit = d;
    Spectrum sp = kit->sp;
    check(t->t == t);

    Sample s = arrBack(t->samples);
    s = arrPush(t->samples, s);

   // project_sample(s, sp->vol, kit->iso);

    Vec3i dimensions = {sp->vol->scal->nx, sp->vol->scal->ny, sp->vol->scal->nz};
    Vec3 spacing = {sp->vol->scal->dx, sp->vol->scal->dy, sp->vol->scal->dz};

    int steps = 0;
    float dist = 1e11;
    const int MAX_STEPS = 8;

    Vec3 closestPoint; vCopy(&s->p, &closestPoint);
    Vec3 tmp_res;

    // initialization
    Vec3i gridPosUp = {(uint) (s->p[0]), (uint) (s->p[1]), (uint) (s->p[2])};
    Vec3i gridPosDown = {(uint) (s->p[0]), (uint) (s->p[1]), (uint) (s->p[2])};

    Vec3i stepUp;
    Vec3i stepDown;

    Vec3 delta = {spacing[0] / algoAbs(s->n[0]), spacing[1] / algoAbs(s->n[1]), spacing[2] / algoAbs(s->n[2])};

    Vec3 maxUp, maxDown;

    for (i = 0; i < 3; i++) {
        if (s->n[i] < 0) {
            stepUp[i] = -1;
            stepDown[i] = 1;

            if (s->n[i] != 0) {
                maxUp[i] = (gridPosUp[i] - s->p[i]) / s->n[i];
                maxDown[i] = (gridPosDown[i] + 1 - s->p[i]) / (-s->n[i]);
            } else {
                maxUp[i] = 1e11;
                maxDown[i] = 1e11;
            }
        } else {
            stepUp[i] = 1;
            stepDown[i] = -1;
            if (s->n[i] != 0) {
                maxUp[i] = (gridPosUp[i] + 1 - s->p[i]) / s->n[i];
                maxDown[i] = (gridPosDown[i] - s->p[i]) / (-s->n[i]);
            } else {
                maxUp[i] = 1e11;
                maxDown[i] = 1e11;
            }
        }
    }


    bool outOfBoundUp = false;
    bool outOfBoundDown = false;

    while (dist > 1e10 && steps < MAX_STEPS) {
        real* min_val = sfAt(sp->vol->min, gridPosUp[0], gridPosUp[1], gridPosUp[2]);
        real* max_val = sfAt(sp->vol->max, gridPosUp[0], gridPosUp[1], gridPosUp[2]);
        if (!outOfBoundUp && *min_val <= kit->iso + eps && *max_val >= kit->iso - eps) {
            float tmp_dist = rayVoxelIntersection((const Vec3*)&s->p, (const Vec3*)&s->n, kit->iso, gridPosUp[0], gridPosUp[1], gridPosUp[2], &tmp_res, sp->vol->scal);
            if (tmp_dist < dist) { dist = tmp_dist; vCopy(&tmp_res, &closestPoint);}
        }

        min_val = sfAt(sp->vol->min, gridPosDown[0], gridPosDown[1], gridPosDown[2]);
        max_val = sfAt(sp->vol->max, gridPosDown[0], gridPosDown[1], gridPosDown[2]);
        if (!outOfBoundDown && *min_val <= kit->iso + eps && *max_val >= kit->iso - eps) {
            Vec3 norm_tmp; vScale((const Vec3*)&s->n, -1, &norm_tmp);
            float tmp_dist = rayVoxelIntersection((const Vec3*)&s->p, (const Vec3*)&norm_tmp, kit->iso, gridPosDown[0], gridPosDown[1], gridPosDown[2], &tmp_res, sp->vol->scal);
            if (tmp_dist < dist) { dist = tmp_dist; vCopy(&tmp_res, &closestPoint);}
        }

        // step up

        if (!outOfBoundUp) {
            if (maxUp[0] < maxUp[1]) {
                if (maxUp[0] < maxUp[2]) {
                    gridPosUp[0] += stepUp[0];
                    maxUp[0] += delta[0];
                } else {
                    gridPosUp[2] += stepUp[2];
                    maxUp[2] += delta[2];
                }
            } else {
                if (maxUp[1] < maxUp[2]) {
                    gridPosUp[1] += stepUp[1];
                    maxUp[1] += delta[1];
                } else {
                    gridPosUp[2] += stepUp[2];
                    maxUp[2] += delta[2];
                }
            }
        }


        // step down
        if (!outOfBoundDown) {
            if (maxDown[0] < maxDown[1]) {
                if (maxDown[0] < maxDown[2]) {
                    gridPosDown[0] += stepDown[0];
                    maxDown[0] += delta[0];
                } else {
                    gridPosDown[2] += stepDown[2];
                    maxDown[2] += delta[2];
                }
            } else {
                if (maxDown[1] < maxDown[2]) {
                    gridPosDown[1] += stepDown[1];
                    maxDown[1] += delta[1];
                } else {
                    gridPosDown[2] += stepDown[2];
                    maxDown[2] += delta[2];
                }
            }
        }

        if (gridPosUp[0] < 0 || gridPosUp[0] >= (int) dimensions[0] - 1 ||
                gridPosUp[1] < 0 || gridPosUp[1] >= (int) dimensions[1] - 1 ||
                gridPosUp[2] < 0 || gridPosUp[2] >= (int) dimensions[2] - 1) {
            outOfBoundUp = true;
        }

        if (gridPosDown[0] < 0 || gridPosDown[0] >= (int) dimensions[0] - 1 ||
                gridPosDown[1] < 0 || gridPosDown[1] >= (int) dimensions[1] - 1 ||
                gridPosDown[2] < 0 || gridPosDown[2] >= (int) dimensions[2] - 1) {
            outOfBoundDown = true;
        }

        steps++;
    }

    if (dist > 1e10) {
        vCopy(&s->p, &closestPoint);
    }

    vCopy(&closestPoint, &s->p);
}
*/

struct projection_kit {
    Spectrum sp;
    real iso;    // the next isovalue to project onto
    real r;     // the radius of a sphere around a critical point
    Array crit; // critical points crossed suringthe projection
    Array unproj; // the threads which were not projected
    Array samples; // the samples from which the delaunay tetrahedrization should be created
    Array threads; // the threads created by marching tets
    Array edges; // the edges created by marching tets
    Array border; // the edges fron the fringe bordering the sampled area
};

inline static
void project_thread(uint64_t i, Obj o, Obj d)
{
    Thread* pt = o;
    Thread t = *pt;
    struct projection_kit* kit = d;
    Spectrum sp = kit->sp;
    check(t->t == t);

    Sample s = arrBack(t->samples);
    Vertex* before = &s->p;
    s = arrPush(t->samples, s);


    snap_sample(s, sp->vol, kit->iso, 0.5 * sp->snap_iso_thr);

    // check if we might be dealing with critical regions
    if (kit->crit) {
        Vec3 b; vSub(&s->p, before, &b);

        // check if the sample might get projected inside the sphere of radius r around a criticality
        uint64_t sz = arrSize(kit->crit);
        Vertex* cp; // position of the critical point
        bool intersects = false;
        for (i=0; i < sz && !intersects; i++) {
            cp = arrGet(kit->crit, i);
            Vec3 a; vSub(cp, before, &a);
            real proj = vDot(&a, &b);

            if (proj <= 0) vCopy(before, &a);
            else if (proj >= vNormSquared((const Vec3*)&b)) vCopy(&s->p, &a);
            else {
                vScale((const Vec3*)&b, proj/vNormSquared((const Vec3*)&b), &a);
                vAddI(&a, (const Vec3*)before);
            }
            vSubI(&a, cp);

            // if it intersects
            if (vNormSquared((const Vec3*)&a) <= kit->r * kit->r) {
                intersects = true;
                /*
                // project the vertex onto the critical isosurface
                vCopy(before, &s->p);
                snap_sample(s, sp->vol, sfValue(sp->vol->scal, (*cp)[0], (*cp)[1], (*cp)[2]), 0.5*sp->snap_iso_thr);
*/
                arrPop(t->samples);
                s = arrBack(t->samples);
                vSet(&s->n, 0, 0, 0);
                arrPush(kit->unproj, pt);
                arrPush(kit->samples, before);
            }

        }
    }

}

inline static
void find_border(uint64_t i, Obj o, Obj d)
{
    HalfEdge a = o;
    HalfEdge b = a->n;
    HalfEdge c = b->n;

    // only check for one edge of the triangle
    if (a > b || a > c) return;

    struct projection_kit* kit = d;

    Thread ta = a->t; while (ta != ta->t) ta = ta->t;
    Thread tb = b->t; while (tb != tb->t) tb = tb->t;
    Thread tc = c->t; while (tc != tc->t) tc = tc->t;

    Sample sa = arrBack(ta->samples);
    Sample sb = arrBack(tb->samples);
    Sample sc = arrBack(tc->samples);

    bool proj[3] = {
        algoAbs(sa->iso - kit->iso) < 2 * algoAbs(kit->sp->snap_iso_thr),
        algoAbs(sb->iso - kit->iso) < 2 * algoAbs(kit->sp->snap_iso_thr),
        algoAbs(sc->iso - kit->iso) < 2 * algoAbs(kit->sp->snap_iso_thr),
    };

    if (proj[0] && proj[1] && !proj[2]) { arrPush(kit->border, &a); a->att = a; arrPush(kit->samples, &sa->p); arrPush(kit->samples, &sb->p);}
    if (proj[0] && !proj[1] && proj[2]) { arrPush(kit->border, &c); c->att = c; arrPush(kit->samples, &sc->p); arrPush(kit->samples, &sa->p);}
    if (!proj[0] && proj[1] && proj[2]) { arrPush(kit->border, &b); b->att = b; arrPush(kit->samples, &sb->p); arrPush(kit->samples, &sc->p);}
}

inline static
void sample_crit(uint64_t i, Obj o, Obj d)
{
    Vertex* c = o;
    struct projection_kit* kit = d;
    arrPush(kit->samples, c);
}

struct sample_kit {
    real r;
    uint64_t n;
    Array s;
};

    inline static
void ball_sample(uint64_t i, Obj o, Obj d)
{
    struct sample_kit* kit = d;
    uint64_t samples = 0;

    while (samples < kit->n) {
        Vec3 displ = {
            algoRandomDouble(-kit->r, kit->r),
            algoRandomDouble(-kit->r, kit->r),
            algoRandomDouble(-kit->r, kit->r)
        };
        if (vNormSquared((const Vec3*)&displ) < kit->r * kit->r) {
            ++samples;
            arrPush(kit->s, vAddI(&displ, o));
        }
    }

}

inline static
void snap_mt_threads(uint64_t i, Obj o, Obj d)
{
    struct projection_kit* kit = d;
    Thread t = o;
    while (t->t != t) t = t->t;
    Sample s = &t->relaxed;
    snap_sample(s, kit->sp->vol, kit->iso, kit->sp->snap_iso_thr);
}

inline static
Thread thread_collapse_mt(Thread t1, Thread t2)
{
    usage(t1);
    usage(t2);

    while (t1->t != t1) t1 = t1->t;
    while (t2->t != t2) t2 = t2->t;

    if (t1==t2) return t1;

    if (t1->depth > t2->depth) {
        t2->t = t1;
        return t1;
    } else if (t1->depth < t2->depth) {
        t1->t = t2;
        return t2;
    } else {
        ++t1->depth;
        t2->t = t1;
        return t1;
    }
}

inline static
void march_tets(uint64_t i, Obj o, Obj d)
{
    Tet t = o;
    struct projection_kit* kit = d;

    // clear the half-edge data
    t->edges[0] = t->edges[1] = t->edges[2] = t->edges[3] = 0;

    // get vertex values
    real val[4];
    bool geq[4];
    int cnt = 0;
    TetVertex vert;
    for (vert = A; vert <= D; ++vert) {
        Vertex* vv = t->v[vert];
        val[vert-A] = sfValue(kit->sp->vol->scal, (*vv)[0], (*vv)[1], (*vv)[2]);
        geq[vert-A] = (val[vert-A] >= kit->iso);
        cnt += geq[vert-A];
    }

    struct HalfEdge edge_tmp = { .t = 0, .n = 0, .o = 0, .iso = kit->iso, .att = 0 };
    struct Thread thread_tmp = {
        .samples = 0,
        .id = 0,
        .t = 0,
        .depth = 0,
        .iso = 0,
        .force_t = VERT_0,
        .force_n = VERT_0,
        .relaxed = {
            .p = VERT_0,
            .n = VERT_0,
            .iso = kit->iso
        }
    };

    // check which case we are in
    if (cnt & 1) { // Frank or Hank
        // find the single vertex which is greater than or equal to the isovalue
        int frank = (cnt==1);
        for (vert=A; vert<=D && (frank)?(!geq[vert-A]):(geq[vert-A]); ++vert);
        check(vert <= D);

        int dir = ((!frank) ^ (vert&1)) * -2 + 1;

        TetVertex v[3] = { (vert+ 4 + 1 * dir)&3, (vert+ 4 + 2 * dir)&3, (vert+ 4 + 3 * dir)&3 };

        // create the half-edges
        t->edges[v[0]] = arrPush(kit->edges, &edge_tmp);
        t->edges[v[1]] = arrPush(kit->edges, &edge_tmp);
        t->edges[v[2]] = arrPush(kit->edges, &edge_tmp);
        // create the threads
        t->edges[v[0]]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[1]]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[2]]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[0]]->t->t = t->edges[v[0]]->t;
        t->edges[v[1]]->t->t = t->edges[v[1]]->t;
        t->edges[v[2]]->t->t = t->edges[v[2]]->t;
        // set the sample positions
        Vec3 v1, v2;
        vScale((const Vec3*)t->v[vert], (kit->iso - val[v[0]])/(val[vert]- val[v[0]]), &v1);
        vScale((const Vec3*)t->v[v[0]], (val[vert] - kit->iso)/(val[vert]- val[v[0]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[2]]->t->relaxed.p);

        vScale((const Vec3*)t->v[vert], (kit->iso - val[v[1]])/(val[vert]- val[v[1]]), &v1);
        vScale((const Vec3*)t->v[v[1]], (val[vert] - kit->iso)/(val[vert]- val[v[1]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[0]]->t->relaxed.p);

        vScale((const Vec3*)t->v[vert], (kit->iso - val[v[2]])/(val[vert]- val[v[2]]), &v1);
        vScale((const Vec3*)t->v[v[2]], (val[vert] - kit->iso)/(val[vert]- val[v[2]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[1]]->t->relaxed.p);

        t->edges[v[0]]->n = t->edges[v[1]];
        t->edges[v[1]]->n = t->edges[v[2]];
        t->edges[v[2]]->n = t->edges[v[0]];

        // check if there are opposing edges to hook up with
        uint8_t j;
        for (j=0; j<3; j++) {
            if (t->n[v[j]]) {
                HalfEdge opp = t->edges[v[j]]->o = t->n[v[j]]->edges[tetReadMap(t->m, v[j])];
                if (opp) {
                    opp->o = t->edges[v[j]];
                    thread_collapse_mt(opp->t, opp->o->n->t);
                    thread_collapse_mt(opp->o->t, opp->n->t);
                }
            }
        }

    } else if (cnt == 2) { // Gabrielle
        TetVertex v[3];
        // looking for the vertex which is on the same side as A
        for (v[0] = B; v[0]<=D && geq[v[0]]!=geq[A]; ++v[0]);
        check(geq[v[0]] == geq[A]);
        // The side of vertex A determines the direction.
        if (geq[A]) {
            v[1] = (v[0]+1+(v[0]==D))&3;
            v[2] = (v[1]+1+(v[1]==D))&3;
        } else {
            v[1] = (v[0]+4-1-(v[0]==B))&3;
            v[2] = (v[1]+4-1-(v[1]==B))&3;
        }

        // create the half-edges
        t->edges[A] = arrPush(kit->edges, &edge_tmp);
        t->edges[v[0]] = arrPush(kit->edges, &edge_tmp);
        t->edges[v[1]] = arrPush(kit->edges, &edge_tmp);
        t->edges[v[2]] = arrPush(kit->edges, &edge_tmp);

        // create the threads
        t->edges[A]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[0]]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[1]]->t = arrPush(kit->threads, &thread_tmp);
        t->edges[v[2]]->t = arrPush(kit->threads, &thread_tmp);

        t->edges[A]->t->t = t->edges[A]->t;
        t->edges[v[0]]->t->t = t->edges[v[0]]->t;
        t->edges[v[1]]->t->t = t->edges[v[1]]->t;
        t->edges[v[2]]->t->t = t->edges[v[2]]->t;

        // set the sample positions
        Vec3 v1, v2;

        vScale((const Vec3*)t->v[v[0]], (kit->iso - val[v[2]])/(val[v[0]]- val[v[2]]), &v1);
        vScale((const Vec3*)t->v[v[2]], (val[v[0]] - kit->iso)/(val[v[0]]- val[v[2]]), &v2);
        vAdd(&v1, &v2, &t->edges[A]->t->relaxed.p);

        vScale((const Vec3*)t->v[A], (kit->iso - val[v[1]])/(val[A]- val[v[1]]), &v1);
        vScale((const Vec3*)t->v[v[1]], (val[A] - kit->iso)/(val[A]- val[v[1]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[0]]->t->relaxed.p);

        vScale((const Vec3*)t->v[A], (kit->iso - val[v[2]])/(val[A]- val[v[2]]), &v1);
        vScale((const Vec3*)t->v[v[2]], (val[A] - kit->iso)/(val[A]- val[v[2]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[1]]->t->relaxed.p);

        vScale((const Vec3*)t->v[v[0]], (kit->iso - val[v[1]])/(val[v[0]]- val[v[1]]), &v1);
        vScale((const Vec3*)t->v[v[1]], (val[v[0]] - kit->iso)/(val[v[0]]- val[v[1]]), &v2);
        vAdd(&v1, &v2, &t->edges[v[2]]->t->relaxed.p);

        t->edges[A]->n = t->edges[v[2]];
        t->edges[v[2]]->n = t->edges[v[0]];
        t->edges[v[0]]->n = t->edges[v[1]];
        t->edges[v[1]]->n = t->edges[A];

        // create the diagonal half-edges
        HalfEdge e1 = arrPush(kit->edges, &edge_tmp);
        HalfEdge e2 = arrPush(kit->edges, &edge_tmp);
        e1->o = e2; e2->o = e1;

        // check which diagonal is best suited to be split
        vSub(&t->edges[A]->t->relaxed.p, &t->edges[v[0]]->t->relaxed.p, &v1);
        vSub(&t->edges[v[1]]->t->relaxed.p, &t->edges[v[2]]->t->relaxed.p, &v2);
        if (vNormSquared((const Vec3*)&v1) < vNormSquared((const Vec3*)&v2)) {
            // split along v1
            e1->t = t->edges[A]->t; e1->n = t->edges[v[0]]; t->edges[v[1]]->n = e1;
            e2->t = t->edges[v[0]]->t; e2->n = t->edges[A]; t->edges[v[2]]->n = e2;
        } else {
            e1->t = t->edges[v[1]]->t; e1->n = t->edges[v[2]]; t->edges[v[0]]->n = e1;
            e2->t = t->edges[v[2]]->t; e2->n = t->edges[v[1]]; t->edges[A]->n = e2;
            // split along v2
        }

        // check if there are opposing edges to hook up with
        uint8_t j;
        for (j=0; j<4; j++) {
            if (t->n[j]) {
                HalfEdge opp = t->edges[j]->o = t->n[j]->edges[tetReadMap(t->m, j)];
                if (opp) {
                    opp->o = t->edges[j];
                    thread_collapse_mt(opp->t, opp->o->n->t);
                    thread_collapse_mt(opp->o->t, opp->n->t);
                }
            }
        }

    } else {
        // no edges are creted. the isosurface does not intersect this tet
    }

}



bool specProject(Spectrum restrict sp)
{
    //call;

    // find the next isovalue to project to
    //debug(fprintf(stderr, "Finding next isovalue.\n"););
    real crt = sp->snap_iso;
    while (!arrIsEmpty(sp->isosamples) && crt <= *oCast(real*, arrBack(sp->isosamples))) arrPop(sp->isosamples);
    if (arrIsEmpty(sp->isosamples)) return false;

    real next = *oCast(real*, arrBack(sp->isosamples));
    Array criticalities = arrCreate(sizeof (Vertex), 1);
    {
        CriticalPoint cp = sp->vol->topology.criticalities;
        CriticalPoint end = cp + sp->vol->topology.size;
        while (++cp < end-1) // skip the global min and global max
            if (crt > cp->isovalue && next < cp->isovalue)
                arrPush(criticalities, &cp->pos);
    }

    // if it does not cross any critical value
    if (arrIsEmpty(criticalities)) {
        // project normally
        //debug(fprintf(stderr, "Projecting normally.\n"););
        struct projection_kit kit = { .sp = sp, .iso = next, .crit = 0, .unproj = 0, .samples = 0, .threads = 0, .edges=0, .border=0};
        arrForEach(sp->active_threads, project_thread, &kit);
    } else {// otherwise:

        specProcessFringe(sp);
        specSimplify(sp);
        specRelax(sp);

        // project across the critical area
        //debug(fprintf(stderr, "Projecting across critical area.\n"););

        // do a partial projection and create a delaunay tetrahedrization containing:
        struct projection_kit kit = {
            .sp = sp, .iso = next, .r = 3,
            .crit = criticalities,
            .unproj = arrCreate(sizeof (Thread), 1),
            .threads = arrCreate(sizeof (struct Thread), 1),
            .edges = arrCreate(sizeof (struct HalfEdge), 1),
            .border = arrCreate(sizeof (HalfEdge), 1),
            .samples = arrCreate(sizeof (Vertex), 1),
        };
        //  > the unprojected samples
        arrForEach(sp->active_threads, project_thread, &kit);
        fprintf(stderr, "Unprojected: %lu threads.\n", arrSize(kit.unproj));

        //  > the projections of samples whose triangle neighbors have not been projected
        // compute the "border"
        arrForEach(sp->fringe, find_border, &kit);
        fprintf(stderr, "Bordering: %lu edges.\n", arrSize(kit.border));

        //  > the critical points crossed
        // sample criticalities
        arrForEach(criticalities, sample_crit, &kit);
        //add some random samples inside the half-radius ball around each critical point
        struct sample_kit s_kit = { .r = kit.r , .n = 10, .s = kit.samples };
        arrForEach(kit.crit, ball_sample, &s_kit);

        fprintf(stderr, "Samples: %lu.\n", arrSize(kit.samples));

        sp->del = isoSample(sp->vol->scal, kit.crit, kit.samples, kit.iso, kit.r);

        arrForEach(sp->del->t, march_tets, &kit);
        fprintf(stderr, "Edges: %lu.\n", arrSize(kit.edges));
        fprintf(stderr, "Threads: %lu.\n", arrSize(kit.threads));

       arrForEach(kit.threads, snap_mt_threads, &kit);
/*
        // stitch to the border
        arrForEach(kit.border, stitch_border, &kit);

        // mark the edges which were clipped out during the stitching process
        // by making their o, and n fields point to themselves
        arrForEach(kit.border, mark_clipped_edges, &kit);

        // create real counterparts for the edges which were not clipped out
        arrForEach(kit.edges, create_real_edges, &kit);

        // propagate the connectivity information to the fringe
        arrForEach(kit.edges, propagate_connectivity, &kit);

        // clear the att fields of the fringe
        arrForEach(sp->fringe, clear_att, 0);

        fprintf(stderr, "Edges: %lu.\n", arrSize(kit.edges));
        fprintf(stderr, "Threads: %lu.\n", arrSize(kit.threads));
*/

        arrDestroy(kit.unproj);
        arrDestroy(kit.samples);
    //    arrDestroy(kit.threads);
        sp->extracted_threads = kit.threads;
    //    arrDestroy(kit.edges);
        sp->extracted_edges = kit.edges;
        arrDestroy(kit.border);
        stub;
        return false;
    }

    arrDestroy(criticalities);
    sp->snap_iso = next;
    return true;
}

void specProcessFringe(Spectrum restrict sp)
{
    specRefine(sp);
    specRelax(sp);
    specSimplify(sp);
    specRelax(sp);
}

void specMerge(Spectrum restrict sp)
{
}

void display_marked_edges(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);

    HalfEdge e = o;
    if (!e->att) return;

    Thread ta = e->t; while (ta!=ta->t) ta = ta->t;
    Thread tb = e->n->t; while (tb!=tb->t) tb = tb->t;

    Sample sa = arrBack(ta->samples);
    Sample sb = arrBack(tb->samples);
    glVertex3v(sa->p);
    glVertex3v(sb->p);

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

    Vec3 n;

//    glColor3f((e->att || e->n->n->att), (!(e->att || e->n->n->att)), 0);
    vScale((const Vec3*)&sa->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sa->n);
    glVertex3v(sa->p);
//    glColor3f((e->att || e->n->att), (!(e->att || e->n->att)), 0);
    vScale((const Vec3*)&sb->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sb->n);
    glVertex3v(sb->p);
//    glColor3f((e->n->att || e->n->n->att), (!(e->n->att || e->n->n->att)), 0);
    vScale((const Vec3*)&sc->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sc->n);
    glVertex3v(sc->p);
}


void display_extracted_edge(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);

    HalfEdge e = o;
    if ((e->t > e->n->t) || (e->t > e->n->n->t)) return;
    Thread ta = e->t; while (ta!=ta->t) ta = ta->t;
    Thread tb = e->n->t; while (tb!=tb->t) tb = tb->t;
    Thread tc = e->n->n->t; while (tc!=tc->t) tc = tc->t;

    Sample sa = &ta->relaxed;
    Sample sb = &tb->relaxed;
    Sample sc = &tc->relaxed;

    Vec3 n;

//    glColor3f((e->att || e->n->n->att), (!(e->att || e->n->n->att)), 0);
    vScale((const Vec3*)&sa->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sa->n);
    glVertex3v(sa->p);
//    glColor3f((e->n->att || e->n->n->att), (!(e->n->att || e->n->n->att)), 0);
    vScale((const Vec3*)&sc->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sc->n);
    glVertex3v(sc->p);
//    glColor3f((e->att || e->n->att), (!(e->att || e->n->att)), 0);
    vScale((const Vec3*)&sb->n, -1, &n);
    glNormal3v(n);
//  glNormal3v(sb->n);
    glVertex3v(sb->p);
}

void display_sample(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);
    Sample s = o;
    if (d) glVertex3v(s->p);
    else {
        glPushMatrix();
        glTranslatef(s->p[0], s->p[1], s->p[2]);
        glutSolidSphere(0.05, 3, 3);
        glPopMatrix();
    }
}

void display_vert(uint64_t i, Obj o, Obj d)
{
    unused(i);
   // Spectrum sp = d;
    Thread* t = o;
    Sample s = arrBack((*t)->samples);
    if (vIsZero(&s->n)) {
        glPushMatrix();
        glTranslated(s->p[0], s->p[1], s->p[2]);
        glutSolidSphere(0.05, 3, 3);
        glPopMatrix();
    } else {
        Vec3 n;
        glBegin(GL_LINES);
        vScale((const Vec3*)&s->n, 1/(255*vNormSquared((const Vec3*)&s->n)), &n);
        vAddI(&n, (const Vec3*)&s->p);
        glVertex3v(s->p);
        glVertex3v(n);
        vScale((const Vec3*)&s->n, 1/(255*vNormSquared((const Vec3*)&s->n)), &n);
        vAddI(&n, (const Vec3*)&s->p);
        glVertex3v(s->p);
        glVertex3v(n);
        glEnd();
    }
}

void display_thread(uint64_t i, Obj o, Obj d)
{
    unused(i);
    unused(d);
    Thread* pt = o;
    Thread t = *pt;
    if (t->active) glColor3f(0.0, 0.0, 1.0); else glColor3f(1.0, 0.0, 0.0);

    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    arrForEach(t->samples, display_sample, pt);
    glEnd();

    arrForEach(t->samples, display_sample, pt);
}


void specDisplay(Spectrum restrict sp)
{
    /*
    glTranslatef(
            sp->vol->scal->nx * sp->vol->scal->dx * -0.5,
            sp->vol->scal->ny * sp->vol->scal->dy * -0.5,
            sp->vol->scal->nz * sp->vol->scal->dz * -0.5
            );
*/

    // nucleon
    //glTranslatef(-19, -20, -26);

    // hydro-200
    //glScalef(0.25, 1.0, 1.0);
    glTranslatef(-62, -63, -63);

    // hydro-15
    //glTranslatef(-53.7646, -63, -63);

    glLineWidth(1.0);

    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0);

    pthread_mutex_lock(&sp->mutex);

   // arrForEach(sp->active_threads, display_thread, 0);

    glEnable(GL_LIGHTING);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);
    arrForEach(sp->fringe, display_edge, 0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glDisable(GL_LIGHTING);


    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    arrForEach(sp->fringe, display_edge, 0);
    glEnd();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


   // if (sp->del) delDisplay(sp->del, 0);


    if (sp->extracted_edges) {
        glEnable(GL_LIGHTING);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_TRIANGLES);
        arrForEach(sp->extracted_edges, display_extracted_edge, 0);
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_LIGHTING);

        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_TRIANGLES);
        arrForEach(sp->extracted_edges, display_extracted_edge, 0);
        glEnd();
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }


    // the marked edges
    glLineWidth(3.0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    arrForEach(sp->fringe, display_marked_edges, 0);
    glEnd();
    glLineWidth(1.0);

/*
    // the critical point
    glPushMatrix();
    glTranslatef(19, 20, 26);
    glColor3f(1.0, 1.0, 0.0);
    glutSolidSphere(0.075, 5, 5);
    glPopMatrix();
*/
    glColor3f(1.0, 0.0, 0.0);
    arrForEach(sp->active_threads, display_vert, sp);
    pthread_mutex_unlock(&sp->mutex);


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
