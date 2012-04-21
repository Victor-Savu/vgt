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
    
    real max_force;
};

#endif//VGT_SPECTRUM_CLS_H
