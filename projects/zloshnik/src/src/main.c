#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>

#include "util.h"
#include "config.h"
#include "dac.h"
#include "hvgen.h"
#include "spi.h"

static const uint8_t xyxor[] =
    { 
      0,     0, 0, 0,
      255,   0, 0, 0,
      255, 255, 0, 0,
      0,   255, 0, 0,
      0,     0, 0, 1,
      30,   30, 0, 0,
      30,  240, 0, 0,
      160, 240, 0, 0,
      200, 170, 0, 0,
      160, 100, 0, 0,
      30,  100, 0, 1,
    };
      
void draw_xyxor()
{
    for(int i = 0; i < sizeof(xyxor)/sizeof(xyxor[0]); i += 4) {
        xyz_setdac(xyxor[i], xyxor[i + 1]);
        if (xyxor[i + 3]) 
            for(int j = 0; j < 500; j++) __asm volatile("nop");
        for(int j = 0; j < 1000; j++) 
            __asm volatile("nop");
    }
//    xyz_setdac(0,0);
//    xyz_setdac(128,128);
//    xyz_setdac(255,255);
//    xyz_setdac(128,128);
}

main() 
{
    spi_setup();
    hvgen_setup();
    hvgen_enable();

    xyz_setup();
    _delay_ms(10);
    xyz_setup();
    _delay_ms(10);
    xyz_setup();
    _delay_ms(10);

    for(;;) {
        draw_xyxor();
    }
    return 0;
}
