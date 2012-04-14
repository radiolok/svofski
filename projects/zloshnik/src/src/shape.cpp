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
        x *= scale/256;
        y *= scale/256;
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

const int8_t boxshape[] PROGMEM = {-1,-1,
                                    1,-1,
                                    1, 1,
                                   -1, 1,
                                   -1,-1};

int Box::GetXYZ(uint8_t n, int* x, int* y, int* z)
{
    if (n >= sizeof(boxshape)/sizeof(int8_t)/2)
        return 0;

    *x = (int8_t) pgm_read_byte(&(boxshape[n<<1]));
    *y = (int8_t) pgm_read_byte(&(boxshape[(n<<1)+1]));
    *z = 1;
    return 1;
}

const int8_t starshape[] PROGMEM = 
    {9, 3,
     -6,-8,
     0,10,
     6,-8,
     -9, 3,
     9, 3};

int Star::GetXYZ(uint8_t n, int* x, int* y, int* z)
{
    if (n >= sizeof(starshape)/sizeof(int8_t)/2)
        return 0;

    *x = (int8_t) pgm_read_byte(&(starshape[n<<1]));
    *y = (int8_t) pgm_read_byte(&(starshape[(n<<1)+1]));
    *z = 1;
    return 1;
}

