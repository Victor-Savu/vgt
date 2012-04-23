
#include <vgt/spectrum.h>
#include <vgt/spectrum_cls.h>

#include <math/obj.h>
#include <math/vertex.h>
#include <math/algorithms.h>

#include <ads/array.h>

#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/volumetric_data.h>
#include <vgt/volumetric_data_cls.h>

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
    // the thread id after the collapse with another thread
    uint32_t c_id;

    // the force exerted on the last sample of the thread by its incident triangles
    Vec3 force;
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
};

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
void elimin_discon(uint64_t i, Obj o, Obj d)
{
    unused(i);

    do {
        Thread* t = o;
        Sample s = arrBack((*t)->samples);
        if (vIsZero(&s->n)) {
            *t = *oCast(Thread*, arrBack(d));
            arrPop(d);
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
        vSet(&smpl.p, x, y, z);
        vfValue(v->grad, &smpl.n, x, y, z);
        struct Thread tmp_thr = {
            .samples = arrCreate(sizeof (struct Sample), 1),
            .id = i,
            .c_id = i
        };
        ignore arrPush(tmp_thr.samples, &smpl);
        Thread t = arrPush(thr, &tmp_thr);
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
        struct HalfEdge edge_a = { .t = arrGet(thr, a), .n = 0, .o = 0 };
        struct HalfEdge edge_b = { .t = arrGet(thr, b), .n = 0, .o = 0 };
        struct HalfEdge edge_c = { .t = arrGet(thr, c), .n = 0, .o = 0 };

        HalfEdge ea = arrPush(fringe, &edge_a);
        HalfEdge eb = arrPush(fringe, &edge_b);
        HalfEdge ec = arrPush(fringe, &edge_c);

        ea->n = eb; eb->n = ec; ec->n = ea;

        // create the triangloid
        struct Triangloid t = {
            .iso = {initial_isovalue, initial_isovalue},
                    .t_ids = {ea->t->id, eb->t->id, ec->t->id}
        };
        arrPush(tri, &t);
/*
        Sample sa = oCast(Sample, arrBack(ea->t->samples));
        Sample sb = oCast(Sample, arrBack(eb->t->samples));
        Sample sc = oCast(Sample, arrBack(ec->t->samples));

        ignore vCross( vSub(&sb->p, &sa->p, &p),
                       vSub(&sc->p, &sa->p, &q),
                       &norm);
        vAddI(&sa->n, &norm);
        vAddI(&sb->n, &norm);
        vAddI(&sc->n, &norm);*/
    }

    // eliminate disconnected vertices
    arrForEach(active_thr, elimin_discon, active_thr);

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
    Sample sa = arrBack(e->t->samples);
    Sample sb = arrBack(e->n->t->samples);
    Sample sc = arrBack(e->n->n->t->samples);
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
  //  arrForEach(s->fringe, interp_bar_error, &kit);

    fprintf(stderr, "Interp error: %lf\n", oCast(double, kit.ierror));
    fprintf(stderr, "Normal error: %lf\n", oCast(double, kit.nerror));
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

    Vec3* fa = &e->t->force;
    Vec3* fb = &e->n->t->force;
    Vec3* fc = &e->n->n->t->force;

    Vertex* a = &oCast(Sample, arrBack(e->t->samples))->p;
    Vertex* b = &oCast(Sample, arrBack(e->n->t->samples))->p;
    Vertex* c = &oCast(Sample, arrBack(e->n->n->t->samples))->p;

    Vec3 m; // median cm
    vSubI(vScaleI(vAdd(a, b, &m), 0.5), c);

    Vec3 ab;
    vNormalizeI(vSub(b, a, &ab));

    vScaleI(&ab, vDot(&m, &ab)/2);

    vAddI(fc, &ab);
    vScaleI(&ab, 0.5);
    vSubI(fa, &ab);
    vSubI(fb, &ab);
}

static real scale = 0.3;

inline static
void exert_normal_force(uint64_t i, Obj o, Obj d)
{
    unused(i);

    Thread* t = o;
    Sample s = arrBack((*t)->samples);

    Normal n; vNormalize(&s->n, &n);

    vSubI(&(*t)->force, vScaleI(&n, vDot(&n, &(*t)->force)));

    real di = s->iso - sfValue(d, s->p[0], s->p[1], s->p[2]);

    vScale(&s->n, di, &n);
    vAddI(&(*t)->force, &n);
}



inline static
void relax(uint64_t i, Obj o, Obj d)
{
    unused(i);
    Thread* thr = o;
    Sample s = arrBack((*thr)->samples);
    Vec3* p = &s->p;
    Vec3* f = &(*thr)->force;

    real sz = vNorm(f);

    vAddI(p, vScaleI(f, scale));

    //if (sz > *oCast(real*, d)) *oCast(real*, d) = sz;

    *oCast(real*, d) += sz;
    vSet(f, 0, 0, 0);
}


inline static
void recompute_normal(uint64_t i, Obj o, Obj d)
{
    unused(i);

    Thread* t = o;
    Sample s = arrBack((*t)->samples);
    while (vIsZero(vfValue(d, &s->n, s->p[0], s->p[1], s->p[2]))) {
        s->p[0] += algoRandomDouble(-0.1, 0.1);
        s->p[1] += algoRandomDouble(-0.1, 0.1);
        s->p[2] += algoRandomDouble(-0.1, 0.1);
    }

    conjecture(!vIsZero(&s->n), "oh no!");

/*
    HalfEdge e = o;
    if ((e->t->id > e->n->t->id) || (e->t->id > e->n->n->t->id)) return;

    Sample sa = arrBack(e->t->samples);
    Sample sb = arrBack(e->n->t->samples);
    Sample sc = arrBack(e->n->n->t->samples);

    Normal p, q, norm;
    ignore vCross( vSub(&sb->p, &sa->p, &p),
            vSub(&sc->p, &sa->p, &q),
            &norm);
    vAddI(&sa->n, &norm);
    vAddI(&sb->n, &norm);
    vAddI(&sc->n, &norm);
*/
}


void specRelax(Spectrum restrict sp)
{
    real force = 1e7;
    uint32_t maxit = 100;
    scale = 0.3;
    real e = force;

    fprintf(stderr, "force: %lf\tscale: %lf\n", oCast(double, force), oCast(double, scale));
    fflush(stderr);

    //while (force > sp->max_force && maxit--) {
    while (maxit--) {
        // compute the exerted force
        arrForEach(sp->fringe, exert_tangent_force, 0);
        arrForEach(sp->active_threads, exert_normal_force, sp->vol->scal);

        // extract the tangential component of the force
        // use it to relax the sample and compute the maximum squared norm
        // of the force exerted to determine the stopping criteria

        force = 0.0;
        pthread_mutex_lock(&sp->mutex);
        arrForEach(sp->active_threads, relax, &force);
        pthread_mutex_unlock(&sp->mutex);

        e /= force; // finite

        //e *= e;

        if (e < 1)
            scale *= 1.05*e*e;
        else
            scale *= 1.0 + 0.05/e;
        /*
        if (e > 1.01)
            scale /= e;
        else if (e < 0.99)
            scale *= 2 - e;
        else force = 0;
        */

        fprintf(stderr, "force: %lf\tscale: %lf\n", oCast(double, force), oCast(double, scale));
        fflush(stderr);



        e = force;

        // recompute normal
        //arrForEach(sp->fringe, recompute_normal, 0);
        arrForEach(sp->active_threads, recompute_normal, sp->vol->grad);
    }
}

void specSimplify(Spectrum restrict sp)
{
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
    glLineWidth(1.0);

    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0);

    pthread_mutex_lock(&sp->mutex);

    arrForEach(sp->thr, display_thread, 0);
    glEnable(GL_LIGHTING);


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

    glDisable(GL_COLOR_MATERIAL);
}
