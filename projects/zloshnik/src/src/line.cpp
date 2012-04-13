#include <inttypes.h>
#include <stdlib.h>

#include "dac.h"
#include "line.h"

Tracer trazador;

#define SWAP(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}

void Tracer::clamp(int *x) 
{
    if(*x < 0) *x = 0;
    else if (*x > 255) *x = 255;
}

void Tracer::MoveTo(int x, int y) 
{
    X = x;
    Y = y;
    clamp(&x);
    clamp(&y);
    xyz.SetXY(x, y);
}

void Tracer::LineTo(int x1, int y1) 
{
    int x0 = X;
    int y0 = Y;
    int Dx = x1 - x0;
    int Dy = y1 - y0;
    int steep = abs(Dy) >= abs(Dx);
    int xstep, ystep;
    int twoDy, twoDyTwoDx, E, x, y, xend;

    if (steep) {
        SWAP(x0, y0);
        SWAP(x1, y1);
        Dx = x1 - x0;
        Dy = y1 - y0;
    }

    xstep = 1;
    if (Dx < 0) {
        xstep = -1;
        Dx = -Dx;
    }

    ystep = 1;
    if (Dy < 0) {
        ystep = -1;
        Dy = -Dy;
    }

    twoDy = 2*Dy;
    twoDyTwoDx = twoDy - 2*Dx;
    E = twoDy - Dx;
    y = y0;
    x = x0;
    xend = x1;

    for(; x != xend;) {
        if (E > 0) {
            E += twoDyTwoDx;
            y += ystep;
        } else {
            E += twoDy;
        }
        x += xstep;

        if (steep) {
            MoveTo(y, x);
        } else {
            MoveTo(x, y);
        }
        //for(int j = 0; j < 20; j++)
        //    __asm volatile("nop");
    }
}
