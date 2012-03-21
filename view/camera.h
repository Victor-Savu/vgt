#ifndef VIEW_CAMERA_H
#define VIEW_CAMERA_H

#include <view/types.h>

/* Public class */
#include <view/camera_cls.h>

void camPan(Camera restrict c, real x, real y);

void camRotate(Camera restrict c, real up, real right);

void camZoom(Camera restrict c, real f);

void camPosition(Camera restrict e);

#endif//VIEW_CAMERA_H

