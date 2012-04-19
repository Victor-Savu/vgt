#ifndef VGT_SPECTRUM_CLS_H
#define VGT_SPECTRUM_CLS_H

#include <types.h>


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
};

// a 3D shape bordered by 3 threads with 2 end-points defined by their isovalues
struct Triangloid {
    // isovalue range for which this triangloid is defined (iso[0] <= iso[1])
    float iso[2];
    // thread IDs defining the vertices of each slice.
    uint32_t t_ids[3];
};

// a set of triangloids and their defining threads
// it is constructed from a starting (minimum) isovalue isoMin
struct Spectrum {
    // the threads
    Array thr;
    // the triangloids
    Array tri;

    // The frontal mesh used to grow the spectrum in positive isovalue direction
    
};

#endif//VGT_SPECTRUM_CLS_H
