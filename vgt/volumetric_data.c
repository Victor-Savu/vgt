
#include <vgt/volumetric_data.h>
#include <vgt/volumetric_data_cls.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <math/obj.h>

#include <vgt/topology.h>
#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>


// Helper functions
static char* strip(char* line);


VolumetricData
vdCreate(const char* filename)
{
    VolumetricData v = oCreate(sizeof (struct VolumetricData));
    FILE* fin = fopen(filename, "r");
    conjecture(fin, "Error opening file.");
    if (!vdRead(v, fin, filename)) {oDestroy(v); return 0;}
    fclose(fin);
    return v;
}

VolumetricData vdCopy(VolumetricData v)
{
    VolumetricData c = oCopy(v, sizeof(struct VolumetricData));
    c->scal = sfCopy(v->scal);
//    c->min = sfCopy(v->min);
//    c->max = sfCopy(v->max);
    c->grad = vfCopy(v->grad);
    c->lapl = sfCopy(v->lapl);
    return c;
}
/*
inline static
real min_real(real x, real y) { return (x<y)?(x):(y); }
inline static
real max_real(real x, real y) { return (x<y)?(x):(y); }

inline static
void min_value(real* arg[8], real* v)
{
    real min_tmp[4] = {
        min_real(*arg[0], *arg[1]),
        min_real(*arg[2], *arg[3]),
        min_real(*arg[4], *arg[5]),
        min_real(*arg[6], *arg[7])
    };
    min_tmp[0] = min_real(min_tmp[0], min_tmp[1]);
    min_tmp[1] = min_real(min_tmp[2], min_tmp[3]);
    *v = min_real(min_tmp[0], min_tmp[1]);
}

inline static
void max_value(real* arg[8], real* v)
{
    real max_tmp[4] = {
        max_real(*arg[0], *arg[1]),
        max_real(*arg[2], *arg[3]),
        max_real(*arg[4], *arg[5]),
        max_real(*arg[6], *arg[7])
    };
    max_tmp[0] = max_real(max_tmp[0], max_tmp[1]);
    max_tmp[1] = max_real(max_tmp[2], max_tmp[3]);
    *v = max_real(max_tmp[0], max_tmp[1]);
}
*/
bool
vdRead(VolumetricData v, FILE* fin, const char* filename)
{
    // temporary data
    char raw_file_name[1024];
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

//    v->min = sfCreate(nx-1, ny-1, nz-1, sx, sy, sz);
//    sfVoxelOp(v->scal, min_value, v->min);

//    v->max = sfCreate(nx-1, ny-1, nz-1, sx, sy, sz);
//    sfVoxelOp(v->scal, max_value, v->max);

    v->grad = sfGradient(v->scal);
    //v->lapl = vfDivergence(v->grad);
    v->lapl = sfLaplacian(v->scal);

    return true;
}

void
vdClear(VolumetricData v)
{
    if (!v) return;
    sfClear(v->scal);
//    sfClear(v->max);
//    sfClear(v->min);
    vfClear(v->grad);
    sfClear(v->lapl);
    free(v->topology.criticalities);
    memset(v, 0, sizeof (struct VolumetricData));
}

void
vdDestroy(VolumetricData v)
{
    if (!v) return;
    sfDestroy(v->scal);
//    sfDestroy(v->max);
//    sfDestroy(v->min);
    vfDestroy(v->grad);
    sfDestroy(v->lapl);
    free(v->topology.criticalities);
    memset(v, 0, sizeof (struct VolumetricData));
    free(v);
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

