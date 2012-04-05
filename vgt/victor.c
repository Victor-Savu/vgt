#include <vgt/victor.h>
#include <vgt/victor_cls.h>

#include <math/obj.h>

#include <ads/array.h>

#include <vgt/half_edge_cls.h>
#include <vgt/volumetric_data.h>
#include <vgt/scalar_field.h>
#include <vgt/vector_field.h>

#include <GL/glut.h>

Victor vicCreate(VolumetricData vol)
{
    Victor vic = oCreate(sizeof (struct Victor));
    vic->vol = vol;
    vic->v = arrCreate(sizeof (Vertex), 2);
    vic->e = arrCreate(sizeof (struct HalfEdge), 2);
    return vic;
}

void vicDestroy(Victor restrict vic)
{
    arrDestroy(vic->v);
    arrDestroy(vic->e);
    oDestroy(vic);
}

Victor vicCopy(Victor restrict vic)
{
    Victor c = vicShallowCopy(vic);
    c->vol = vdCopy(vic->vol);
    return c;
}

Victor vicShallowCopy(Victor restrict vic)
{
    Victor c = oCopy(vic, sizeof (struct Victor));
    c->v = arrCopy(vic->v);
    c->e = arrCopy(vic->e);
    c->vol = 0;
    return c;
}

inline static void render_half_edge(uint64_t i, Obj e, Obj unused)
{
    ignore(unused);
    Vertex* const a = oCast(HalfEdge, e)->v;
    Vertex* const b = oCast(HalfEdge, e)->n->v;
    if (a < b) {
        glVertex3v(*a);
        glVertex3v(*b);
    }
}


inline static void render_vertex(uint64_t i, Obj v, Obj unused)
{
    ignore(unused);
    glVertex3v(*oCast(Vertex*, v));
}

void vicDisplay(Victor restrict vic)
{
    glColor3f(0.0, 0.5, 1.0);
    glBegin(GL_LINES);
    arrForEach(vic->e, render_half_edge, 0);
    glEnd();

    glColor3f(1.0, 0.5, 0.0);
    glBegin(GL_POINTS);
    arrForEach(vic->v, render_vertex, 0);
    glEnd();

}
