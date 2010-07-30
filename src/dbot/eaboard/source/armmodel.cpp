#include <math.h>
#include "ArmModel.h"

float MathUtil::dist(float x1, float y1, float z1,
                     float x2, float y2, float z2) 
{
    return sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
}

Point::Point(const float _x, const float _y, const float _z) :
    x(_x), y(_y), z(_z)
{
}

void Point::moveto(const float _x, const float _y, const float _z) {
    x = _x;
    y = _y;
    z = _z;
}

void Point::moveto(Point& o) {
    moveto(o.x, o.y, o.z);
}

void Point::offset(const float _x, const float _y, const float _z) {
    x += _x;
    y += _y;
    z += _z;
}

void Point::offset(Point& o) {
    offset(o.x, o.y, o.z);
}

float Point::distFrom(Point& o) {
    return MathUtil::dist(x, y, z, o.x, o.y, o.z);
}

void Point::rotateY(float phi) {
    float x2 = x*cosf(phi) - z*sinf(phi);
    float z2 = x*sinf(phi) + z*cosf(phi);
                
    x = x2;
    z = z2;
}

void Point::rotateZ(float phi) {
    float x2 = x*cosf(phi) - y*sinf(phi);
    float y2 = x*sinf(phi) + y*cosf(phi);
    
    x = x2;
    y = y2;
}


ArmModel::ArmModel(const float _angle, const float _hip, const float _ankle, const float _base, const float _effector):
            a(_ankle), 
            b(_hip), 
            base(_base), 
            effector(_effector),
            angle(MathUtil::rad(_angle))
{
}


float ArmModel::MoveTo(const float xg, const float yg, const float zg) {
    Point g(xg, yg, zg);
    g.rotateY(-angle);
    
    // offset the goal by effector size
    g.offset(effector,0,0);
    
    // find projection of a on the z=0 plane, squared
    float a2 = a*a - g.z*g.z;  // sqrt(a*a - g.z*g.z), but we only need it squared

    // calculate c with respect to base offset
    // pure math version (for verification):
    //   c = sqrt((xg+effector-base)*(xg+effector-base) + yg*yg);
    // but since we have g-spot offset by effector, we can use dist() 
    // to calculate c like this:
    c = MathUtil::dist(g.x, g.y, 0, base, 0, 0);
    alpha = acosf((-a2+b*b+c*c)/(2*b*c));
    
    beta = atan2f(g.y, g.x-base);
    
    rho = alpha - beta;
   
    return rho;
}

