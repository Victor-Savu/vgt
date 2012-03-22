#include <view/renderer.h>
#include <view/renderer_cls.h>

#include <math/obj.h>
#include <string.h>
#include <stdio.h>

#include <math/obj.h>
#include <vgt/mesh.h>
#include <view/camera.h>
#include <view/graphics.h>

#include <GL/glut.h>

void cb_display(void);
void cb_reshape(int w, int h);
void cb_keyboard(unsigned char key, int x, int y);
void cb_special_key(int key, int x, int y);
void cb_update(int time);
void cb_idle(void);

void* init_rendering(void*);

#define REQ_SHUTDOWN        (1)
#define RUNNING             (2)
#define DRAW_FOCUS_POINT    (4)


struct Renderer instance = {
    .m = 0,
    .q = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .props = 0,

    .camera = {
        .view = {.rho = 0.0, .phi = 0.0, .theta = 0.0},
        .frame = FRAME_I,
        .focus_point = {
            .draw = GRAPHICS_DRAW_3D_ARROW
        },
    },
    .mouse = {
        .buttons = 0,
        .x = 0,
        .y = 0
    }
};

Renderer rCreate(const char* winname)
{
    if (instance.props & REQ_SHUTDOWN) rDestroy(&instance);

    instance.props = (RUNNING);
    pthread_create(&instance.threadId, 0, init_rendering, 0);

    return &instance;
}

void rDestroy(Renderer restrict r)
{
    unused(r);
    if (!(instance.props & RUNNING)) return;

    instance.props |= REQ_SHUTDOWN;
    rWait(r);

    if (instance.m) mDestroy(instance.m);
    if (instance.q) mDestroy(instance.q);
    instance.m = 0;
    instance.q = 0;
    instance.props = 0;
}

void rWait(Renderer restrict r)
{
    Obj ret = 0;
    if (!pthread_join(instance.threadId, &ret)) oDestroy(ret);
}

void rDisplay(Renderer restrict r, Mesh restrict m)
{
    pthread_mutex_lock(&instance.mutex);
    Mesh restrict old = instance.q;
    instance.q = m;
    pthread_mutex_unlock(&instance.mutex);
    if (old) mDestroy(old);
}

void cb_display(void)
{
    if (instance.m) mDisplay(instance.m);

    if (instance.props | DRAW_FOCUS_POINT) {
        gfxDraw(&instance.camera.focus_point);
    }
}

void cb_reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    double scale = 1.0;
    double aspect = (double) w / (double) h;

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    glFrustum(-1 * scale * aspect,
                   scale * aspect,
              -1 * scale,
                   scale,
                   scale,
                   scale * 1000.0);
    glMatrixMode(GL_MODELVIEW);

    glutPostRedisplay();
}

void cb_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        instance.props |= REQ_SHUTDOWN;
        exit(0);
    case '-': // zoom out
        camZoom(&instance.camera, 1.05);
        break;
    case '+': // zoom in
        camZoom(&instance.camera,1/1.05);
        break;
    default:
        break;
    }
}


void cb_special_key(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        camRotate(&instance.camera, 0.1, 0);
        break;
    case GLUT_KEY_DOWN:
        camRotate(&instance.camera, -0.1, 0);
        break;
    case GLUT_KEY_RIGHT:
        camRotate(&instance.camera, 0.0, 0.1);
        break;
    case GLUT_KEY_LEFT:
        camRotate(&instance.camera, 0.0, -0.1);
        break;
    default:
        break;
    }
}

void cb_update(int time)
{
    glutPostRedisplay();
    glutTimerFunc(30, cb_update, 0);
}

void cb_idle(void)
{
    if (pthread_mutex_trylock(&instance.mutex)) return;
    Mesh old = 0;
    if (instance.q) {
        old = instance.m;
        instance.m = instance.q;
        instance.q = 0;
    }
    pthread_mutex_unlock(&instance.mutex);
    if (old) mDestroy(old);
}

void* init_rendering(void* arg)
{
    unused(arg);

    int argc = 1;
    char* argv = "viewer";
    glutInit(&argc, &argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1165, 720);
    glutCreateWindow("Homework 3");

    glClearColor(0.9, 0.9, 0.9, 1.0);

    GLfloat mat_specular[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1.0, 1.0, -1.0, 0.0 };

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LINE_SMOOTH);

    glShadeModel(GL_SMOOTH);


    glutDisplayFunc(cb_display);
    glutReshapeFunc(cb_reshape);
    glutKeyboardFunc(cb_keyboard);
    glutSpecialFunc(cb_special_key);
    glutTimerFunc(25, cb_update, 0);

    glutMainLoop();

    return 0;
}

