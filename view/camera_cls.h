#ifndef VIEW_CAMERA_CLS_H
#define VIEW_CAMERA_CLS_H

#include <view/types.h>

struct Camera {
    struct Spherical view;
    struct Frame frame;
    struct Graphics focus_point;
};

#endif//VIEW_CAMERA_CLS_H
