#include <math/frame.h>
#include <math/frame_cls.h>

#include <math/obj.h>
#include <math/mat.h>
#include <math/vertex.h>

Frame frCreate()
{
    return oCreate(sizeof (struct Frame));
}

void frDestroy(Frame restrict f)
{
    oDestroy(f);
}

Frame frRotate(Frame restrict f, Mat* restrict r, Frame restrict g)
{
    matMul(&f->rot, r, &g->rot);
    vCopy(&f->trans, &g->trans);
    return g;
}

Frame frTranslate(Frame restrict f, Vertex* restrict p, Frame restrict g)
{
    matCopy(&f->rot, &g->rot);
    vAdd(&f->trans, p, &g->trans);
    return g;
}

Frame frCompose(Frame restrict f, Frame restrict r, Frame restrict g)
{
    Mat* restrict r1 = &f->rot;
    Mat* restrict r2 = &r->rot;
    Mat* restrict r3 = &g->rot;

    Vertex* restrict t1 = &f->trans;
    Vertex* restrict t2 = &r->trans;
    Vertex* restrict t3 = &g->trans;

    matMul(r1, r2, r3);
    matCross(r1, t2, t3);
    vAddI(t3, t1);

    return g;
}

Vertex* frTransform(Frame restrict f, Vertex* restrict p, Vertex* restrict t)
{
    ignore matCross(&f->rot, p, t);
    ignore vAddI(t, &f->trans);
    return t;
}

Vertex* frTransformI(Frame restrict f, Vertex* restrict p)
{
    ignore matCrossI(&f->rot, p);
    ignore vAddI(p, &f->trans);
    return p;
}

