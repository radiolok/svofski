#include "common.h"
#include "armmodel.h"
#include "motion.h"

MotionPath::MotionPath() {}

bool MotionPath::next(Point* p, int* speed) {
    (void)*p;
    (void)*speed;
    return FALSE;
}

VectorPath::VectorPath(Point& from, Point& to):
    loc(from),
    delta(0,0,0)
{
    loc.moveto(from);
    float dist = MathUtil::dist(from, to);
    delta.moveto((to.x-from.x)/dist,
                 (to.y-from.y)/dist,
                 (to.z-from.z)/dist);
    step = 1 + (int)roundf(dist);
}

bool VectorPath::next(Point* p, int* speed) {
    if (!step) return FALSE;

    p->moveto(loc);
    *speed = 1;

    loc.offset(delta);

    return --step;
}

