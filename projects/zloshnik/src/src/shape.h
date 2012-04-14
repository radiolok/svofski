#pragma once

#include <inttypes.h>

class Shape 
{
private:
    int scale;
    int angle;
    int ox, oy;

protected:
    virtual int GetXYZ(uint8_t n, int* x, int* y, int* z) = 0;
public:
    void SetTransform(int ox, int oy, int scale, uint8_t angle);
    void Trace(void);
};

class Box : public Shape
{
protected:
    virtual int GetXYZ(uint8_t n, int* x, int* y, int* z);
};

class Star : public Shape
{
protected:
    virtual int GetXYZ(uint8_t n, int* x, int* y, int* z);
};

