#ifndef VIEW_RENDERER_CLS_H
#define VIEW_RENDERER_CLS_H

#include <view/types.h>

#include <view/camera_cls.h>
#include <view/mouse_cls.h>

#include <pthread.h>

struct Renderer
{
    Mesh m;
    Mesh new_m;

    Victor v;
    Victor new_v;

    Delaunay d;
    Delaunay new_d;

    pthread_t threadId;
    pthread_mutex_t mutex;
//    pthread_cond_t cond;
    byte props;

    struct Camera camera;
    struct Mouse mouse;
};

#endif//VIEW_RENDERER_CLS_H
