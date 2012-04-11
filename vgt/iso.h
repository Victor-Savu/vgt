#ifndef VGT_ISO_H
#define VGT_ISO_H

#include <vgt/types.h>

Mesh isoMarchingTets(   const ScalarField const restrict data,
                        Array restrict border,
                        Array restrict samples,
                        real isoValue
                        );

#endif//VGT_ISO_H
