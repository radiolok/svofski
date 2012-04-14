#include <inttypes.h>
#include <avr/pgmspace.h>
#include "shape.h"
#include "line.h"
#include "pecado.h"


void Shape::SetTransform(int ox, int oy, int scale, uint8_t angle)
{
    this->ox = ox;
    this->oy = oy;
    this->scale = scale;
    this->angle = angle;
}

void Shape::Trace(void) 
{
    int x, y, z;

    for (int i = 0; GetXYZ(i, &x, &y, &z) != 0; i++) {
        volatile int xx, yy;

        xx = x * scale;  x = xx/256;
        yy =  y * scale; y = yy/256;

        //x *= scale; x/=256;
        //y *= scale; x/=256;
        irotate(&x, &y, 0, 0, angle);
        x += ox;
        y += oy;
        if (i == 0 || z == 0) {
            trazador.MoveTo(x, y);
        } else {
            trazador.LineTo(x, y);
        }
    }
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

int SinShape::GetXYZ(uint8_t n, int* x, int* y, int* z) 
{
    if (n >= npoints) return 0;

    *z = 1;

    *y = isin(n*(128/SINSUBDIVS))/4;
    *x = n*step/8;

    return 1;
}

SinShape sinus;
