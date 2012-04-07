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

    byte props;

    struct Camera camera;
    struct Mouse mouse;

    pthread_mutex_t wait_key;
    pthread_cond_t  key_pressed;
    char* key;

    int widget;
};

#endif//VIEW_RENDERER_CLS_H
