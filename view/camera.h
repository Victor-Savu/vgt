#ifndef VIEW_CAMERA_H
#define VIEW_CAMERA_H

/* Public class */
#include <view/camera_cls.h>

void camPan(Camera c, real x, real y);

void camRotate(Camera c, real up, real, right);

void camZoom(Camera x, real f);

#endif//VIEW_CAMERA_H

