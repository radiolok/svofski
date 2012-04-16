#include <inttypes.h>
#include <avr/pgmspace.h>
#include "shape.h"
#include "line.h"
#include "pecado.h"

#define SWAP(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}

void Shape::SetTransform(int ox, int oy, int scalex, int scaley, uint8_t angle)
{
    this->ox = ox;
    this->oy = oy;
    this->scalex = scalex;
    this->scaley = scaley;
    this->angle = angle;
}

void Shape::Trace(void) 
{
    int x, y, z;

    for (int i = 0; GetXYZ(i, &x, &y, &z) != 0; i++) {
        x = x * scalex;  x = x/256;
        y = y * scaley;  y = y/256;

        irotate(&x, &y, 0, 0, angle);
        x += ox;
        y += oy;
        if (i == 0 || z == 0) {
            trazador.MoveTo(x, y);
        } else {
            LineTo(x, y);
        }
    }
}

void Shape::LineTo(int x, int y) 
{
    trazador.LineTo(x, y);
}

int ContiguousShape::GetXYZ(uint8_t n, int* x, int* y, int* z)
{
    if (n >= npoints)
        return 0;

    *x = (int8_t) pgm_read_byte(&(data[n<<1]));
    *y = (int8_t) pgm_read_byte(&(data[(n<<1)+1]));
    *z = 1;
    return 1;
}

ContiguousShape::ContiguousShape(PGM_IP data, int npoints)
{
    this->data = data;
    this->npoints = npoints;
}


const int8_t boxshape[] PROGMEM = 
    {-1,-1,
      1,-1,
      1, 1,
     -1, 1,
     -1,-1};

const int8_t starshape[] PROGMEM = 
    { 9, 3,
     -6,-8,
      0,10,
      6,-8,
     -9, 3,
      9, 3};

ContiguousShape box(boxshape, sizeof(boxshape)/sizeof(boxshape[0])/2);
ContiguousShape star(starshape, sizeof(starshape)/sizeof(starshape[0])/2);

void SinShape::SetHalfPeriods(uint8_t n)
{ 
    npoints = n * SINSUBDIVS; 
    step = 256/npoints;
}

int SinShape::GetXYZ(uint8_t n, int* x, int* y, int* z) 
{
    if (n >= npoints) return 0;

    *z = 1;

    *y = isin(n*(128/SINSUBDIVS));
    *x = n*step - 128;

    return 1;
}

void SinShape::LineTo(int x, int y) 
{
    trazador.LineTo(x, y);
}

SinShape sinus;

#define GRID8

int GridShape::GetXYZ(uint8_t n, int* x, int* y, int* z) 
{
    uint8_t swap;
#ifdef GRID8
    // 8x8 grid: 18+18 = 36 points
    if (n >= 36) return 0;
    swap = n >= 18;
    if (swap) n -= 18;
#else
    // 16x16 grid: 34+34 = 68 points
    if (n >= 68) return 0;
    swap = n >= 34;
    if (swap) n -= 34;
#endif
    switch (n & 3) {
        case 0: *x = -127; *z = 0; break;
        case 1: *x =  127; *z = 1; break;
        case 2: *x =  127; *z = 0; break;
        case 3: *x = -127; *z = 1; break;
    }
#ifdef GRID8
    *y = ((n>>1) <<5) - 127;
#else
    *y = ((n>>1) <<4) - 127;
#endif
    if (swap) SWAP(*x, *y);

    return 1;
}

GridShape grid;
