#pragma once

#include <inttypes.h>

class Tracer {
public:
    int X, Y;

    void MoveTo(int x, int y);
    void LineTo(int x1, int y1);

private:
    void clamp(int* x);
};

extern Tracer trazador;
