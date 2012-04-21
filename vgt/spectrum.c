
#include <vgt/spectrum.h>
#include <vgt/spectrum_cls.h>

#include <math/obj.h>
#include <math/vertex.h>

#include <ads/array.h>

#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>
#include <vgt/volumetric_data.h>
#include <vgt/volumetric_data_cls.h>


#include <string.h>
#include <stdlib.h>
#include <errno.h>

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
    fputs(line, stdout);
    conjecture(sscanf(line, "%u %u %u", &vert, &faces, &aux) == 3, "Failed to read #vert #faces #<unused> from off file.");
    printf("%u vertices, %u faces\n", vert, faces);

    // read vertices
    uint64_t i = 0;
    struct Sample smpl = { .p = VERT_0, .n = VERT_0, .iso = initial_isovalue };
    for (i = 0; i < vert; i++) {
        conjecture(fgets(line, sizeof(line), fin), "Error reading from off file.");
        conjecture(sscanf(line, "%f %f %f\n", &x, &y, &z) == 3, "failed to read vertex from off file.");
        vSet(&smpl.p, x, y, z);
        struct Thread tmp_thr = {
            .samples = arrCreate(sizeof (struct Sample), 1),
            .id = i,
            .c_id = i
        };
        ignore arrPush(tmp_thr.samples, &smpl);
        Thread t = arrPush(thr, &tmp_thr);
        ignore arrPush(active_thr, &t);
    }

    {// read faces
        unsigned int a, b, c, cnt;
        Normal p, q, norm;
        ind i;

        for (i = 0; i < faces; i++) {
            conjecture(fgets(line, sizeof(line), fin), "Error reading from off file.");
            conjecture(sscanf(line, "%u %u %u %u\n", &cnt, &a, &b, &c) == 4, "Failed to read face from off file.");

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

            Sample sa = oCast(Sample, arrBack(ea->t->samples));
            Sample sb = oCast(Sample, arrBack(eb->t->samples));
            Sample sc = oCast(Sample, arrBack(ec->t->samples));

            ignore vCross( vSub(&sb->p, &sa->p, &p),
                           vSub(&sc->p, &sa->p, &q),
                           &norm);
            vAddI(&sa->n, &norm);
            vAddI(&sb->n, &norm);
            vAddI(&sc->n, &norm);
        }
    }

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
        .max_force = 0.01;
    };

    return oCopy(&sp, sizeof (struct Spectrum));
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
    oDestroy(sp);
}

inline static
void exert_force(uint64_t i, Obj o, Obj d)
{
}

inline static
void extract_tangential(uint64_t i, Obj o, Obj d)
{
}

inline static
void relax(uint64_t i, Obj o, Obj d)
{
}

inline static
void recompute_normal(uint64_t i, Obj o, Obj d)
{
}


void specRelax(Spectrum restrict sp)
{
    while (1) {
        // compute the exerted force
        arrForEach(sp->fringe, exert_force, 0);

        // extract the tangential component of the force
        // and compute the maximum squared norm
        real force = 0.0;
        arrForEach(sp->active_threads, extract_tangential, &force);

        if (force < sp->max_force) break;

        // relax samples
        arrForEach(sp->active_threads, relax, 0);

        // recompute normal
        arrForEach(sp->fringe, recompute_normal, 0);
    }
    oDestroy(energy);
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
