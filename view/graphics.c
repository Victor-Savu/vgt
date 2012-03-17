#include <graphics.h>
#include <graphics_cls.h>

#include <GL/gl.h>

void GRAPHICS_DRAW_3D_ARROW(void)
{
    GLfloat normals[5][3] = {
        {0.0, 0.0, -1.0},
        {1.0, -3.0, 4.0},
        {-1.0, 3.0, -4.0},
        {2.0, -1.0, 2.0},
        {-2.0, -1.0, -2.0}
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
    }


    GLfloat no_mat[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_difuse[] = {1.0, 0.84314, 0.0, 1.0};
    GLfloat mat_emission[] = {1.0, 0.84314, 0.0, 1.0};
    GLfloat mat_shininess[] = {50.0};

    glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    unsigned int t;
    glBegin(GL_TRIANGLES);
    for (t = 0; t < 6; t++) {
        glNormal3fv(triangles[t][3]);
        glVertex3fv(triangles[t][0]);
        glVertex3fv(triangles[t][1]);
        glVertex3fv(triangles[t][2]);
    }
    glEnd();
}

//Graphics gfxCreate(const Method draw);
//void gfxDestroy(Graphics g);

Graphics gfxDraw(Graphics g)
{
    if (g->draw) g->draw();
}
