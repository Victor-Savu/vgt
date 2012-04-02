#include <view/renderer.h>
#include <view/renderer_cls.h>

#include <math/obj.h>
#include <string.h>
#include <stdio.h>

#include <math/obj.h>
#include <vgt/mesh.h>
#include <vgt/delaunay.h>
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
    .new_m = 0,
    .d = 0,
    .new_d = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .props = 0,

    .camera = {
        .view = {.rho = 100.0, .phi = 0.0, .theta = 0.00001},
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
    if (instance.new_m) mDestroy(instance.new_m);
    if (instance.d) delDestroy(instance.d);
    if (instance.new_d) delDestroy(instance.new_d);

    instance.m = 0;
    instance.new_m = 0;
    instance.d = 0;
    instance.new_d = 0;
    instance.props = 0;
}

void rWait(Renderer restrict r)
{
    Obj ret = 0;
    if (!pthread_join(instance.threadId, &ret)) oDestroy(ret);
}

void rDisplayMesh(Renderer restrict r, Mesh restrict m)
{
    pthread_mutex_lock(&instance.mutex);
    Mesh restrict old = instance.new_m;
    instance.new_m = m;
    pthread_mutex_unlock(&instance.mutex);
    if (old) mDestroy(old);
}

void rDisplayDelaunay(Renderer restrict r, Delaunay restrict d)
{
    pthread_mutex_lock(&instance.mutex);
    Delaunay restrict old = instance.new_d;
    instance.new_d = d;
    pthread_mutex_unlock(&instance.mutex);
    if (old) delDestroy(old);
}



void cb_display(void)
{
    //printf("Drawing..\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    camPosition(&instance.camera);

    //glPushMatrix();
    //glTranslatef(0.0, 0.0, -2.0);

    //glutSolidTeapot(1.0);

    //glPopMatrix();

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (instance.m) mDisplay(instance.m);
    if (instance.d) delDisplay(instance.d);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    //if (instance.props & DRAW_FOCUS_POINT) {
     //     gfxDraw(&instance.camera.focus_point);
    //}
    glutSwapBuffers();
}

void cb_reshape(int w, int h)
{
    glViewport(0, 0, w, h);

//    double scale = 4.0;
    double aspect = (double) w / (double) h;

    glMatrixMode(GL_PROJECTION);
    //glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    gluPerspective(45, aspect, 1, 1000.0);
    /*glFrustum(-1 * scale * aspect,
                   scale * aspect,
              -1 * scale,
                   scale,
                   scale / 1.0,
                   scale * 1000.0);*/
    /*glOrtho(-1 * scale * aspect,
                   scale * aspect,
              -1 * scale,
                   scale,
                   scale / 1.0,
                   scale * 1000.0);*/
    glMatrixMode(GL_MODELVIEW);

    glScalef(10, 10, 10);

    glutPostRedisplay();
}

void cb_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        instance.props |= REQ_SHUTDOWN;
        pthread_exit(0);
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
    Mesh old_m = 0;
    if (instance.new_m) {
        old_m = instance.m;
        instance.m = instance.new_m;
        instance.new_m = 0;
    }
    Delaunay old_d = 0;
    if (instance.new_d) {
        old_d = instance.d;
        instance.d = instance.new_d;
        instance.new_d = 0;
    }
    pthread_mutex_unlock(&instance.mutex);
    if (old_m) mDestroy(old_m);
    if (old_d) delDestroy(old_d);
}

void* init_rendering(void* arg)
{
    unused(arg);

    int argc = 1;
    char* argv = "viewer";
    glutInit(&argc, &argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(1165, 720);
    glutCreateWindow("Homework 3");

    //glClearColor(0.9, 0.9, 0.9, 1.0);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
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
    //glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);

    //GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat grey[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
    //GLfloat gold[] = {1.0, 0.84314, 0.0, 1.0};
    GLfloat shininess = 100.0;

    glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, grey);




    glutDisplayFunc(cb_display);
    glutReshapeFunc(cb_reshape);
    glutKeyboardFunc(cb_keyboard);
    glutSpecialFunc(cb_special_key);
    glutIdleFunc(cb_idle);
    glutTimerFunc(30, cb_update, 0);

    glutMainLoop();

    return 0;
}

