#ifndef VGT_TOPOLOGY_H
#define VGT_TOPOLOGY_H

#include <vgt/types.h>

#include <vgt/vec_cls.h>

enum CRITICAL_POINT_TYPE { MINIMUM, MAXIMUM, SADDLE };

typedef struct CriticalPoint *CriticalPoint;
struct CriticalPoint {
    real isovalue;
    enum CRITICAL_POINT_TYPE type;
    struct Vec pos;
};

typedef struct Toplology *Topology;
struct Topology {
    CriticalPoint criticalities;
    uint size;
};

#endif//TOPOLOGY_H
