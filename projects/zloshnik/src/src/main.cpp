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
#include "shape.h"

int16_t scale = 0;
int16_t scale_sin = 128;
int8_t  scale_sin_d = -1;
int16_t textscale = 4*256;
int16_t  d_textscale = 100;
uint8_t a;
uint16_t textangle = 0;

uint8_t centre_x, centre_y;
uint16_t centreangle = 0;

#define XYXOR_WORD "ZLOSHA"
#define XYXOR_LEN  3

void draw_marchingborder() 
{
    for(int i = -1, f = 0; i < 256; i+=8, f++) {
        int16_t x = 128 + isin(i);
        int16_t y = 128 + icos(i);
        if (i == -1 || ((f + textangle/4) % 2 == 0)) {
            trazador.MoveTo(x, y);
        }
        else {
            trazador.LineTo(x,y);
        }
    }
}

void draw_spiral() 
{
    for(int i = -1; i < 256; i+=8) {
        int16_t x = isin(a);
        int16_t y = icos(a);
        x = 128 + (x * scale)/128;
        y = 128 + (y * scale)/128;
        if (i == -1) {
            trazador.MoveTo(x, y);
        }
        else {
            trazador.LineTo(x,y);
        }
        scale+=4;
        a+=8;
    }
}

void draw_xyxor()
{
    trazador.SetPace(0);
    scale = 0;

    //draw_marchingborder();
    //draw_spiral();

    int ox = 128 - textscale*TEXT_CHARWIDTH*XYXOR_LEN/2/256;
    int oy = 128 - textscale*TEXT_CHARHEIGHT/2/256;
    irotate(&ox, &oy, 128, 128, textangle/5);
    trazador.MoveTo(ox, oy);
    //text.Str(XYXOR_WORD, textscale, textangle/5);

    textangle--;
    textscale += d_textscale;
    if (textscale > 12*256 || textscale < 4*256) d_textscale = -d_textscale;

    centre_x = 128+isin(centreangle)/4;
    centre_y = 128+icos(centreangle)/4;
    centreangle+=16;

    if (++scale == 128) scale = 1;

    star.SetTransform(centre_x, centre_y, textscale, textscale, textangle*5);
    //star.Trace();

    sinus.SetHalfPeriods(2);
    sinus.SetTransform(128,128, 256, scale_sin, 0);
    sinus.Trace();

    scale_sin += scale_sin_d;
    if (scale_sin == 128 || scale_sin == -128) scale_sin_d = -scale_sin_d;
/*
    trazador.MoveTo(80, 50);
    text.Str("Gru..", 1024, 0);
    trazador.MoveTo(80, 80);
    text.Str("Gru..", 1024, 0);
    trazador.MoveTo(80, 110);
    text.Str("Gru..", 1024, 0);
    trazador.MoveTo(80, 140);
    text.Str("Gru..", 1024, 0);
    trazador.MoveTo(80, 170);
    text.Str("Gru..", 1024, 0);
    trazador.MoveTo(80, 200);
    text.Str("Gru..", 1024, 0);
*/
    //draw_marchingborder();
    if ((textangle & 7) == 0) {
        grid.SetTransform(128, 128, 180, 180, 0);
        //grid.SetTransform(centre_x, centre_y, 180, 180, textangle);
        grid.Trace();
    }
    trazador.MoveTo(80, 50);
    text.Str("18:38", 1024, 0);
}

int main() 
{
    spi_setup();
    hvgen_setup();
    hvgen_enable();

    xyz.Setup();
    _delay_ms(10);
    xyz.Setup();
    _delay_ms(10);
    xyz.Setup();
    _delay_ms(10);

    for(;;) {
        draw_xyxor();
    }
    return 0;
}
