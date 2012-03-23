#include <view/graphics.h>
#include <view/graphics_cls.h>

#include <GL/gl.h>
#include <stdio.h>
void GRAPHICS_DRAW_3D_ARROW(void)
{
    GLfloat normals[5][3] = {
        {0.0, 0.0, -1.0},
        {1.0, -3.0, 4.0},
        {-1.0, 3.0, -4.0},
        {2.0, -1.0, 2.0},
        {2.0,  1.0, 2.0}
    };

    GLfloat vertices[5][3] = {
        {0.0, 0.0, 1.0},
        {-1.0, 0.0, 0.0},
        {-2.0, -2.0, 0.0},
        {-2.0, 2.0, 0.0},
        {4.0, 0.0, 0.0},
    };

    GLfloat* triangles[6][4] = {
        {vertices[0], vertices[3], vertices[1], normals[4]},
        {vertices[0], vertices[1], vertices[2], normals[3]},
        {vertices[0], vertices[4], vertices[3], normals[2]},
        {vertices[0], vertices[2], vertices[4], normals[1]},
        {vertices[1], vertices[3], vertices[4], normals[0]},
        {vertices[2], vertices[1], vertices[4], normals[0]},
    };
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat gold[] = {1.0, 0.84314, 0.0, 1.0};
    GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
    glColor4fv(gold);
    uint64_t t;
    glBegin(GL_TRIANGLES);
    for (t = 0; t < 6; t++) {
        glNormal3fv(triangles[t][3]);
        glVertex3fv(triangles[t][0]);
        glVertex3fv(triangles[t][1]);
        glVertex3fv(triangles[t][2]);
    }
    glEnd();
    glColor4fv(white);
    glDisable(GL_COLOR_MATERIAL);


}

//Graphics gfxCreate(const Method draw);
//void gfxDestroy(Graphics g);

void gfxDraw(Graphics restrict g)
{
    if (g->draw) g->draw();
}
