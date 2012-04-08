#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>

#include "util.h"
#include "config.h"
#include "dac.h"
#include "hvgen.h"

static const uint8_t xyxor[] =
    { 128, 128, 0, 1,
      0,     0, 0, 1,
      255,   0, 0, 1,
      255, 255, 0, 1,
      0,   255, 0, 1,
      0,     0, 0, 1,
    };
      
void draw_xyxor()
{
    for(int i = 0; i < sizeof(xyxor)/sizeof(xyxor[0]); i += 4) {
        xyz_setdac(xyxor[i], xyxor[i + 1]);
        if (xyxor[i + 3]) _delay_ms(10);
    }
}

main() 
{
    hvgen_setup();
    // hvgen_enable();

    xyz_setup();

    for(;;) {
        draw_xyxor();
    }
    return 0;
}
