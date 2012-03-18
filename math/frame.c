#include <frame.h>
#include <frame_cls.h>


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
    vCopy(&f->pos, &g->pos);
    return g;
}

Frame frTranslate(Frame restrict f, Vec* restrict p, Frame restrict g)
{
    matCopy(&f->rot, &g->rot);
    vAdd(&f->pos, p, &g->pos);
    return g;
}

Frame frCompose(Frame restrict f, Frame restrict r, Frame restrict g)
{
    Mat* restrict r1 = &f->rot;
    Mat* restrict r2 = &r->rot;
    Mat* restrict r3 = &g->rot;

    Vec* restrict t1 = &f->pos;
    Vec* restrict t2 = &r->pos;
    Vec* restrict t3 = &g->pos;

    matMul(r1, r2, r3);
    matCross(r1, t2, t3);
    vAddI(t3, t1);

    return g;
}
