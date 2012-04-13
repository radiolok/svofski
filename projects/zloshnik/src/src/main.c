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
#include "pecado.h"
#include "text.h"
#include "line.h"

int16_t scale = 0;
int16_t textscale = 1;
int8_t  d_textscale = 4;
uint8_t a;
uint16_t textangle = 0;

uint8_t centre_x, centre_y;
uint16_t centreangle = 0;

#define XYXOR_WORD "3LO"
#define XYXOR_LEN  3

void draw_xyxor()
{
    scale = 0;
    for(int i = -1, f = 0; i < 256; i+=8, f++) {
        int16_t x = 128 + isin(i);
        int16_t y = 128 + icos(i);
        if (i == -1 || ((f + textangle/4) % 2 == 0)) {
            move_to(x, y);
        }
        else {
            line_to(x,y);
        }
    }

    for(int i = -1; i < 256; i+=8) {
        int16_t x = isin(a);
        int16_t y = icos(a);
        x = 128 + (x * scale)/128;
        y = 128 + (y * scale)/128;
        if (i == -1) {
            move_to(x, y);
        }
        else {
            line_to(x,y);
        }
        scale+=4;
        a+=8;
    }

    int ts = textscale/16;
    int ox = centre_x-5*ts*XYXOR_LEN/2;
    int oy = centre_y-8*ts/2;
    irotate(&ox, &oy, centre_x, centre_y, textangle/4);
    move_to(ox, oy);
    text_str(XYXOR_WORD, ts, textangle/4);
    move_to(ox+2, oy+2);
    text_str(XYXOR_WORD, ts, textangle/4);
    textangle--;
    textscale += d_textscale;
    if (textscale > 200 || textscale < 3) d_textscale = -d_textscale;

    centre_x = 128+isin(centreangle)/4;
    centre_y = 128+icos(centreangle)/4;
    centreangle--;

    if (++scale == 128) scale = 1;
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
