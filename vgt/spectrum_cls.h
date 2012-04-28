#ifndef VGT_SPECTRUM_CLS_H
#define VGT_SPECTRUM_CLS_H

#include <vgt/types.h>

// a set of triangloids and their defining threads
// it is constructed from a starting (minimum) isovalue isoMin
struct Spectrum {
    // the threads
    Array thr;
    // the triangloids
    Array tri;

    // The frontal mesh used to grow the spectrum
    // in positive isovalue direction
    Array fringe;

    VolumetricData vol;

    Array active_threads;
    Array active_triangles;
    
    pthread_mutex_t mutex;
    uint64_t observers;

    // snapping parameters
    real snap_iso;
    real snap_iso_thr;

    // relaxation parameters
    real scale;
    real scale_threshold;
    real current_stress;
    real previous_stress;

    // simplification parameters
    real area_sqr;
    real lambda;

    // refinement parameters
    real ref_norm_thr;
};

#endif//VGT_SPECTRUM_CLS_H
