
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
    if (!vdRead(v, filename)) {oDestroy(v); return 0;}
    return v;
}

VolumetricData vdCopy(VolumetricData v)
{
    VolumetricData c = oCopy(v, sizeof(struct VolumetricData));
    c->scal = sfCopy(v->scal);
    c->grad = vfCopy(v->grad);
    c->lapl = sfCopy(v->lapl);
    return c;
}

bool
vdRead(VolumetricData v, const char* filename)
{
    /* TODO: check preconditions */

    if (!filename) return true;

    FILE* fin = fopen(filename, "r");

    if (!fin) {
        printf("Error opening file [%s].", filename);
        return false;
    }

    // temporary data
    char raw_file_name[1024];
    char off_file_name[1024];
    float initial_isovalue = 0.0;
    char line[1024];


    // read the .raw file name
    do {
        if (!fgets(line, 1024, fin)) {
            fprintf(stderr, "Error reading the raw file name from configuration file [%s].", filename);
            fclose(fin);
            return false;
        }
        strip(line);
    } while (sscanf(line, "%s", raw_file_name) != 1);

    unsigned int nx, ny, nz;

    // read the size of the data in the raw file
    do {
        if (!fgets(line, 1024, fin)) {
            fprintf(stderr, "Error reading the size of the raw file data from configuration file [%s].", filename);
            fclose(fin);
            return false;
        }
        strip(line);
    } while (sscanf(line, "%u %u %u", &nx, &ny, &nz) != 3);

    float sx, sy, sz;

    // read the voxel size in each dimension
    do {
        if (!fgets(line, 1024, fin)) {
            fprintf(stderr, "Error reading the voxel size from configuration file [%s].", filename);
            fclose(fin);
            return false;
        }
        strip(line);
    } while (sscanf(line, "%f %f %f", &sx, &sy, &sz) != 3);

    v->scal = sfCreate(nx, ny, nz, sx, sy, sz);
    sfReadRaw(v->scal, raw_file_name);

    v->grad = sfGradient(v->scal);
    //v->lapl = vfDivergence(v->grad);
    v->lapl = sfLaplacian(v->scal);

    // read the name of the .off file with the initial mesh and the initial isovalue for which it was extracted
    do {
        if (!fgets(line, 1024, fin)) {
            fprintf(stderr, "Error reading the off file name and initial isovalue from configuration file [%s].", filename);
            fclose(fin);
            return false;
        }
        strip(line);
    } while (sscanf(line, "%s %f", off_file_name, &initial_isovalue) != 2);
    // scale down the initial isovalue
    initial_isovalue /= 255.0f;


    // read the number of critical points
    do {
        if (!fgets(line, 1024, fin)) {
            fprintf(stderr, "Error reading the number of critical points from configuration file [%s].", filename);
            vdClear(v);
            fclose(fin);
            return false;
        }
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

    // read the data from the off file
    if (!freopen(off_file_name, "r", fin)) {
        fprintf(stderr, "[x] File [%s]: %s\n", off_file_name, strerror(errno));
        vdClear(v);
        fclose(fin);
        return false;
    }

    fclose(fin);

    return true;
}

void
vdClear(VolumetricData v)
{
    if (!v) return;
    sfClear(v->scal);
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

