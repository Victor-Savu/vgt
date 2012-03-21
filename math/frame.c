#include <math/frame.h>
#include <math/frame_cls.h>

#include <math/obj.h>
#include <math/mat.h>
#include <math/vec.h>

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

Frame frTranslate(Frame restrict f, Vec* restrict p, Frame restrict g)
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

    Vec* restrict t1 = &f->trans;
    Vec* restrict t2 = &r->trans;
    Vec* restrict t3 = &g->trans;

    matMul(r1, r2, r3);
    matCross(r1, t2, t3);
    vAddI(t3, t1);

    return g;
}

Vec* frTransform(Frame restrict f, Vec* restrict p, Vec* restrict t)
{
    ignore matCrossI(&f->rot, t);
    ignore vAddI(t, &f->trans);
    return t;
}
