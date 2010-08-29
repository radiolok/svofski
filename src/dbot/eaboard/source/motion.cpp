#include "common.h"
#include "armmodel.h"
#include "motion.h"

MotionPath::MotionPath() {}

bool MotionPath::next(Point* p, int* speed) {
    (void)*p;
    (void)*speed;
    return FALSE;
}

const char* MotionPath::name() const {
    return 0;
}


VectorPath::VectorPath(Point from, Point to):
    loc(from),
    delta(0,0,0)
{
    Init(from, to, ACCEL_DECEL);
}

VectorPath::VectorPath() 
{
}

void VectorPath::Init(Point from, Point to, int velocity) {
    //xprintf("VP:["); from.print(); to.print(); xprintf("]\n");
    loc.moveto(from);
    float dist = MathUtil::dist(from, to);
    delta.moveto((to.x-from.x)/dist,
                 (to.y-from.y)/dist,
                 (to.z-from.z)/dist);
    step = 1 + (int)roundf(dist);
    accel = 0;
    SetVelocity(velocity);
}

void VectorPath::SetVelocity(int v) {
    speed = v;
}

bool VectorPath::next(Point* p, int* spd) {
    if (!step) return FALSE;


    accel++;
    if (accel < 64) {
        if (speed == ACCEL_DECEL || speed == ACCEL) {
            *spd = (64-accel)/16 + 1;
        } else {
            *spd = speed == CONST_S ? 3 : speed == CONST_F ? 1 : 1;
        }
    } 
    else 
    if (step < 128) {
        if (speed == ACCEL_DECEL || speed == DECEL) { 
            *spd = (128-step)/32 + 1;
        } else {
            *spd = speed == CONST_S ? 3 : speed == CONST_F ? 1 : 1;
        }
    }
    else {
        *spd = speed == CONST_S ? 3 : speed == CONST_F ? 1 : 1;
    }

    p->moveto(loc);

    loc.offset(delta);

    return --step;
}

const char* VectorPath::name() const {
    return "vp";
}


CirclePath::CirclePath(Point& centre, float radius, float startangle, float endangle):
    c(centre),
    r(radius),
    phi(MathUtil::rad(startangle)),
    phi2(MathUtil::rad(endangle))
{
    step = (int)floorf(endangle - startangle) + 1;
}

CirclePath::CirclePath(Point centre, float radius, float startangle, float endangle):
    c(centre),
    r(radius),
    phi(MathUtil::rad(startangle)),
    phi2(MathUtil::rad(endangle))
{
    step = (int)floorf(endangle - startangle)*2 + 1;
}

bool CirclePath::next(Point*p, int* speed) {
    if (!step) return FALSE;

    p->moveto(r * cosf(phi), p->y, r * sinf(phi));
    *speed = 1;
    phi += MathUtil::rad(0.5);

    return --step;
}

const char* CirclePath::name() const {
    return "cp";
}

Multipath::Multipath(Waypoint* q, int qlen) : 
    queue(q), queueLength(qlen), head(0), tail(0)
{
}

void Multipath::newPoint(int32_t x, int32_t y, int32_t z) 
{
    bool needUpdate = FALSE;

    queue[head].loc.moveto(x,y,z);
    if (head == tail) {
        needUpdate = TRUE;
    }

    advanceHead();

    if (needUpdate) nextSegment(Point(x,y,z));
}

const char* Multipath::name() const {
    return "p+";
}

bool Multipath::nextSegment(Point p) {
    if (head != tail) {
        vp.Init(p, queue[tail].loc, MotionPath::CONST_F);
        advanceTail();
        return TRUE;
    }

    return FALSE;
}

bool Multipath::next(Point* p, int* speed) {
    bool result = vp.next(p, speed);
    if (!result) {
        result = nextSegment(*p);
    }

    return result;
}
