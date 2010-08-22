#pragma once

class MotionPath {
public:
    MotionPath();

    virtual bool next(Point* p, int* speed);
};

class VectorPath : public MotionPath {
public:
    VectorPath(Point& from, Point& to);

    bool next(Point* p, int* speed);

private:
    Point loc;
    Point delta;
    int   step;
};

class CirclePath : public MotionPath {
public:
    CirclePath(Point& centre, float radius, float angle0 = 0, float angle1 = 0);
    bool next(Point* p, int* speed);

private:
    Point c;
    float r;
    float phi;
    float phi2;
};

