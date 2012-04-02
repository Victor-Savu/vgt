#ifndef VGT_VICTOR_H
#define VGT_VICTOR_H

#include <vgt/types.h>

Victor vicCreate(VolumetricData vol);

void vicDestroy(Victor restrict vic);

Victor vicShallowCopy(Victor restrict vic);

Victor vicCopy(Victor restrict vic);

void vicDisplay(Victor restrict vic);

#endif//VGT_VICTOR_H
