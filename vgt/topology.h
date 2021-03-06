#ifndef VGT_TOPOLOGY_H
#define VGT_TOPOLOGY_H

#include <vgt/types.h>

enum CRITICAL_POINT_TYPE { MINIMUM, MAXIMUM, SADDLE };

typedef struct CriticalPoint* CriticalPoint;
struct CriticalPoint {
    real isovalue;
    enum CRITICAL_POINT_TYPE type;
    Vertex pos;
};

typedef struct Toplology* Topology;
struct Topology {
    CriticalPoint criticalities;
    uint64_t size;
};

#endif//TOPOLOGY_H
