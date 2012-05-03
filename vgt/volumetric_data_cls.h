#ifndef VGT_VOLUMETRIC_DATA_CLS_H
#define VGT_VOLUMETRIC_DATA_CLS_H

#include <vgt/topology.h>

/*
   @class VolumetricData
*/
struct VolumetricData {
    // The scalar values
    ScalarField scal;

    // Minimum cell values
    ScalarField min;

    // Maximum cell values
    ScalarField max;

    // The precomputed gradient of the field
    VectorField grad;

    // The precomputed laplacian
    ScalarField lapl;

    /* The precomputed distances between dimensions.
       e.g.: data[i][j][k] = data + i * dz + j * dy + k * dx
    */
    int dx, dy, dz;

    /* Information about the critical regions in the scalar field */
    struct Topology topology;

    /* The size of the scalar field along each dimension */
    uint64_t nx, ny, nz;

    /* The real size of the data voxel. */
    real sx, sy, sz;
};

#endif//VGT_VOLUMETRIC_DATA_CLS_H
