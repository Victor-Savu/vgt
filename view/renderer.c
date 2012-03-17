#include <view/renderer.h>
#include <view/renderer_cls.h>

#include <vgt/obj.h>
#include <string.h>
#include <stdio.h>

#include <vgt/mesh.h>

void cb_display(void);
void cb_reshape(int w, int h);
void cb_keyboard(unsigned char key, int x, int y);
void cb_special_key(int key, int x, int y);
void cb_update(int time);
void cb_idle(void);

void init_rendering(void*);



struct Renderer instance = {
    .m = 0,
    .q = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,

    .running = false,
    .shutdown_requested = false,

    .camera = {
        .view = {.rho = 0.0, .phi = 0.0, .theta = 0.0},
        .frame = FRAME_IDENTITY,
        .focus_point = {
            .color = GRAPHICS_DRAW_3D_ARROW
        },
    .mouse = {
        .state = 0,
        .x = 0,
        .y = 0
    }
};

Renderer rCreate(const char* winname)
{
    if (instance->props & REQ_SHUTDOWN) rDestroy(r);

    instance.props = (RUNNING);
    pthread_create(&instance.threadId, 0, init_rendering, 0);

    return &instance;
}

void rDestroy(Renderer r)
{
    if (!(instance.props & RUNNING)) return;

    instance.props |= REQ_SHUTDOWN;
    void* ret;
    if (!pthread_join(instance.threadID, ret)) free(ret);

    if (instance.m) mDestroy(instance.m);
    if (instance.q) mDestroy(instance.q);
    instance.m = 0;
    instance.q = 0;
    instance.props = 0;
}

void rDisplay(Renderer r, Mesh m)
{
    pthread_mutex_lock(&instance.mutex);
    Mesh old = instance.q;
    instance.q = m;
    pthread_mutex_unlock(&instance.mutex);
    if (old) mDestroy(old);
}

void cb_display(void)
{
    if (instance.m) mDisplay(m);

    if (instance.camera.props | DRAW_FOCUS_POINT) {
        gDraw(&camera.focus_point);
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
        cleanup();
        exit(0);
        return 1;
    case '-': // zoom out
        if (instance.camera.rho < 150.0) instance.camera.rho *= 1.05;
        return 1;
    case '+': // zoom in
        if (instance.camera.rho > 2.5) instance.camera.rho /= 1.05;
        return 1;
    default:
        return 0;
    }
}


void cb_special_key(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        instance.camera.theta += 0.1;
        while (instance.camera.theta > 2 * M_PI) instance.camera.theta -= 2*M_PI;
        return 1;
    case GLUT_KEY_DOWN:
        instance.camera.theta -= 0.1;
        while (instance.camera.theta < 0) instance.camera.theta += 2*M_PI;
        return 1;
    case GLUT_KEY_RIGHT:
        instance.camera.phi += 0.1;
        while (instance.camera.phi > 2 * M_PI) instance.camera.phi -= 2 * M_PI;
        return 1;
    case GLUT_KEY_LEFT:
        instance.camera.phi -= 0.1;
        while (instance.camera.phi < 0) instance.camera.phi += 2 * M_PI;
        return 1;
    default:
        return 0;
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

void init_rendering(void* arg)
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
}

