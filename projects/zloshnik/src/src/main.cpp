#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
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
int16_t scale_sin = 1;
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

char timetext[6];
char bigtext[17];
uint16_t time;
uint16_t frameno;

void updateText() 
{
    timetext[0] = '0' + (time >> 12);
    timetext[1] = '0' + ((time >> 8) & 15);
    timetext[2] = (frameno & 64) == 0 ? ':' : ' ';
    timetext[3] = '0' + ((time >> 4) & 15);
    timetext[4] = '0' + (time & 15);
    timetext[5] = 0;

    if ((frameno & 16) == 0) {
        //for (int i = 0; i < 4; i++) {
        int i = rand() % 4;
            int rnd;
            while ((rnd = rand() % 32) > 25);
            bigtext[i] = 'A' + rnd;
        //}
        bigtext[4] = 0;
    }
}

void draw_xyxor()
{
    frameno++;

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
    centreangle+=1;

    if (++scale == 128) scale = 1;

    //star.SetTransform(centre_x, centre_y, textscale/2, textscale/2, textangle/5);
    //star.Trace();

    sinus.SetHalfPeriods(2);
    scale_sin += scale_sin_d;
    if (scale_sin == 32 || scale_sin == -32) scale_sin_d = -scale_sin_d;
    sinus.SetTransform(128,200, 100, scale_sin, 0);
    sinus.Trace();

    //sinus.SetTransform(128,200, 256, scale_sin, 0);
    //scale_sin += scale_sin_d;
    //if (scale_sin == 128 || scale_sin == -128) scale_sin_d = -scale_sin_d;

    //draw_marchingborder();
#if 0
    if ((textangle & 7) == 0) {
        grid.SetTransform(128, 128, 180, 180, 0);
        //grid.SetTransform(centre_x, centre_y, 180, 180, textangle);
        grid.Trace();
    }
#endif
    
    if (frameno % 16 == 0) {
        time = (time & 0xff00) + bcd_increment(time & 0xff);
        if ((time & 0xff) == 0x60) {
            time = bcd_increment(time>>8)<<8;
        }
        updateText();
    }

    trazador.MoveTo(60, 50);
    text.Str(timetext, 1024, 0);
    trazador.MoveTo(30, 100);
    text.Str(bigtext, 2048, 0);
}

int main() 
{
    spi_setup();
    hvgen_setup();
    hvgen_enable();

    _delay_ms(500);
    xyz.Setup();
    _delay_ms(500);
    xyz.Setup();

    for(;;) {
        draw_xyxor();
    }
    return 0;
}
