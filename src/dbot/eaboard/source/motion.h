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
    VectorPath();

    void Init(Point from, Point to, int velocity);

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
    inline Waypoint() {}

    Point loc;
    MotionPath::Profile profile;    
};

class Multipath : public MotionPath {
public:
    Multipath(Waypoint *queue, int qlen);

    virtual bool next(Point* p, int* speed);
    virtual const char* name() const;

    void newPoint(int32_t x, int32_t y, int32_t z);

private:
    inline int8_t advanceHead() { return head = ++head == queueLength ? 0 : head; }
    inline int8_t advanceTail() { return tail = ++tail == queueLength ? 0 : tail; }
    bool nextSegment(Point p);

private:
    Waypoint* queue;
    int8_t queueLength;
    int8_t head; 
    int8_t tail;
    VectorPath vp;
};
