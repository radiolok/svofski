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
