#pragma once

#include <inttypes.h>

class MotionPath {
public:
    enum Profile {
        ACCEL_DECEL = 0,
        ACCEL = 1,
        DECEL = 2,
        CONST_F = 3,
        CONST_S = 4,
    };

    MotionPath();

    virtual bool next(Point* p, int* speed);

    virtual const char* name() const;
};

class VectorPath : public MotionPath {
public:
    VectorPath(Point from, Point to);

    bool next(Point* p, int* speed);

    const char* name() const;

    void SetVelocity(int v);

private:
    Point loc;
    Point delta;
    uint16_t   step;
    uint16_t   accel;
    uint16_t   speed;
};

class CirclePath : public MotionPath {
public:
    CirclePath(Point& centre, float radius, float angle0 = 0, float angle1 = 0);
    CirclePath(Point centre, float radius, float angle0 = 0, float angle1 = 0);
    bool next(Point* p, int* speed);

    const char* name() const;
private:
    Point c;
    float r;
    float phi;
    float phi2;
    int step;
};

class Waypoint {
public:
    inline Waypoint(Point p, MotionPath::Profile prof) : loc(p), profile(prof) {}

    Point loc;
    MotionPath::Profile profile;    
};
