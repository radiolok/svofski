#pragma once

#include <math.h>
#include "xprintf.h"

class MathUtil {
public:
    inline static float deg(float rad) { return rad / M_PI * 180.0f; }
    inline static float rad(float deg) { return deg / 180.0f * M_PI; }
    static float dist(float, float, float, float, float, float);
};

class Point {
public:
    float x, y, z;

    Point(float, float, float);
    void moveto(float,float,float);
    void moveto(Point& o);
    void offset(Point& o);
    void offset(float,float,float);
    float distFrom(Point& o);
    void rotateY(float phi);
    void rotateZ(float phi);

    inline void print() {xprintf("P[%d,%d,%d]", (int)x, (int)y, (int)z);}
};


class ArmModel {
public:
    ArmModel(const float _angle, const float _hip, const float _ankle, const float _base, const float _effector);

    float MoveTo(const float xg, const float yg, const float zg);

    inline float getRho() const { return rho; }

private:
    float alpha, beta, rho;

    float a, b;                 // ankle length, hip length
    float c;                    // base to goal distance

    const float base;           // base radius
    const float effector;       // effector radius
    const float angle;          // this arm's angle (0, 120 or 240)
};
