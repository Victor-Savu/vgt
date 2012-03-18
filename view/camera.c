#include <view/camera.h>
#include <view/camera_cls.h>

const real min_distance = 1e-6;
const real max_distance = 1e+6;

void camPan(Camera c, real x, real y)
{
    const real st = sin(c->view.theta);
    const real sp = sin(v->view.phi);
    const real ct = cos(v->view.theta);
    const real cp = cos(v->view.phi);
    c->frame.pos[0] -= x * (cp * st) * (ct + st) + y * sp * ct;
    c->frame.pos[1] += st * (y - x * cp * sp * (ct-sp));
    c->frame.pos[2] += x * sp * (ct * ct + st * st) - y * cp * st;
}

void camRotate(Camera c, real up, real, right)
{
    c->view.theta -= up;
    while (c->view.theta < 0) c->view.theta += M_2PI;
    while (c->view.theta > M_2PI) c->view.theta -= M_2PI;
    c->view.phi += right;
    while (c->view.phi < 0) c->view.phi += M_PI;
    while (c->view.phi > M_PI) c->view.phi -= M_PI;
}

void camZoom(Camera x, real f)
{
    if (f * c->view.rho < min_distance) {
        c->view.rho = min_distance;
    } else {
        if (f * c->view > max_distance) {
            c->view.rho = max_distance;
        } else {
            c->view.rho *= f;
        }
    }
}

void camPosition(Camera c)
{
    Spherical restrict v = &c->view;
    Vec f = {
        v->rho * sin(v->phi) * sin(v->theta),
        v->rho * cos(v->theta),
        v->rho * cos(v->phi) * sin (v->theta)
    };
    matCrossI(&c->frame, &f);
    gluLookAt(
            f->x, f->y, f->z,   // from
            c->frame.x, c->frame.y, c->frame.z, // to
            -1.0 * sin(v->phi) * cos(v->theta), sin(v->theta), -1.0 * cos(v->phi) * sin(v->theta) // up
            );
}
