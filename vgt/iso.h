#ifndef VGT_ISO_H
#define VGT_ISO_H

#include <vgt/types.h>

Delaunay isoSample(   const ScalarField const restrict data,
                        Array restrict crit,
                        Array restrict samples,
                        real isoValue,
                        real readius
                        );

#endif//VGT_ISO_H
