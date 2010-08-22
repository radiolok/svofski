#pragma once

#include <math.h>
#include "xprintf.h"

class Point;

/// Generic math utilities
class MathUtil {
public:
    /// Convert angle in radians into degrees
    inline static float deg(float rad) { return rad / M_PI * 180.0f; }

    /// Convert angle in degrees into radians
    inline static float rad(float deg) { return deg / 180.0f * M_PI; }

    /// Euclidean distance betwenn two points: (x1,y1,z1) and (x2,y2,z2)
    static float dist(float, float, float, float, float, float);
    static float dist(Point& a, Point& b);
};

/// A point in 3-D space, floating point
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

/// \brief Model of a single arm of a delta robot
///
/// This class represents one of the 3 identical arms.
/// A complete robot is a junction of 3 arms fixed at 
/// 120 degrees and controlling the single effector.
/// 
/// All units used in calculation are relative, the resulting
/// servo angles are in radians.
///
/// For an interactive model and full explanation see
/// http://sensi.org/~svo/dbot/
///
class ArmModel {
public:
    /// \param _angle this arm's angle: 0, 120 or 240 degrees
    /// \param _hip   hip bone length 
    /// \param _ankle ankle bone length
    /// \param _base  base radius, distance from the vertical axis to the hip pivot point
    /// \param _effector effector radius, distance from the bottom hinge to effector centre
    ArmModel(const float _angle, const float _hip, const float _ankle, const float _base, const float _effector);

    /// Calculate servo set angle for the effector position
    /// defined by (xg, yg, zg). The value can also be later
    /// obtained via getRho()
    /// \return servo set angle in radians
    float MoveTo(const float xg, const float yg, const float zg);

    /// Get calculated servo set angle, same value as returned by MoveTo()
    /// \return servo angle in radians
    inline float getRho() const { return rho; }

private:
    float rho;                  /// servo angle, see getRho()

    float a;                    /// ankle length
    float b;                    /// hip length
    float c;                    /// base to goal distance

    const float base;           /// base radius
    const float effector;       /// effector radius
    const float angle;          /// this arm's angle (0, 120 or 240)
};
